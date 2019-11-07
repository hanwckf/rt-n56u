
/*
 * Handle communication with knfsd internal cache
 *
 * We open /proc/net/rpc/{auth.unix.ip,nfsd.export,nfsd.fh}/channel
 * and listen for requests (using my_svc_run)
 * 
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include <mntent.h>
#include "misc.h"
#include "nfslib.h"
#include "exportfs.h"
#include "mountd.h"
#include "fsloc.h"
#include "pseudoflavors.h"

#ifdef USE_BLKID
#include "blkid/blkid.h"
#endif

/*
 * Invoked by RPC service loop
 */
void	cache_set_fds(fd_set *fdset);
int	cache_process_req(fd_set *readfds);

enum nfsd_fsid {
	FSID_DEV = 0,
	FSID_NUM,
	FSID_MAJOR_MINOR,
	FSID_ENCODE_DEV,
	FSID_UUID4_INUM,
	FSID_UUID8,
	FSID_UUID16,
	FSID_UUID16_INUM,
};

/*
 * Support routines for text-based upcalls.
 * Fields are separated by spaces.
 * Fields are either mangled to quote space tab newline slosh with slosh
 * or a hexified with a leading \x
 * Record is terminated with newline.
 *
 */
static int cache_export_ent(char *domain, struct exportent *exp, char *p);

#define INITIAL_MANAGED_GROUPS 100

char *lbuf  = NULL;
int lbuflen = 0;
extern int use_ipaddr;

static void auth_unix_ip(FILE *f)
{
	/* requests are
	 *  class IP-ADDR
	 * Ignore if class != "nfsd"
	 * Otherwise find domainname and write back:
	 *
	 *  "nfsd" IP-ADDR expiry domainname
	 */
	char *cp;
	char class[20];
	char ipaddr[INET6_ADDRSTRLEN + 1];
	char *client = NULL;
	struct addrinfo *tmp = NULL;
	if (readline(fileno(f), &lbuf, &lbuflen) != 1)
		return;

	xlog(D_CALL, "auth_unix_ip: inbuf '%s'", lbuf);

	cp = lbuf;

	if (qword_get(&cp, class, 20) <= 0 ||
	    strcmp(class, "nfsd") != 0)
		return;

	if (qword_get(&cp, ipaddr, sizeof(ipaddr) - 1) <= 0)
		return;

	tmp = host_pton(ipaddr);
	if (tmp == NULL)
		return;

	auth_reload();

	/* addr is a valid, interesting address, find the domain name... */
	if (!use_ipaddr) {
		struct addrinfo *ai = NULL;

		ai = client_resolve(tmp->ai_addr);
		if (ai) {
			client = client_compose(ai);
			freeaddrinfo(ai);
		}
	}
	qword_print(f, "nfsd");
	qword_print(f, ipaddr);
	qword_printtimefrom(f, DEFAULT_TTL);
	if (use_ipaddr) {
		memmove(ipaddr + 1, ipaddr, strlen(ipaddr) + 1);
		ipaddr[0] = '$';
		qword_print(f, ipaddr);
	} else if (client)
		qword_print(f, *client?client:"DEFAULT");
	qword_eol(f);
	xlog(D_CALL, "auth_unix_ip: client %p '%s'", client, client?client: "DEFAULT");

	free(client);
	freeaddrinfo(tmp);

}

static void auth_unix_gid(FILE *f)
{
	/* Request are
	 *  uid
	 * reply is
	 *  uid expiry count list of group ids
	 */
	uid_t uid;
	struct passwd *pw;
	static gid_t *groups = NULL;
	static int groups_len = 0;
	gid_t *more_groups;
	int ngroups;
	int rv, i;
	char *cp;

	if (groups_len == 0) {
		groups = malloc(sizeof(gid_t) * INITIAL_MANAGED_GROUPS);
		if (!groups)
			return;

		groups_len = INITIAL_MANAGED_GROUPS;
	}

	ngroups = groups_len;

	if (readline(fileno(f), &lbuf, &lbuflen) != 1)
		return;

	cp = lbuf;
	if (qword_get_uint(&cp, &uid) != 0)
		return;

	pw = getpwuid(uid);
	if (!pw)
		rv = -1;
#ifndef __UCLIBC__
	else {
		rv = getgrouplist(pw->pw_name, pw->pw_gid, groups, &ngroups);
		if (rv == -1 && ngroups >= groups_len) {
			more_groups = realloc(groups, sizeof(gid_t)*ngroups);
			if (!more_groups)
				rv = -1;
			else {
				groups = more_groups;
				groups_len = ngroups;
				rv = getgrouplist(pw->pw_name, pw->pw_gid,
						  groups, &ngroups);
			}
		}
	}
#endif /* __UCLIBC__  */
	qword_printuint(f, uid);
	qword_printtimefrom(f, DEFAULT_TTL);
	if (rv >= 0) {
		qword_printuint(f, ngroups);
		for (i=0; i<ngroups; i++)
			qword_printuint(f, groups[i]);
	} else
		qword_printuint(f, 0);
	qword_eol(f);
}

#if USE_BLKID
static const char *get_uuid_blkdev(char *path)
{
	/* We set *safe if we know that we need the
	 * fsid from statfs too.
	 */
	static blkid_cache cache = NULL;
	struct stat stb;
	char *devname;
	blkid_tag_iterate iter;
	blkid_dev dev;
	const char *type;
	const char *val, *uuid = NULL;

	if (cache == NULL)
		blkid_get_cache(&cache, NULL);

	if (stat(path, &stb) != 0)
		return NULL;
	devname = blkid_devno_to_devname(stb.st_dev);
	if (!devname)
		return NULL;
	dev = blkid_get_dev(cache, devname, BLKID_DEV_NORMAL);
	free(devname);
	if (!dev)
		return NULL;
	iter = blkid_tag_iterate_begin(dev);
	if (!iter)
		return NULL;
	while (blkid_tag_next(iter, &type, &val) == 0) {
		if (strcmp(type, "UUID") == 0)
			uuid = val;
		if (strcmp(type, "TYPE") == 0 &&
		    strcmp(val, "btrfs") == 0) {
			uuid = NULL;
			break;
		}
	}
	blkid_tag_iterate_end(iter);
	return uuid;
}
#else
#define get_uuid_blkdev(path) (NULL)
#endif

static int get_uuid(const char *val, size_t uuidlen, char *u)
{
	/* extract hex digits from uuidstr and compose a uuid
	 * of the given length (max 16), xoring bytes to make
	 * a smaller uuid.
	 */
	size_t i = 0;
	
	memset(u, 0, uuidlen);
	for ( ; *val ; val++) {
		int c = *val;
		if (!isxdigit(c))
			continue;
		if (isalpha(c)) {
			if (isupper(c))
				c = c - 'A' + 10;
			else
				c = c - 'a' + 10;
		} else
			c = c - '0' + 0;
		if ((i&1) == 0)
			c <<= 4;
		u[i/2] ^= (char)c;
		i++;
		if (i == uuidlen*2)
			i = 0;
	}
	return 1;
}

static int uuid_by_path(char *path, int type, size_t uuidlen, char *uuid)
{
	/* get a uuid for the filesystem found at 'path'.
	 * There are several possible ways of generating the
	 * uuids (types).
	 * Type 0 is used for new filehandles, while other types
	 * may be used to interpret old filehandle - to ensure smooth
	 * forward migration.
	 * We return 1 if a uuid was found (and it might be worth 
	 * trying the next type) or 0 if no more uuid types can be
	 * extracted.
	 */

	/* Possible sources of uuid are
	 * - blkid uuid
	 * - statfs64 uuid
	 *
	 * On some filesystems (e.g. vfat) the statfs64 uuid is simply an
	 * encoding of the device that the filesystem is mounted from, so
	 * it we be very bad to use that (as device numbers change).  blkid
	 * must be preferred.
	 * On other filesystems (e.g. btrfs) the statfs64 uuid contains
	 * important info that the blkid uuid cannot contain:  This happens
	 * when multiple subvolumes are exported (they have the same
	 * blkid uuid but different statfs64 uuids).
	 * We rely on get_uuid_blkdev *knowing* which is which and not returning
	 * a uuid for filesystems where the statfs64 uuid is better.
	 *
	 */
	struct statfs64 st;
	char fsid_val[17];
	const char *blkid_val;
	const char *val;

	blkid_val = get_uuid_blkdev(path);

	if (statfs64(path, &st) == 0 &&
	    (st.f_fsid.__val[0] || st.f_fsid.__val[1]))
		snprintf(fsid_val, 17, "%08x%08x",
			 st.f_fsid.__val[0], st.f_fsid.__val[1]);
	else
		fsid_val[0] = 0;

	if (blkid_val && (type--) == 0)
		val = blkid_val;
	else if (fsid_val[0] && (type--) == 0)
		val = fsid_val;
	else
		return 0;

	get_uuid(val, uuidlen, uuid);
	return 1;
}

/* Iterate through /etc/mtab, finding mountpoints
 * at or below a given path
 */
static char *next_mnt(void **v, char *p)
{
	FILE *f;
	struct mntent *me;
	size_t l = strlen(p);
	if (*v == NULL) {
		f = setmntent("/etc/mtab", "r");
		*v = f;
	} else
		f = *v;
	while ((me = getmntent(f)) != NULL &&
	       (strncmp(me->mnt_dir, p, l) != 0 ||
		me->mnt_dir[l] != '/'))
		;
	if (me == NULL) {
		endmntent(f);
		*v = NULL;
		return NULL;
	}
	return me->mnt_dir;
}

static int is_subdirectory(char *child, char *parent)
{
	size_t l = strlen(parent);

	if (strcmp(parent, "/") == 0)
		return 1;

	return strcmp(child, parent) == 0
		|| (strncmp(child, parent, l) == 0 && child[l] == '/');
}

static int path_matches(nfs_export *exp, char *path)
{
	if (exp->m_export.e_flags & NFSEXP_CROSSMOUNT)
		return is_subdirectory(path, exp->m_export.e_path);
	return strcmp(path, exp->m_export.e_path) == 0;
}

static int
export_matches(nfs_export *exp, char *dom, char *path, struct addrinfo *ai)
{
	return path_matches(exp, path) && client_matches(exp, dom, ai);
}

/* True iff e1 is a child of e2 and e2 has crossmnt set: */
static bool subexport(struct exportent *e1, struct exportent *e2)
{
	char *p1 = e1->e_path, *p2 = e2->e_path;
	size_t l2 = strlen(p2);

	return e2->e_flags & NFSEXP_CROSSMOUNT
		&& strncmp(p1, p2, l2) == 0
		&& p1[l2] == '/';
}

struct parsed_fsid {
	int fsidtype;
	/* We could use a union for this, but it would be more
	 * complicated; why bother? */
	unsigned int inode;
	unsigned int minor;
	unsigned int major;
	unsigned int fsidnum;
	size_t uuidlen;
	char *fhuuid;
};

static int parse_fsid(int fsidtype, int fsidlen, char *fsid,
		struct parsed_fsid *parsed)
{
	unsigned int dev;
	unsigned long long inode64;

	memset(parsed, 0, sizeof(*parsed));
	parsed->fsidtype = fsidtype;
	switch(fsidtype) {
	case FSID_DEV: /* 4 bytes: 2 major, 2 minor, 4 inode */
		if (fsidlen != 8)
			return -1;
		memcpy(&dev, fsid, 4);
		memcpy(&parsed->inode, fsid+4, 4);
		parsed->major = ntohl(dev)>>16;
		parsed->minor = ntohl(dev) & 0xFFFF;
		break;

	case FSID_NUM: /* 4 bytes - fsid */
		if (fsidlen != 4)
			return -1;
		memcpy(&parsed->fsidnum, fsid, 4);
		break;

	case FSID_MAJOR_MINOR: /* 12 bytes: 4 major, 4 minor, 4 inode 
		 * This format is never actually used but was
		 * an historical accident
		 */
		if (fsidlen != 12)
			return -1;
		memcpy(&dev, fsid, 4);
		parsed->major = ntohl(dev);
		memcpy(&dev, fsid+4, 4);
		parsed->minor = ntohl(dev);
		memcpy(&parsed->inode, fsid+8, 4);
		break;

	case FSID_ENCODE_DEV: /* 8 bytes: 4 byte packed device number, 4 inode */
		/* This is *host* endian, not net-byte-order, because
		 * no-one outside this host has any business interpreting it
		 */
		if (fsidlen != 8)
			return -1;
		memcpy(&dev, fsid, 4);
		memcpy(&parsed->inode, fsid+4, 4);
		parsed->major = (dev & 0xfff00) >> 8;
		parsed->minor = (dev & 0xff) | ((dev >> 12) & 0xfff00);
		break;

	case FSID_UUID4_INUM: /* 4 byte inode number and 4 byte uuid */
		if (fsidlen != 8)
			return -1;
		memcpy(&parsed->inode, fsid, 4);
		parsed->uuidlen = 4;
		parsed->fhuuid = fsid+4;
		break;
	case FSID_UUID8: /* 8 byte uuid */
		if (fsidlen != 8)
			return -1;
		parsed->uuidlen = 8;
		parsed->fhuuid = fsid;
		break;
	case FSID_UUID16: /* 16 byte uuid */
		if (fsidlen != 16)
			return -1;
		parsed->uuidlen = 16;
		parsed->fhuuid = fsid;
		break;
	case FSID_UUID16_INUM: /* 8 byte inode number and 16 byte uuid */
		if (fsidlen != 24)
			return -1;
		memcpy(&inode64, fsid, 8);
		parsed->inode = inode64;
		parsed->uuidlen = 16;
		parsed->fhuuid = fsid+8;
		break;
	}
	return 0;
}

static bool match_fsid(struct parsed_fsid *parsed, nfs_export *exp, char *path)
{
	struct stat stb;
	int type;
	char u[16];

	if (stat(path, &stb) != 0)
		return false;
	if (!S_ISDIR(stb.st_mode) && !S_ISREG(stb.st_mode))
		return false;

	switch (parsed->fsidtype) {
	case FSID_DEV:
	case FSID_MAJOR_MINOR:
	case FSID_ENCODE_DEV:
		if (stb.st_ino != parsed->inode)
			return false;
		if (parsed->major != major(stb.st_dev) ||
		    parsed->minor != minor(stb.st_dev))
			return false;
		return true;
	case FSID_NUM:
		if (((exp->m_export.e_flags & NFSEXP_FSID) == 0 ||
		     exp->m_export.e_fsid != parsed->fsidnum))
			return false;
		return true;
	case FSID_UUID4_INUM:
	case FSID_UUID16_INUM:
		if (stb.st_ino != parsed->inode)
			return false;
		goto check_uuid;
	case FSID_UUID8:
	case FSID_UUID16:
		if (!is_mountpoint(path))
			return false;
	check_uuid:
		if (exp->m_export.e_uuid)
			get_uuid(exp->m_export.e_uuid, parsed->uuidlen, u);
		else
			for (type = 0;
			     uuid_by_path(path, type, parsed->uuidlen, u);
			     type++)
				if (memcmp(u, parsed->fhuuid, parsed->uuidlen) == 0)
					return true;

		if (memcmp(u, parsed->fhuuid, parsed->uuidlen) != 0)
			return false;
		return true;
	}
	/* Well, unreachable, actually: */
	return false;
}

static struct addrinfo *lookup_client_addr(char *dom)
{
	struct addrinfo *ret;
	struct addrinfo *tmp;

	dom++; /* skip initial "$" */

	tmp = host_pton(dom);
	if (tmp == NULL)
		return NULL;
	ret = client_resolve(tmp->ai_addr);
	freeaddrinfo(tmp);
	return ret;
}

static void nfsd_fh(FILE *f)
{
	/* request are:
	 *  domain fsidtype fsid
	 * interpret fsid, find export point and options, and write:
	 *  domain fsidtype fsid expiry path
	 */
	char *cp;
	char *dom;
	int fsidtype;
	int fsidlen;
	char fsid[32];
	struct parsed_fsid parsed;
	struct exportent *found = NULL;
	struct addrinfo *ai = NULL;
	char *found_path = NULL;
	nfs_export *exp;
	int i;
	int dev_missing = 0;

	if (readline(fileno(f), &lbuf, &lbuflen) != 1)
		return;

	xlog(D_CALL, "nfsd_fh: inbuf '%s'", lbuf);

	cp = lbuf;

	dom = malloc(strlen(cp));
	if (dom == NULL)
		return;
	if (qword_get(&cp, dom, strlen(cp)) <= 0)
		goto out;
	if (qword_get_int(&cp, &fsidtype) != 0)
		goto out;
	if (fsidtype < 0 || fsidtype > 7)
		goto out; /* unknown type */
	if ((fsidlen = qword_get(&cp, fsid, 32)) <= 0)
		goto out;
	if (parse_fsid(fsidtype, fsidlen, fsid, &parsed))
		goto out;

	auth_reload();

	if (is_ipaddr_client(dom)) {
		ai = lookup_client_addr(dom);
		if (!ai)
			goto out;
	}

	/* Now determine export point for this fsid/domain */
	for (i=0 ; i < MCL_MAXTYPES; i++) {
		nfs_export *next_exp;
		for (exp = exportlist[i].p_head; exp; exp = next_exp) {
			char *path;

			if (exp->m_export.e_flags & NFSEXP_CROSSMOUNT) {
				static nfs_export *prev = NULL;
				static void *mnt = NULL;
				
				if (prev == exp) {
					/* try a submount */
					path = next_mnt(&mnt, exp->m_export.e_path);
					if (!path) {
						next_exp = exp->m_next;
						prev = NULL;
						continue;
					}
					next_exp = exp;
				} else {
					prev = exp;
					mnt = NULL;
					path = exp->m_export.e_path;
					next_exp = exp;
				}
			} else {
				path = exp->m_export.e_path;
				next_exp = exp->m_next;
			}

			if (!is_ipaddr_client(dom)
					&& !namelist_client_matches(exp, dom))
				continue;
			if (exp->m_export.e_mountpoint &&
			    !is_mountpoint(exp->m_export.e_mountpoint[0]?
					   exp->m_export.e_mountpoint:
					   exp->m_export.e_path))
				dev_missing ++;

			if (!match_fsid(&parsed, exp, path))
				continue;
			if (is_ipaddr_client(dom)
					&& !ipaddr_client_matches(exp, ai))
				continue;
			if (!found || subexport(&exp->m_export, found)) {
				found = &exp->m_export;
				free(found_path);
				found_path = strdup(path);
				if (found_path == NULL)
					goto out;
			} else if (strcmp(found->e_path, exp->m_export.e_path) != 0
				   && !subexport(found, &exp->m_export))
			{
				xlog(L_WARNING, "%s and %s have same filehandle for %s, using first",
				     found_path, path, dom);
			} else {
				/* same path, if one is V4ROOT, choose the other */
				if (found->e_flags & NFSEXP_V4ROOT) {
					found = &exp->m_export;
					free(found_path);
					found_path = strdup(path);
					if (found_path == NULL)
						goto out;
				}
			}
		}
	}
	if (found && 
	    found->e_mountpoint &&
	    !is_mountpoint(found->e_mountpoint[0]?
			   found->e_mountpoint:
			   found->e_path)) {
		/* Cannot export this yet 
		 * should log a warning, but need to rate limit
		   xlog(L_WARNING, "%s not exported as %d not a mountpoint",
		   found->e_path, found->e_mountpoint);
		 */
		/* FIXME we need to make sure we re-visit this later */
		goto out;
	}
	if (!found && dev_missing) {
		/* The missing dev could be what we want, so just be
		 * quite rather than returning stale yet
		 */
		goto out;
	}

	if (found)
		if (cache_export_ent(dom, found, found_path) < 0)
			found = 0;

	qword_print(f, dom);
	qword_printint(f, fsidtype);
	qword_printhex(f, fsid, fsidlen);
	/* The fsid -> path lookup can be quite expensive as it
	 * potentially stats and reads lots of devices, and some of those
	 * might have spun-down.  The Answer is not likely to
	 * change underneath us, and an 'exportfs -f' can always
	 * remove this from the kernel, so use a really log
	 * timeout.  Maybe this should be configurable on the command
	 * line.
	 */
	qword_printint(f, 0x7fffffff);
	if (found)
		qword_print(f, found_path);
	qword_eol(f);
 out:
	if (found_path)
		free(found_path);
	freeaddrinfo(ai);
	free(dom);
	xlog(D_CALL, "nfsd_fh: found %p path %s", found, found ? found->e_path : NULL);
	return;		
}

static void write_fsloc(FILE *f, struct exportent *ep)
{
	struct servers *servers;

	if (ep->e_fslocmethod == FSLOC_NONE)
		return;

	servers = replicas_lookup(ep->e_fslocmethod, ep->e_fslocdata);
	if (!servers)
		return;
	qword_print(f, "fsloc");
	qword_printint(f, servers->h_num);
	if (servers->h_num >= 0) {
		int i;
		for (i=0; i<servers->h_num; i++) {
			qword_print(f, servers->h_mp[i]->h_host);
			qword_print(f, servers->h_mp[i]->h_path);
		}
	}
	qword_printint(f, servers->h_referral);
	release_replicas(servers);
}

static void write_secinfo(FILE *f, struct exportent *ep, int flag_mask)
{
	struct sec_entry *p;

	for (p = ep->e_secinfo; p->flav; p++)
		; /* Do nothing */
	if (p == ep->e_secinfo) {
		/* There was no sec= option */
		return;
	}
	fix_pseudoflavor_flags(ep);
	qword_print(f, "secinfo");
	qword_printint(f, p - ep->e_secinfo);
	for (p = ep->e_secinfo; p->flav; p++) {
		qword_printint(f, p->flav->fnum);
		qword_printint(f, p->flags & flag_mask);
	}

}

static int dump_to_cache(FILE *f, char *domain, char *path, struct exportent *exp)
{
	qword_print(f, domain);
	qword_print(f, path);
	if (exp) {
		int different_fs = strcmp(path, exp->e_path) != 0;
		int flag_mask = different_fs ? ~NFSEXP_FSID : ~0;

		qword_printtimefrom(f, exp->e_ttl);
		qword_printint(f, exp->e_flags & flag_mask);
		qword_printint(f, exp->e_anonuid);
		qword_printint(f, exp->e_anongid);
		qword_printint(f, exp->e_fsid);
		write_fsloc(f, exp);
		write_secinfo(f, exp, flag_mask);
 		if (exp->e_uuid == NULL || different_fs) {
 			char u[16];
 			if (uuid_by_path(path, 0, 16, u)) {
 				qword_print(f, "uuid");
 				qword_printhex(f, u, 16);
 			}
 		} else {
 			char u[16];
 			get_uuid(exp->e_uuid, 16, u);
 			qword_print(f, "uuid");
 			qword_printhex(f, u, 16);
 		}
	} else
		qword_printtimefrom(f, DEFAULT_TTL);
	return qword_eol(f);
}

static nfs_export *
lookup_export(char *dom, char *path, struct addrinfo *ai)
{
	nfs_export *exp;
	nfs_export *found = NULL;
	int found_type = 0;
	int i;

	for (i=0 ; i < MCL_MAXTYPES; i++) {
		for (exp = exportlist[i].p_head; exp; exp = exp->m_next) {
			if (!export_matches(exp, dom, path, ai))
				continue;
			if (!found) {
				found = exp;
				found_type = i;
				continue;
			}
			/* Always prefer non-V4ROOT exports */
			if (exp->m_export.e_flags & NFSEXP_V4ROOT)
				continue;
			if (found->m_export.e_flags & NFSEXP_V4ROOT) {
				found = exp;
				found_type = i;
				continue;
			}

			/* If one is a CROSSMOUNT, then prefer the longest path */
			if (((found->m_export.e_flags & NFSEXP_CROSSMOUNT) ||
			     (exp->m_export.e_flags & NFSEXP_CROSSMOUNT)) &&
			    strlen(found->m_export.e_path) !=
			    strlen(exp->m_export.e_path)) {

				if (strlen(exp->m_export.e_path) >
				    strlen(found->m_export.e_path)) {
					found = exp;
					found_type = i;
				}
				continue;

			} else if (found_type == i && found->m_warned == 0) {
				xlog(L_WARNING, "%s exported to both %s and %s, "
				     "arbitrarily choosing options from first",
				     path, found->m_client->m_hostname, exp->m_client->m_hostname,
				     dom);
				found->m_warned = 1;
			}
		}
	}
	return found;
}

#ifdef HAVE_NFS_PLUGIN_H
#include <dlfcn.h>
#include <link.h>
#include <nfs-plugin.h>

/*
 * Find the export entry for the parent of "pathname".
 * Caller must not free returned exportent.
 */
static struct exportent *lookup_parent_export(char *dom,
		const char *pathname, struct addrinfo *ai)
{
	char *parent, *slash;
	nfs_export *result;

	parent = strdup(pathname);
	if (parent == NULL) {
		xlog(D_GENERAL, "%s: failed to allocate parent path buffer",
			__func__);
		goto out_default;
	}
	xlog(D_CALL, "%s: pathname = '%s'", __func__, pathname);

again:
	/* shorten pathname by one component */
	slash = strrchr(parent, '/');
	if (slash == NULL) {
		xlog(D_GENERAL, "%s: no slash found in pathname",
			__func__);
		goto out_default;
	}
	*slash = '\0';

	if (strlen(parent) == 0) {
		result = lookup_export(dom, "/", ai);
		if (result == NULL) {
			xlog(L_ERROR, "%s: no root export found.", __func__);
			goto out_default;
		}
		goto out;
	}

	result = lookup_export(dom, parent, ai);
	if (result == NULL) {
		xlog(D_GENERAL, "%s: lookup_export(%s) found nothing",
			__func__, parent);
		goto again;
	}

out:
	xlog(D_CALL, "%s: found export for %s", __func__, parent);
	free(parent);
	return &result->m_export;

out_default:
	free(parent);
	return mkexportent("*", "/", "insecure");
}

/*
 * Walk through a set of FS locations and build an e_fslocdata string.
 * Returns true if all went to plan; otherwise, false.
 */
static bool locations_to_fslocdata(struct jp_ops *ops,
		nfs_fsloc_set_t locations, char *fslocdata,
		size_t remaining, int *ttl)
{
	char *server, *last_path, *rootpath, *ptr;
	_Bool seen = false;

	last_path = NULL;
	rootpath = NULL;
	server = NULL;
	ptr = fslocdata;
	*ttl = 0;

	for (;;) {
		enum jp_status status;
		int len;

		status = ops->jp_get_next_location(locations, &server,
							&rootpath, ttl);
		if (status == JP_EMPTY)
			break;
		if (status != JP_OK) {
			xlog(D_GENERAL, "%s: failed to parse location: %s",
				__func__, ops->jp_error(status));
			goto out_false;
		}
		xlog(D_GENERAL, "%s: Location: %s:%s",
			__func__, server, rootpath);

		if (last_path && strcmp(rootpath, last_path) == 0) {
			len = snprintf(ptr, remaining, "+%s", server);
			if (len < 0) {
				xlog(D_GENERAL, "%s: snprintf: %m", __func__);
				goto out_false;
			}
			if ((size_t)len >= remaining) {
				xlog(D_GENERAL, "%s: fslocdata buffer overflow", __func__);
				goto out_false;
			}
			remaining -= (size_t)len;
			ptr += len;
		} else {
			if (last_path == NULL)
				len = snprintf(ptr, remaining, "%s@%s",
							rootpath, server);
			else
				len = snprintf(ptr, remaining, ":%s@%s",
							rootpath, server);
			if (len < 0) {
				xlog(D_GENERAL, "%s: snprintf: %m", __func__);
				goto out_false;
			}
			if ((size_t)len >= remaining) {
				xlog(D_GENERAL, "%s: fslocdata buffer overflow",
					__func__);
				goto out_false;
			}
			remaining -= (size_t)len;
			ptr += len;
			last_path = rootpath;
		}

		seen = true;
		free(rootpath);
		free(server);
	}

	xlog(D_CALL, "%s: fslocdata='%s', ttl=%d",
		__func__, fslocdata, *ttl);
	return seen;

out_false:
	free(rootpath);
	free(server);
	return false;
}

/*
 * Duplicate the junction's parent's export options and graft in
 * the fslocdata we constructed from the locations list.
 */
static struct exportent *create_junction_exportent(struct exportent *parent,
		const char *junction, const char *fslocdata, int ttl)
{
	static struct exportent *eep;

	eep = (struct exportent *)malloc(sizeof(*eep));
	if (eep == NULL)
		goto out_nomem;

	dupexportent(eep, parent);
	strcpy(eep->e_path, junction);
	eep->e_hostname = strdup(parent->e_hostname);
	if (eep->e_hostname == NULL) {
		free(eep);
		goto out_nomem;
	}
	free(eep->e_uuid);
	eep->e_uuid = NULL;
	eep->e_ttl = (unsigned int)ttl;

	free(eep->e_fslocdata);
	eep->e_fslocdata = strdup(fslocdata);
	if (eep->e_fslocdata == NULL) {
		free(eep->e_hostname);
		free(eep);
		goto out_nomem;
	}
	eep->e_fslocmethod = FSLOC_REFER;
	return eep;

out_nomem:
	xlog(L_ERROR, "%s: No memory", __func__);
	return NULL;
}

/*
 * Walk through the set of FS locations and build an exportent.
 * Returns pointer to an exportent if "junction" refers to a junction.
 */
static struct exportent *locations_to_export(struct jp_ops *ops,
		nfs_fsloc_set_t locations, const char *junction,
		struct exportent *parent)
{
	static char fslocdata[BUFSIZ];
	int ttl;

	fslocdata[0] = '\0';
	if (!locations_to_fslocdata(ops, locations,
					fslocdata, sizeof(fslocdata), &ttl))
		return NULL;
	return create_junction_exportent(parent, junction, fslocdata, ttl);
}

/*
 * Retrieve locations information in "junction" and dump it to the
 * kernel.  Returns pointer to an exportent if "junction" refers
 * to a junction.
 */
static struct exportent *invoke_junction_ops(void *handle, char *dom,
		const char *junction, struct addrinfo *ai)
{
	struct exportent *parent, *exp = NULL;
	nfs_fsloc_set_t locations;
	enum jp_status status;
	struct jp_ops *ops;
	char *error;

	ops = (struct jp_ops *)dlsym(handle, "nfs_junction_ops");
	error = dlerror();
	if (error != NULL) {
		xlog(D_GENERAL, "%s: dlsym(jp_junction_ops): %s",
			__func__, error);
		return NULL;
	}
	if (ops->jp_api_version != JP_API_VERSION) {
		xlog(D_GENERAL, "%s: unrecognized junction API version: %u",
			__func__, ops->jp_api_version);
		return NULL;
	}

	status = ops->jp_init(false);
	if (status != JP_OK) {
		xlog(D_GENERAL, "%s: failed to resolve %s: %s",
			__func__, junction, ops->jp_error(status));
		return NULL;
	}

	status = ops->jp_get_locations(junction, &locations);
	switch (status) {
	case JP_OK:
		break;
	case JP_NOTJUNCTION:
		xlog(D_GENERAL, "%s: %s is not a junction",
			__func__, junction);
		goto out;
	default:
		xlog(L_WARNING, "Dangling junction %s: %s",
			junction, ops->jp_error(status));
		goto out;
	}

	parent = lookup_parent_export(dom, junction, ai);
	if (parent == NULL)
		goto out;

	exp = locations_to_export(ops, locations, junction, parent);

	ops->jp_put_locations(locations);

out:
	ops->jp_done();
	return exp;
}

/*
 * Load the junction plug-in, then try to resolve "pathname".
 * Returns pointer to an initialized exportent if "junction"
 * refers to a junction, or NULL if not.
 */
static struct exportent *lookup_junction(char *dom, const char *pathname,
		struct addrinfo *ai)
{
	struct exportent *exp;
	struct link_map *map;
	void *handle;

	handle = dlopen("libnfsjunct.so", RTLD_NOW);
	if (handle == NULL) {
		xlog(D_GENERAL, "%s: dlopen: %s", __func__, dlerror());
		return NULL;
	}

	if (dlinfo(handle, RTLD_DI_LINKMAP, &map) == 0)
		xlog(D_GENERAL, "%s: loaded plug-in %s",
			__func__, map->l_name);

	(void)dlerror();	/* Clear any error */

	exp = invoke_junction_ops(handle, dom, pathname, ai);

	/* We could leave it loaded to make junction resolution
	 * faster next time.  However, if we want to replace the
	 * library, that would require restarting mountd. */
	(void)dlclose(handle);
	return exp;
}

static void lookup_nonexport(FILE *f, char *dom, char *path,
		struct addrinfo *ai)
{
	struct exportent *eep;

	eep = lookup_junction(dom, path, ai);
	dump_to_cache(f, dom, path, eep);
	if (eep == NULL)
		return;
	exportent_release(eep);
	free(eep);
}
#else	/* !HAVE_NFS_PLUGIN_H */
static void lookup_nonexport(FILE *f, char *dom, char *path,
		struct addrinfo *UNUSED(ai))
{
	dump_to_cache(f, dom, path, NULL);
}
#endif	/* !HAVE_NFS_PLUGIN_H */

static void nfsd_export(FILE *f)
{
	/* requests are:
	 *  domain path
	 * determine export options and return:
	 *  domain path expiry flags anonuid anongid fsid
	 */

	char *cp;
	char *dom, *path;
	nfs_export *found = NULL;
	struct addrinfo *ai = NULL;

	if (readline(fileno(f), &lbuf, &lbuflen) != 1)
		return;

	xlog(D_CALL, "nfsd_export: inbuf '%s'", lbuf);

	cp = lbuf;
	dom = malloc(strlen(cp));
	path = malloc(strlen(cp));

	if (!dom || !path)
		goto out;

	if (qword_get(&cp, dom, strlen(lbuf)) <= 0)
		goto out;
	if (qword_get(&cp, path, strlen(lbuf)) <= 0)
		goto out;

	auth_reload();

	if (is_ipaddr_client(dom)) {
		ai = lookup_client_addr(dom);
		if (!ai)
			goto out;
	}

	found = lookup_export(dom, path, ai);

	if (found) {
		if (dump_to_cache(f, dom, path, &found->m_export) < 0) {
			xlog(L_WARNING,
			     "Cannot export %s, possibly unsupported filesystem"
			     " or fsid= required", path);
			dump_to_cache(f, dom, path, NULL);
		}
	} else
		lookup_nonexport(f, dom, path, ai);

 out:
	xlog(D_CALL, "nfsd_export: found %p path %s", found, path ? path : NULL);
	if (dom) free(dom);
	if (path) free(path);
	freeaddrinfo(ai);
}


struct {
	char *cache_name;
	void (*cache_handle)(FILE *f);
	FILE *f;
	char vbuf[RPC_CHAN_BUF_SIZE];
} cachelist[] = {
	{ "auth.unix.ip", auth_unix_ip, NULL, ""},
	{ "auth.unix.gid", auth_unix_gid, NULL, ""},
	{ "nfsd.export", nfsd_export, NULL, ""},
	{ "nfsd.fh", nfsd_fh, NULL, ""},
	{ NULL, NULL, NULL, ""}
};

extern int manage_gids;

/**
 * cache_open - prepare communications channels with kernel RPC caches
 *
 */
void cache_open(void) 
{
	int i;
	for (i=0; cachelist[i].cache_name; i++ ) {
		char path[100];
		if (!manage_gids && cachelist[i].cache_handle == auth_unix_gid)
			continue;
		sprintf(path, "/proc/net/rpc/%s/channel", cachelist[i].cache_name);
		cachelist[i].f = fopen(path, "r+");
		if (cachelist[i].f != NULL) {
			setvbuf(cachelist[i].f, cachelist[i].vbuf, _IOLBF, 
				RPC_CHAN_BUF_SIZE);
		}
	}
}

/**
 * cache_set_fds - prepare cache file descriptors for one iteration of the service loop
 * @fdset: pointer to fd_set to prepare
 */
void cache_set_fds(fd_set *fdset)
{
	int i;
	for (i=0; cachelist[i].cache_name; i++) {
		if (cachelist[i].f)
			FD_SET(fileno(cachelist[i].f), fdset);
	}
}

/**
 * cache_process_req - process any active cache file descriptors during service loop iteration
 * @fdset: pointer to fd_set to examine for activity
 */
int cache_process_req(fd_set *readfds) 
{
	int i;
	int cnt = 0;
	for (i=0; cachelist[i].cache_name; i++) {
		if (cachelist[i].f != NULL &&
		    FD_ISSET(fileno(cachelist[i].f), readfds)) {
			cnt++;
			cachelist[i].cache_handle(cachelist[i].f);
			FD_CLR(fileno(cachelist[i].f), readfds);
		}
	}
	return cnt;
}


/*
 * Give IP->domain and domain+path->options to kernel
 * % echo nfsd $IP  $[now+DEFAULT_TTL] $domain > /proc/net/rpc/auth.unix.ip/channel
 * % echo $domain $path $[now+DEFAULT_TTL] $options $anonuid $anongid $fsid > /proc/net/rpc/nfsd.export/channel
 */

static int cache_export_ent(char *domain, struct exportent *exp, char *path)
{
	int err;
	FILE *f = fopen("/proc/net/rpc/nfsd.export/channel", "w");
	if (!f)
		return -1;

	err = dump_to_cache(f, domain, exp->e_path, exp);
	if (err) {
		xlog(L_WARNING,
		     "Cannot export %s, possibly unsupported filesystem or"
		     " fsid= required", exp->e_path);
	}

	while (err == 0 && (exp->e_flags & NFSEXP_CROSSMOUNT) && path) {
		/* really an 'if', but we can break out of
		 * a 'while' more easily */
		/* Look along 'path' for other filesystems
		 * and export them with the same options
		 */
		struct stat stb;
		size_t l = strlen(exp->e_path);
		__dev_t dev;

		if (strlen(path) <= l || path[l] != '/' ||
		    strncmp(exp->e_path, path, l) != 0)
			break;
		if (stat(exp->e_path, &stb) != 0)
			break;
		dev = stb.st_dev;
		while(path[l] == '/') {
			char c;
			/* errors for submount should fail whole filesystem */
			int err2;

			l++;
			while (path[l] != '/' && path[l])
				l++;
			c = path[l];
			path[l] = 0;
			err2 = lstat(path, &stb);
			path[l] = c;
			if (err2 < 0)
				break;
			if (stb.st_dev == dev)
				continue;
			dev = stb.st_dev;
			path[l] = 0;
			dump_to_cache(f, domain, path, exp);
			path[l] = c;
		}
		break;
	}

	fclose(f);
	return err;
}

/**
 * cache_export - Inform kernel of a new nfs_export
 * @exp: target nfs_export
 * @path: NUL-terminated C string containing export path
 */
int cache_export(nfs_export *exp, char *path)
{
	char buf[INET6_ADDRSTRLEN];
	int err;
	FILE *f;

	f = fopen("/proc/net/rpc/auth.unix.ip/channel", "w");
	if (!f)
		return -1;


	qword_print(f, "nfsd");
	qword_print(f,
		host_ntop(get_addrlist(exp->m_client, 0), buf, sizeof(buf)));
	qword_printtimefrom(f, exp->m_export.e_ttl);
	qword_print(f, exp->m_client->m_hostname);
	err = qword_eol(f);
	
	fclose(f);

	err = cache_export_ent(exp->m_client->m_hostname, &exp->m_export, path)
		|| err;
	return err;
}

/**
 * cache_get_filehandle - given an nfs_export, get its root filehandle
 * @exp: target nfs_export
 * @len: length of requested file handle
 * @p: NUL-terminated C string containing export path
 *
 * Returns pointer to NFS file handle of root directory of export
 *
 * { 
 *   echo $domain $path $length 
 *   read filehandle <&0
 * } <> /proc/fs/nfsd/filehandle
 */
struct nfs_fh_len *
cache_get_filehandle(nfs_export *exp, int len, char *p)
{
	FILE *f = fopen("/proc/fs/nfsd/filehandle", "r+");
	char buf[200];
	char *bp = buf;
	int failed;
	static struct nfs_fh_len fh;

	if (!f)
		f = fopen("/proc/fs/nfs/filehandle", "r+");
	if (!f)
		return NULL;

	qword_print(f, exp->m_client->m_hostname);
	qword_print(f, p);
	qword_printint(f, len);	
	failed = qword_eol(f);
	
	if (!failed)
		failed = (fgets(buf, sizeof(buf), f) == NULL);
	fclose(f);
	if (failed)
		return NULL;
	memset(fh.fh_handle, 0, sizeof(fh.fh_handle));
	fh.fh_size = qword_get(&bp, (char *)fh.fh_handle, NFS3_FHSIZE);
	return &fh;
}
