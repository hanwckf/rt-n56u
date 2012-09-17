#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
#include <stdlib.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <linux/netfilter_ipv4/ip_pool.h>
#include <libippool/ip_pool_support.h>

/* translate errnos, as returned by our *sockopt() functions */
static char *errno2msg(int op, int err)
{
	switch(err) {
	case ERANGE:
		return "Address out of pool range";
	}
	return strerror(err);
}

static void usage(char *msg)
{
	if (msg) fprintf(stderr, "ERROR: %s\n", msg);
	fprintf(stderr,
"Usage: ippool [-LADCNF] [POOL] [args]\n"
"The POOL argument is either a number, or a name from %s\n"
, IPPOOL_CONF);
	fprintf(stderr,
"ippool [-n] [-l|-L POOL] [-u] [-v]\n"
	"\tLists all (-l), or a single (-L) pool.\n"
	"\tWith -u, summarized usage counts are listed.\n"
	"\tWith -v, each pool membership is shown, one per line.\n"
"ippool [-n] [-f|-F [POOL]]\n"
	"\tflushes POOL (or all pools.)\n"
"ippool [-n] [-x|-X [POOL]]\n"
	"\tremoves POOL (or all pools.)\n"
"ippool [-n] -B\n"
	"\tcreates all fully specified pools found in the config file.\n"
"ippool [-n] -N POOL [-t type] [FIRST LAST]\n"
	"\tcreates POOL of IP addresses FIRST to LAST (inclusive).  If a\n"
	"\tpool name from the config file %s is used, type and\n"
	"\taddress information can be defined there.  The -t argument\n"
	"\tgives the type (default bitmap).\n"
"ippool [-n] -A POOL ADDR\n"
	"\tadds ADDR to POOL\n"
"ippool [-n] -D POOL ADDR\n"
	"\tremoves ADDR from POOL\n"
"ippool [-n] -C POOL ADDR\n"
	"\ttests ADDR against membership in POOL\n"
, IPPOOL_CONF);
	exit(1);
}

/* config file parsing */

#define IP_POOL_T_NONE		0
#define IP_POOL_T_BITMAP	1

static int conf_type = IP_POOL_T_NONE;
static unsigned long conf_addr = 0;
static unsigned long conf_addr2 = 0;

#define SCAN_EOF (IP_POOL_NONE-1)

static ip_pool_t get_index_line(
	FILE *fp,
	char **namep,
	char **typep,
	char **argp
) {
	char *p;
	ip_pool_t index;
	static char buf[256];

	if (namep) *namep = 0;
	if (typep) *typep = 0;
	if (argp) *argp = 0;

	if (!fgets(buf, sizeof(buf), fp)) return SCAN_EOF;

	p = strtok(buf, " \t\n");
	if (!p || *p == '#') return IP_POOL_NONE;
	index = atoi(p);

	p = strtok(0, " \t\n");
	if (!p || *p == '#') return index;
	if (namep) *namep = p;

	p = strtok(0, " \t\n");
	if (!p || *p == '#') return index;
	if (typep) *typep = p;

	p = strtok(0, "#\n");
	if (argp) *argp = p;

	return index;
}

static ip_pool_t get_index(char *name)
{
	FILE *fp;
	char *poolname, *type, *arg, *p;
	ip_pool_t res;

	if (isdigit(*name))
		return atoi(name);
	fp = fopen(IPPOOL_CONF, "r");
	if (!fp) {
		fprintf(stderr, "cannot open %s - no pool names", IPPOOL_CONF);
		exit(1);
	}
	while (SCAN_EOF != (res=get_index_line(fp, &poolname, &type, &arg))) {
		if (poolname && 0 == strcmp(poolname, name)) {
			if (!type || (0 == strcmp(type, "bitmap"))) {
				conf_type = IP_POOL_T_BITMAP;
				p = strtok(arg, " \t");
				if (p) {
					conf_addr = inet_addr(p);
					p = strtok(0, " \t");
					if (p)
						conf_addr2 = inet_addr(p);
					else
						conf_addr = 0;
				}
			}
			break;
		}
	}
	fclose(fp);
	if (res == SCAN_EOF) {
		fprintf(stderr, "pool '%s' not found in %s\n",
			name, IPPOOL_CONF);
		exit(1);
	}
	return res;
}

static char *get_name(ip_pool_t index)
{
	FILE *fp;
	static char sbuf[256];
	int ok = 0;

	fp = fopen(IPPOOL_CONF, "r");
	if (fp) {
		while (fgets(sbuf, sizeof(sbuf), fp)) {
			char *p = strtok(sbuf, " \t\n");

			if (!p || *p == '#') continue;
			if (index != atoi(p)) continue;
			p = strtok(0, " \t\n");
			if (!p || *p == '#') continue;
			memmove(sbuf, p, strlen(p)+1);
			ok = 1;
			break;
		}
		fclose(fp);
	}
	if (!ok) sprintf(sbuf, "%d", index);
	return sbuf;
}

/* user/kernel interaction */

static int fd = -1;
static int flag_n = 0;		/* do not do anything; just brag about it */
static int flag_v = 0;		/* be verbose (list members) */
static int flag_q = 0;		/* be quiet */
static int flag_u = 0;		/* show usage counts in listings */
static char *flag_t = "bitmap";	/* pool type */

static ip_pool_t high_nr(void)
{
	struct ip_pool_request req;
	int reqlen = sizeof(req);

	req.op = IP_POOL_HIGH_NR;
	if (0 > getsockopt(fd, SOL_IP, SO_IP_POOL, &req, &reqlen)) {
		fprintf(stderr,
			"IP_POOL_HIGH_NR failed: %s\n",
			errno2msg(IP_POOL_HIGH_NR, errno));
		exit(1);
	}
	return req.index;
}

static void do_list(ip_pool_t pool)
{
	struct ip_pool_request req;
	int reqlen = sizeof(req);
	u_int32_t first_ip;
	u_int32_t last_ip;

	req.op = IP_POOL_LOOKUP;
	req.index = pool;
	req.addr = req.addr2 = 0;
	reqlen = sizeof(req);
	if (0 == getsockopt(fd, SOL_IP, SO_IP_POOL, &req, &reqlen)) {
		struct in_addr ina;
		ina.s_addr = req.addr;
		printf("%s %s", get_name(req.index), inet_ntoa(ina));
		ina.s_addr = req.addr2;
		printf(" %s", inet_ntoa(ina));
		first_ip = ntohl(req.addr);
		last_ip = ntohl(req.addr2);
		if (flag_u) {
			req.op = IP_POOL_USAGE;
			req.index = pool;
			reqlen = sizeof(req);
			if (0 == getsockopt(fd, SOL_IP, SO_IP_POOL,
						&req, &reqlen)) {
				printf(" %d %d", req.addr, req.addr2);
			} else {
				printf(" - -");
			}
		}
		printf("\n");
		if (flag_v) {
			while (first_ip <= last_ip) {
				req.op = IP_POOL_TEST_ADDR;
				req.index = pool;
				ina.s_addr = req.addr = htonl(first_ip);
				reqlen = sizeof(req);
				if (0 == getsockopt(fd, SOL_IP, SO_IP_POOL,
							&req, &reqlen)) {
					if (req.addr) printf("\t%s\n",
							inet_ntoa(ina));
				}
				first_ip++;
			}
		}
	}
}

static void do_list_all(void)
{
	ip_pool_t i, highest = high_nr();

	for (i=0; i<=highest; i++)
		do_list(i);
}

static void do_flush(ip_pool_t pool)
{
	struct ip_pool_request req;
	int reqlen = sizeof(req);

	req.op = IP_POOL_FLUSH;
	req.index = pool;
	if (flag_n) {
		printf("ippool -F %d\n", req.index);
		return;
	}
	if (0 > getsockopt(fd, SOL_IP, SO_IP_POOL, &req, &reqlen)) {
		int err = errno;
		fprintf(stderr,
			"IP_POOL_FLUSH %s failed: %s\n",
			get_name(pool), errno2msg(IP_POOL_FLUSH, err));
		exit(1);
	}
}

static void do_flush_all(void)
{
	ip_pool_t i, highest = high_nr();

	for (i=0; i<=highest; i++)
		do_flush(i);
}

static void do_destroy(ip_pool_t pool)
{
	struct ip_pool_request req;
	int reqlen = sizeof(req);

	req.op = IP_POOL_DESTROY;
	req.index = pool;
	if (flag_n) {
		printf("ippool -X %d\n", req.index);
		return;
	}
	if (0 > getsockopt(fd, SOL_IP, SO_IP_POOL, &req, &reqlen)) {
		int err = errno;
		fprintf(stderr,
			"IP_POOL_DESTROY %s failed: %s\n",
			get_name(pool), errno2msg(IP_POOL_DESTROY, err));
		exit(1);
	}
}

static void do_destroy_all(void)
{
	ip_pool_t i, highest = high_nr();

	for (i=0; i<=highest; i++)
		do_destroy(i);
}

static int do_adddel(ip_pool_t pool, char *straddr, int op)
{
	struct ip_pool_request req;
	int res;
	int reqlen = sizeof(req);

	req.op = op;
	req.index = pool;
	req.addr = inet_addr(straddr);
	req.addr2 = 0;
	if (flag_n) {
		printf("ippool -%c %s %s\n",
				(op == IP_POOL_ADD_ADDR) ? 'A' : 'D',
				get_name(req.index), straddr);
		return 0;
	}
	res = getsockopt(fd, SOL_IP, SO_IP_POOL, &req, &reqlen);
	if (0 > res) {
		int err = errno;
		fprintf(stderr,
			"IP_POOL_ADD/DEL %s in %s failed: %s\n",
			straddr, get_name(pool), errno2msg(op, err));
		exit(1);
	}
	if (!flag_q)
		printf("%s %s %d %d\n", get_name(pool), straddr, req.addr,
			op == IP_POOL_ADD_ADDR ? 1 : 0);
	return req.addr;
}

static int do_check(ip_pool_t pool, char *straddr)
{
	struct ip_pool_request req;
	int reqlen = sizeof(req);

	req.op = IP_POOL_TEST_ADDR;
	req.index = pool;
	req.addr = inet_addr(straddr);
	if (0 > getsockopt(fd, SOL_IP, SO_IP_POOL, &req, &reqlen)) {
		int err = errno;
		fprintf(stderr,
			"IP_POOL_TEST_ADDR %s in %s failed: %s\n",
			straddr, get_name(pool),
			errno2msg(IP_POOL_TEST_ADDR, err));
		exit(1);
	}
	if (!flag_q)
		printf("%s %s %d\n", get_name(req.index), straddr, req.addr);
	return !req.addr;
}

static void do_new(ip_pool_t pool, int argc, char **argv)
{
	struct ip_pool_request req;
	int reqlen = sizeof(req);

	req.op = IP_POOL_INIT;
	req.index = pool;
	if (argc >= 2) {
		conf_type = IP_POOL_T_BITMAP;
		conf_addr = inet_addr(argv[0]);
		conf_addr2 = inet_addr(argv[1]);
	}
	if (conf_type != IP_POOL_T_BITMAP || conf_addr == 0 || conf_addr2 == 0)
		usage("bad pool specification");
	req.addr = conf_addr;
	req.addr2 = conf_addr2;
	if (flag_n) {
		printf("ippool -N %s [-T %s] ...\n", get_name(pool),
					conf_type == IP_POOL_T_BITMAP
						? "bitmap"
						: "???");
		return;
	}
	if (0 > getsockopt(fd, SOL_IP, SO_IP_POOL, &req, &reqlen)) {
		struct in_addr ina;
		int err = errno;
		ina.s_addr = conf_addr;
		fprintf(stderr,
			"IP_POOL_INIT %s [%s-",
			get_name(pool), inet_ntoa(ina));
		ina.s_addr = conf_addr2;
		fprintf(stderr,
			"%s] failed: %s\n",
			inet_ntoa(ina), errno2msg(IP_POOL_INIT, err));
		exit(1);
	}
}

static void do_new_all(void)
{
	FILE *fp;
	char *poolname, *type, *arg, *p;
	int res;

	fp = fopen(IPPOOL_CONF, "r");
	if (!fp) {
		fprintf(stderr, "cannot open %s - no pool names", IPPOOL_CONF);
		exit(1);
	}
	while (SCAN_EOF != (res=get_index_line(fp, &poolname, &type, &arg))) {
		if (poolname && type && (0 == strcmp(type, "bitmap"))) {
			conf_type = IP_POOL_T_BITMAP;
			p = strtok(arg, " \t");
			if (p) {
				conf_addr = inet_addr(p);
				p = strtok(0, " \t");
				if (p)
					conf_addr2 = inet_addr(p);
				else
					conf_addr = 0;
			}
			if (conf_addr != 0) {
				if (flag_v)
					printf("ippool -N %s (%s) [%s]\n",
						poolname, type, arg);
				do_new(res, 0, 0);
			}
		}
	}
	fclose(fp);
}

int main(int argc, char **argv)
{
	int opt;
	int op;
#define OP_NONE		0
#define OP_LIST		1
#define OP_LIST_ALL	2
#define OP_FLUSH	3
#define OP_FLUSH_ALL	4
#define OP_DESTROY	5
#define OP_DESTROY_ALL	6
#define OP_ADD		7
#define OP_DEL		8
#define OP_CHECK	9
#define OP_NEW		10
#define OP_NEW_ALL	11
#define OP_HIGHEST	12
	char *op_pool;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		fprintf(stderr, "cannot get DGRAM socket: %s\n",
			strerror(errno));
		exit(1);
	}
	op_pool = 0;
	op = OP_NONE;
	/* GRRR. I thought getopt() would allow an "L*" specifier, for an -L
	 * taking an optional argument. Does not work. Bad.
	 * Adding -l for -L without argument, also -f/-F and -x/-X.
	 */
	while (EOF != (opt=getopt( argc, argv, "HhnvuqA:D:C:N:t:L:F:X:lfxB")))
	switch(opt) {
		case 'l':
			if (op != OP_NONE) usage("conflicting operations");
			op = OP_LIST_ALL;
			break;
		case 'L':
			if (op != OP_NONE) usage("conflicting operations");
			op = OP_LIST;
			op_pool = optarg;
			break;
		case 'f':
			if (op != OP_NONE) usage("conflicting operations");
			op = OP_FLUSH_ALL;
			break;
		case 'F':
			if (op != OP_NONE) usage("conflicting operations");
			op = OP_FLUSH;
			op_pool = optarg;
			break;
		case 'x':
			if (op != OP_NONE) usage("conflicting operations");
			op = OP_DESTROY_ALL;
			break;
		case 'X':
			if (op != OP_NONE) usage("conflicting operations");
			op = OP_DESTROY;
			op_pool = optarg;
			break;
		case 'A':
			if (op != OP_NONE) usage("conflicting operations");
			op = OP_ADD;
			op_pool = optarg;
			break;
		case 'D':
			if (op != OP_NONE) usage("conflicting operations");
			op = OP_DEL;
			op_pool = optarg;
			break;
		case 'C':
			if (op != OP_NONE) usage("conflicting operations");
			op = OP_CHECK;
			op_pool = optarg;
			break;
		case 'B':
			if (op != OP_NONE) usage("conflicting operations");
			op = OP_NEW_ALL;
			break;
		case 'N':
			if (op != OP_NONE) usage("conflicting operations");
			op = OP_NEW;
			op_pool = optarg;
			break;
		case 'H':
			if (op != OP_NONE) usage("conflicting operations");
			op = OP_HIGHEST;
			break;
		case 't':
			flag_t = optarg;
			break;
		case 'n':
			flag_n = 1;
			break;
		case 'v':
			flag_v = 1;
			break;
		case 'u':
			flag_u = 1;
			break;
		case 'q':
			flag_q = 1;
			break;
		case 'h':
			usage(0);
		default:
			usage("bad option");
	}
	if (op == OP_NONE)
		usage("no operation specified");
	if (op == OP_LIST_ALL) {
		do_list_all();
		return 0;
	}
	if (op == OP_LIST) {
		do_list(get_index(op_pool));
		return 0;
	}
	if (op == OP_FLUSH_ALL) {
		do_flush_all();
		return 0;
	}
	if (op == OP_FLUSH) {
		do_flush(get_index(op_pool));
		return 0;
	}
	if (op == OP_DESTROY_ALL) {
		do_destroy_all();
		return 0;
	}
	if (op == OP_DESTROY) {
		do_destroy(get_index(op_pool));
		return 0;
	}
	if (op == OP_CHECK) {
		if (optind >= argc)
			usage("missing address to check");
		return do_check(get_index(op_pool), argv[optind]);
	}
	if (op == OP_NEW_ALL) {
		do_new_all();
		return 0;
	}
	if (op == OP_NEW) {
		do_new(get_index(op_pool), argc-optind, argv+optind);
		return 0;
	}
	if (op == OP_ADD) {
		if (optind >= argc)
			usage("missing address to add");
		return do_adddel(get_index(op_pool),
				argv[optind], IP_POOL_ADD_ADDR);
	}
	if (op == OP_DEL) {
		if (optind >= argc)
			usage("missing address to delete");
		return do_adddel(get_index(op_pool),
				argv[optind], IP_POOL_DEL_ADDR);
	}
	if (op == OP_HIGHEST) {
		printf("%d\n", high_nr());
		return 0;
	}
	usage("no operation specified");
	return 0;
}
