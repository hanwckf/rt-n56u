
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <pwd.h>
#include <grp.h>
#include <keyutils.h>
#include <nfsidmap.h>

#include <unistd.h>
#include "xlog.h"
#include "conffile.h"

int verbose = 0;
char *usage="Usage: %s [-v] [-c || [-u|-g|-r key] || [-t timeout] key desc]";

#define MAX_ID_LEN   11
#define IDMAP_NAMESZ 128
#define USER  1
#define GROUP 0

#define PROCKEYS "/proc/keys"
#ifndef DEFAULT_KEYRING
#define DEFAULT_KEYRING "id_resolver"
#endif

#ifndef PATH_IDMAPDCONF
#define PATH_IDMAPDCONF "/etc/idmapd.conf"
#endif

static int keyring_clear(char *keyring);

#define UIDKEYS 0x1
#define GIDKEYS 0x2

/*
 * Find either a user or group id based on the name@domain string
 */
int id_lookup(char *name_at_domain, key_serial_t key, int type)
{
	char id[MAX_ID_LEN];
	uid_t uid = 0;
	gid_t gid = 0;
	int rc;

	if (type == USER) {
		rc = nfs4_owner_to_uid(name_at_domain, &uid);
		sprintf(id, "%u", uid);
	} else {
		rc = nfs4_group_owner_to_gid(name_at_domain, &gid);
		sprintf(id, "%u", gid);
	}
	if (rc < 0)
		xlog_err("id_lookup: %s: failed: %m",
			(type == USER ? "nfs4_owner_to_uid" : "nfs4_group_owner_to_gid"));

	if (rc == 0) {
		rc = keyctl_instantiate(key, id, strlen(id) + 1, 0);
		if (rc < 0) {
			switch(rc) {
			case -EDQUOT:
			case -ENFILE:
			case -ENOMEM:
				/*
			 	 * The keyring is full. Clear the keyring and try again
			 	 */
				rc = keyring_clear(DEFAULT_KEYRING);
				if (rc == 0)
					rc = keyctl_instantiate(key, id, strlen(id) + 1, 0);
				break;
			default:
				break;
			}
		}
		if (rc < 0)
			xlog_err("id_lookup: keyctl_instantiate failed: %m");
	}

	return rc;
}

/*
 * Find the name@domain string from either a user or group id
 */
int name_lookup(char *id, key_serial_t key, int type)
{
	char name[IDMAP_NAMESZ];
	char domain[NFS4_MAX_DOMAIN_LEN];
	uid_t uid;
	gid_t gid;
	int rc;

	rc = nfs4_get_default_domain(NULL, domain, NFS4_MAX_DOMAIN_LEN);
	if (rc != 0) {
		rc = -1;
		xlog_err("name_lookup: nfs4_get_default_domain failed: %m");
		goto out;
	}

	if (type == USER) {
		uid = atoi(id);
		rc = nfs4_uid_to_name(uid, domain, name, IDMAP_NAMESZ);
	} else {
		gid = atoi(id);
		rc = nfs4_gid_to_name(gid, domain, name, IDMAP_NAMESZ);
	}
	if (rc < 0)
		xlog_err("name_lookup: %s: failed: %m",
			(type == USER ? "nfs4_uid_to_name" : "nfs4_gid_to_name"));

	if (rc == 0) {
		rc = keyctl_instantiate(key, &name, strlen(name), 0);
		if (rc < 0)
			xlog_err("name_lookup: keyctl_instantiate failed: %m");
	}
out:
	return rc;
}
/*
 * Clear all the keys on the given keyring
 */
static int keyring_clear(char *keyring)
{
	FILE *fp;
	char buf[BUFSIZ];
	key_serial_t key;

	if (keyring == NULL)
		keyring = DEFAULT_KEYRING;

	if ((fp = fopen(PROCKEYS, "r")) == NULL) {
		xlog_err("fopen(%s) failed: %m", PROCKEYS);
		return 1;
	}

	while(fgets(buf, BUFSIZ, fp) != NULL) {
		if (strstr(buf, "keyring") == NULL)
			continue;
		if (strstr(buf, keyring) == NULL)
			continue;
		if (verbose) {
			*(strchr(buf, '\n')) = '\0';
			xlog_warn("clearing '%s'", buf);
		}
		/*
		 * The key is the first arugment in the string
		 */
		*(strchr(buf, ' ')) = '\0';
		sscanf(buf, "%x", &key);
		if (keyctl_clear(key) < 0) {
			xlog_err("keyctl_clear(0x%x) failed: %m", key);
			fclose(fp);
			return 1;
		}
		fclose(fp);
		return 0;
	}
	xlog_err("'%s' keyring was not found.", keyring);
	fclose(fp);
	return 1;
}
/*
 * Revoke a key 
 */
static int key_revoke(char *keystr, int keymask)
{
	FILE *fp;
	char buf[BUFSIZ], *ptr;
	key_serial_t key;
	int mask;

	xlog_syslog(0);

	if ((fp = fopen(PROCKEYS, "r")) == NULL) {
		xlog_err("fopen(%s) failed: %m", PROCKEYS);
		return 1;
	}

	while(fgets(buf, BUFSIZ, fp) != NULL) {
		if (strstr(buf, "keyring") != NULL)
			continue;

		mask = 0;
		if ((ptr = strstr(buf, "uid:")) != NULL)
			mask = UIDKEYS;
		else if ((ptr = strstr(buf, "gid:")) != NULL)
			mask = GIDKEYS;
		else 
			continue;

		if ((keymask & mask) == 0)
			continue;

		if (strncmp(ptr+4, keystr, strlen(keystr)) != 0)
			continue;

		if (verbose) {
			*(strchr(buf, '\n')) = '\0';
			xlog_warn("revoking '%s'", buf);
		}
		/*
		 * The key is the first arugment in the string
		 */
		*(strchr(buf, ' ')) = '\0';
		sscanf(buf, "%x", &key);

		if (keyctl_revoke(key) < 0) {
			xlog_err("keyctl_revoke(0x%x) failed: %m", key);
			fclose(fp);
			return 1;
		}

		keymask &= ~mask;
		if (keymask == 0) {
			fclose(fp);
			return 0;
		}
	}
	xlog_err("'%s' key was not found.", keystr);
	fclose(fp);
	return 1;
}

int main(int argc, char **argv)
{
	char *arg;
	char *value;
	char *type;
	int rc = 1, opt;
	int timeout = 600;
	key_serial_t key;
	char *progname, *keystr = NULL;
	int clearing = 0, keymask = 0;

	/* Set the basename */
	if ((progname = strrchr(argv[0], '/')) != NULL)
		progname++;
	else
		progname = argv[0];

	xlog_open(progname);

	while ((opt = getopt(argc, argv, "u:g:r:ct:v")) != -1) {
		switch (opt) {
		case 'u':
			keymask = UIDKEYS;
			keystr = strdup(optarg);
			break;
		case 'g':
			keymask = GIDKEYS;
			keystr = strdup(optarg);
			break;
		case 'r':
			keymask = GIDKEYS|UIDKEYS;
			keystr = strdup(optarg);
			break;
		case 'c':
			clearing++;
			break;
		case 'v':
			verbose++;
			break;
		case 't':
			timeout = atoi(optarg);
			break;
		default:
			xlog_warn(usage, progname);
			break;
		}
	}

	if (nfs4_init_name_mapping(PATH_IDMAPDCONF))  {
		xlog_err("Unable to create name to user id mappings.");
		return 1;
	}
	if (!verbose)
		verbose = conf_get_num("General", "Verbosity", 0);

	if (keystr) {
		rc = key_revoke(keystr, keymask);
		return rc;		
	}
	if (clearing) {
		xlog_syslog(0);
		rc = keyring_clear(DEFAULT_KEYRING);
		return rc;		
	}

	xlog_stderr(0);
	if ((argc - optind) != 2) {
		xlog_err("Bad arg count. Check /etc/request-key.conf");
		xlog_warn(usage, progname);
		return 1;
	}

	if (verbose)
		nfs4_set_debug(verbose, NULL);

	key = strtol(argv[optind++], NULL, 10);

	arg = strdup(argv[optind]);
	if (arg == NULL) {
		xlog_err("strdup failed: %m");
		return 1;
	}
	type = strtok(arg, ":");
	value = strtok(NULL, ":");

	if (verbose) {
		xlog_warn("key: 0x%lx type: %s value: %s timeout %ld",
			key, type, value, timeout);
	}

	if (strcmp(type, "uid") == 0)
		rc = id_lookup(value, key, USER);
	else if (strcmp(type, "gid") == 0)
		rc = id_lookup(value, key, GROUP);
	else if (strcmp(type, "user") == 0)
		rc = name_lookup(value, key, USER);
	else if (strcmp(type, "group") == 0)
		rc = name_lookup(value, key, GROUP);

	/* Set timeout to 10 (600 seconds) minutes */
	if (rc == 0)
		keyctl_set_timeout(key, timeout);

	free(arg);
	return rc;
}
