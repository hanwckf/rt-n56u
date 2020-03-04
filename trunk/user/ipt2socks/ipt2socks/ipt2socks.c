#define _GNU_SOURCE
#include "logutils.h"
#include "lrucache.h"
#include "netutils.h"
#include "protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <getopt.h>
#include <pthread.h>
#undef _GNU_SOURCE

/* option flags */
enum {
    OPTION_TCP     = 0x01 << 0, /* enable tcp */
    OPTION_UDP     = 0x01 << 1, /* enable udp */
    OPTION_IPV4    = 0x01 << 2, /* enable ipv4 */
    OPTION_IPV6    = 0x01 << 3, /* enable ipv6 */
    OPTION_DNAT    = 0x01 << 4, /* use REDIRECT instead of TPROXY (for tcp) */
    OPTION_HFCLS   = 0x01 << 5, /* gracefully close the tcp connection pair */
    OPTION_DEFAULT = OPTION_TCP | OPTION_UDP | OPTION_IPV4 | OPTION_IPV6, /* default behavior */
};

/* if verbose logging */
#define IF_VERBOSE if (g_verbose)

/* number of threads */
#define THREAD_NUMBERS_DEFAULT 1

/* udp idle timeout(sec) */
#define UDP_IDLE_TIMEO_DEFAULT 300

/* tcp socket buffer size */
#define TCP_SKBUFSIZE_MINIMUM 1024
#define TCP_SKBUFSIZE_DEFAULT 8192

/* ipt2socks bind address */
#define BIND_IPV4_DEFAULT IP4STR_LOOPBACK
#define BIND_IPV6_DEFAULT IP6STR_LOOPBACK
#define BIND_PORT_DEFAULT 60080

/* ipt2socks version string */
#define IPT2SOCKS_VERSION "ipt2socks v1.0.2 <https://github.com/zfl9/ipt2socks>"

/* tcp stream context typedef */
typedef struct {
    uv_tcp_t   *client_stream;
    uv_tcp_t   *socks5_stream;
    void       *client_buffer;
    void       *socks5_buffer;
    uv_write_t *client_wrtreq;
    uv_write_t *socks5_wrtreq;
    bool        is_half_close;
} tcpcontext_t;

/* function declaration in advance */
static void* run_event_loop(void *is_main_thread);

static void tcp_socket_listen_cb(uv_stream_t *listener, int status);
static void tcp_socks5_tcp_connect_cb(uv_connect_t *connreq, int status);
static void tcp_common_alloc_cb(uv_handle_t *stream, size_t sugsize, uv_buf_t *uvbuf);
static void tcp_socks5_auth_read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *uvbuf);
static void tcp_socks5_usrpwd_read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *uvbuf);
static void tcp_socks5_resp_read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *uvbuf);
static void tcp_stream_read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *uvbuf);
static void tcp_stream_write_cb(uv_write_t *writereq, int status);
static void tcp_stream_close_cb(uv_handle_t *stream);

static void udp_socket_listen_cb(uv_poll_t *listener, int status, int events);
static void udp_socks5_tcp_connect_cb(uv_connect_t *connreq, int status);
static void udp_socks5_tcp_alloc_cb(uv_handle_t *stream, size_t sugsize, uv_buf_t *uvbuf);
static void udp_socks5_auth_read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *uvbuf);
static void udp_socks5_usrpwd_read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *uvbuf);
static void udp_socks5_resp_read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *uvbuf);
static void udp_socks5_tcp_read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *uvbuf);
static void udp_client_alloc_cb(uv_handle_t *client, size_t sugsize, uv_buf_t *uvbuf);
static void udp_client_recv_cb(uv_udp_t *client, ssize_t nread, const uv_buf_t *uvbuf, const skaddr_t *addr, unsigned flags);
static void udp_cltentry_timer_cb(uv_timer_t *timer);
static void udp_svrentry_timer_cb(uv_timer_t *timer);
static void udp_cltentry_release(cltentry_t *entry);
static void udp_svrentry_release(svrentry_t *entry);

/* static global variable definition */
static bool        g_verbose                                = false;
static uint8_t     g_options                                = OPTION_DEFAULT;
static uint8_t     g_nthreads                               = THREAD_NUMBERS_DEFAULT;
static uint32_t    g_tcpbufsiz                              = TCP_SKBUFSIZE_DEFAULT;
static uint16_t    g_udpidletmo                             = UDP_IDLE_TIMEO_DEFAULT;

static char        g_bind_ipstr4[IP4STRLEN]                 = BIND_IPV4_DEFAULT;
static char        g_bind_ipstr6[IP6STRLEN]                 = BIND_IPV6_DEFAULT;
static portno_t    g_bind_portno                            = BIND_PORT_DEFAULT;
static skaddr4_t   g_bind_skaddr4                           = {0};
static skaddr6_t   g_bind_skaddr6                           = {0};

static bool        g_server_isipv4                          = true;
static char        g_server_ipstr[IP6STRLEN]                = {0};
static portno_t    g_server_portno                          = 0;
static skaddr6_t   g_server_skaddr                          = {0};

static uint8_t     g_usrpwd_reqbuf[SOCKS5_USRPWD_REQMAXLEN] = {0};
static uint16_t    g_usrpwd_reqlen                          = 0;

static cltcache_t *g_udp_cltcache                           = NULL;
static svrcache_t *g_udp_svrcache                           = NULL;
static char        g_udp_ipstrbuf[IP6STRLEN]                = {0};
static char        g_udp_packetbuf[UDP_PACKET_MAXSIZE]      = {0};
static char        g_udp_socks5buf[SOCKS5_HDR_MAXSIZE]      = {0};

/* socks5 authentication request (noauth/usrpwd) */
static socks5_authreq_t g_socks5_auth_request = {
    .version = SOCKS5_VERSION,
    .mlength = 1,
    .method = SOCKS5_METHOD_NOAUTH, /* noauth by default */
};

/* socks5 udp4 association request constant */
static const socks5_ipv4req_t G_SOCKS5_UDP4_REQUEST = {
    .version = SOCKS5_VERSION,
    .command = SOCKS5_COMMAND_UDPASSOCIATE,
    .reserved = 0,
    .addrtype = SOCKS5_ADDRTYPE_IPV4,
    .ipaddr4 = 0,
    .portnum = 0,
};

/* socks5 udp6 association request constant */
static const socks5_ipv6req_t G_SOCKS5_UDP6_REQUEST = {
    .version = SOCKS5_VERSION,
    .command = SOCKS5_COMMAND_UDPASSOCIATE,
    .reserved = 0,
    .addrtype = SOCKS5_ADDRTYPE_IPV6,
    .ipaddr6 = {0},
    .portnum = 0,
};

/* print command help information */
static void print_command_help(void) {
    printf("usage: ipt2socks <options...>. the existing options are as follows:\n"
           " -s, --server-addr <addr>           socks5 server ip address, <required>\n"
           " -p, --server-port <port>           socks5 server port number, <required>\n"
           " -a, --auth-username <user>         username for socks5 authentication\n"
           " -k, --auth-password <passwd>       password for socks5 authentication\n"
           " -b, --listen-addr4 <addr>          listen ipv4 address, default: 127.0.0.1\n"
           " -B, --listen-addr6 <addr>          listen ipv6 address, default: ::1\n"
           " -l, --listen-port <port>           listen port number, default: 60080\n"
           " -j, --thread-nums <num>            number of worker threads, default: 1\n"
           " -n, --nofile-limit <num>           set nofile limit, maybe need root priv\n"
           " -o, --udp-timeout <sec>            udp socket idle timeout, default: 300\n"
           " -c, --cache-size <size>            max size of udp lrucache, default: 256\n"
           " -f, --buffer-size <size>           buffer size of tcp socket, default: 8192\n"
           " -u, --run-user <user>              run the ipt2socks with the specified user\n"
           " -G, --graceful                     gracefully close the tcp connection pair\n"
           " -R, --redirect                     use redirect instead of tproxy (for tcp)\n"
           " -T, --tcp-only                     listen tcp only, aka: disable udp proxy\n"
           " -U, --udp-only                     listen udp only, aka: disable tcp proxy\n"
           " -4, --ipv4-only                    listen ipv4 only, aka: disable ipv6 proxy\n"
           " -6, --ipv6-only                    listen ipv6 only, aka: disable ipv4 proxy\n"
           " -v, --verbose                      print verbose log, default: <disabled>\n"
           " -V, --version                      print ipt2socks version number and exit\n"
           " -h, --help                         print ipt2socks help information and exit\n"
    );
}

/* parsing command line arguments */
static void parse_command_args(int argc, char* argv[]) {
    const char *optstr = ":s:p:a:k:b:B:l:j:n:o:c:f:u:GRTU46vVh";
    const struct option options[] = {
        {"server-addr",   required_argument, NULL, 's'},
        {"server-port",   required_argument, NULL, 'p'},
        {"auth-username", required_argument, NULL, 'a'},
        {"auth-password", required_argument, NULL, 'k'},
        {"listen-addr4",  required_argument, NULL, 'b'},
        {"listen-addr6",  required_argument, NULL, 'B'},
        {"listen-port",   required_argument, NULL, 'l'},
        {"thread-nums",   required_argument, NULL, 'j'},
        {"nofile-limit",  required_argument, NULL, 'n'},
        {"udp-timeout",   required_argument, NULL, 'o'},
        {"cache-size",    required_argument, NULL, 'c'},
        {"buffer-size",   required_argument, NULL, 'f'},
        {"run-user",      required_argument, NULL, 'u'},
        {"graceful",      no_argument,       NULL, 'G'},
        {"redirect",      no_argument,       NULL, 'R'},
        {"tcp-only",      no_argument,       NULL, 'T'},
        {"udp-only",      no_argument,       NULL, 'U'},
        {"ipv4-only",     no_argument,       NULL, '4'},
        {"ipv6-only",     no_argument,       NULL, '6'},
        {"verbose",       no_argument,       NULL, 'v'},
        {"version",       no_argument,       NULL, 'V'},
        {"help",          no_argument,       NULL, 'h'},
        {NULL,            0,                 NULL,   0},
    };

    const char *opt_auth_username = NULL;
    const char *opt_auth_password = NULL;

    opterr = 0;
    int optindex = -1;
    int shortopt = -1;
    while ((shortopt = getopt_long(argc, argv, optstr, options, &optindex)) != -1) {
        switch (shortopt) {
            case 's':
                if (strlen(optarg) + 1 > IP6STRLEN) {
                    printf("[parse_command_args] ip address max length is 45: %s\n", optarg);
                    goto PRINT_HELP_AND_EXIT;
                }
                if (get_ipstr_family(optarg) == -1) {
                    printf("[parse_command_args] invalid server ip address: %s\n", optarg);
                    goto PRINT_HELP_AND_EXIT;
                }
                g_server_isipv4 = get_ipstr_family(optarg) == AF_INET;
                strcpy(g_server_ipstr, optarg);
                break;
            case 'p':
                if (strlen(optarg) + 1 > PORTSTRLEN) {
                    printf("[parse_command_args] port number max length is 5: %s\n", optarg);
                    goto PRINT_HELP_AND_EXIT;
                }
                g_server_portno = strtol(optarg, NULL, 10);
                if (g_server_portno == 0) {
                    printf("[parse_command_args] invalid server port number: %s\n", optarg);
                    goto PRINT_HELP_AND_EXIT;
                }
                break;
            case 'a':
                if (strlen(optarg) > SOCKS5_USRPWD_USRMAXLEN) {
                    printf("[parse_command_args] socks5 username max length is 255: %s\n", optarg);
                    goto PRINT_HELP_AND_EXIT;
                }
                opt_auth_username = optarg;
                break;
            case 'k':
                if (strlen(optarg) > SOCKS5_USRPWD_PWDMAXLEN) {
                    printf("[parse_command_args] socks5 password max length is 255: %s\n", optarg);
                    goto PRINT_HELP_AND_EXIT;
                }
                opt_auth_password = optarg;
                break;
            case 'b':
                if (strlen(optarg) + 1 > IP4STRLEN) {
                    printf("[parse_command_args] ipv4 address max length is 15: %s\n", optarg);
                    goto PRINT_HELP_AND_EXIT;
                }
                if (get_ipstr_family(optarg) != AF_INET) {
                    printf("[parse_command_args] invalid listen ipv4 address: %s\n", optarg);
                    goto PRINT_HELP_AND_EXIT;
                }
                strcpy(g_bind_ipstr4, optarg);
                break;
            case 'B':
                if (strlen(optarg) + 1 > IP6STRLEN) {
                    printf("[parse_command_args] ipv6 address max length is 45: %s\n", optarg);
                    goto PRINT_HELP_AND_EXIT;
                }
                if (get_ipstr_family(optarg) != AF_INET6) {
                    printf("[parse_command_args] invalid listen ipv6 address: %s\n", optarg);
                    goto PRINT_HELP_AND_EXIT;
                }
                strcpy(g_bind_ipstr6, optarg);
                break;
            case 'l':
                if (strlen(optarg) + 1 > PORTSTRLEN) {
                    printf("[parse_command_args] port number max length is 5: %s\n", optarg);
                    goto PRINT_HELP_AND_EXIT;
                }
                g_bind_portno = strtol(optarg, NULL, 10);
                if (g_bind_portno == 0) {
                    printf("[parse_command_args] invalid listen port number: %s\n", optarg);
                    goto PRINT_HELP_AND_EXIT;
                }
                break;
            case 'j':
                g_nthreads = strtol(optarg, NULL, 10);
                if (g_nthreads == 0) {
                    printf("[parse_command_args] invalid number of worker threads: %s\n", optarg);
                    goto PRINT_HELP_AND_EXIT;
                }
                break;
            case 'n':
                set_nofile_limit(strtol(optarg, NULL, 10));
                break;
            case 'o':
                g_udpidletmo = strtol(optarg, NULL, 10);
                if (g_udpidletmo == 0) {
                    printf("[parse_command_args] invalid udp socket idle timeout: %s\n", optarg);
                    goto PRINT_HELP_AND_EXIT;
                }
                break;
            case 'c':
                if (strtol(optarg, NULL, 10) == 0) {
                    printf("[parse_command_args] invalid maxsize of udp lrucache: %s\n", optarg);
                    goto PRINT_HELP_AND_EXIT;
                }
                lrucache_set_maxsize(strtol(optarg, NULL, 10));
                break;
            case 'f':
                g_tcpbufsiz = strtol(optarg, NULL, 10);
                if (g_tcpbufsiz < TCP_SKBUFSIZE_MINIMUM) {
                    printf("[parse_command_args] buffer should have at least 1024B: %s\n", optarg);
                    goto PRINT_HELP_AND_EXIT;
                }
                break;
            case 'u':
                run_as_user(optarg, argv);
                break;
            case 'G':
                g_options |= OPTION_HFCLS;
                break;
            case 'R':
                g_options |= OPTION_DNAT;
                strcpy(g_bind_ipstr4, IP4STR_WILDCARD);
                strcpy(g_bind_ipstr6, IP6STR_WILDCARD);
                break;
            case 'T':
                g_options &= ~OPTION_UDP;
                break;
            case 'U':
                g_options &= ~OPTION_TCP;
                break;
            case '4':
                g_options &= ~OPTION_IPV6;
                break;
            case '6':
                g_options &= ~OPTION_IPV4;
                break;
            case 'v':
                g_verbose = true;
                break;
            case 'V':
                printf(IPT2SOCKS_VERSION"\n");
                exit(0);
            case 'h':
                print_command_help();
                exit(0);
            case ':':
                printf("[parse_command_args] missing optarg: '%s'\n", argv[optind - 1]);
                goto PRINT_HELP_AND_EXIT;
            case '?':
                if (optopt) {
                    printf("[parse_command_args] unknown option: '-%c'\n", optopt);
                } else {
                    char *longopt = argv[optind - 1];
                    char *equalsign = strchr(longopt, '=');
                    if (equalsign) *equalsign = 0;
                    printf("[parse_command_args] unknown option: '%s'\n", longopt);
                }
                goto PRINT_HELP_AND_EXIT;
        }
    }

    if (strlen(g_server_ipstr) == 0) {
        printf("[parse_command_args] missing option: '-s/--server-addr'\n");
        goto PRINT_HELP_AND_EXIT;
    }
    if (g_server_portno == 0) {
        printf("[parse_command_args] missing option: '-p/--server-port'\n");
        goto PRINT_HELP_AND_EXIT;
    }

    if (!(g_options & (OPTION_TCP | OPTION_UDP))) {
        printf("[parse_command_args] both tcp and udp are disabled, nothing to do\n");
        goto PRINT_HELP_AND_EXIT;
    }
    if (!(g_options & (OPTION_IPV4 | OPTION_IPV6))) {
        printf("[parse_command_args] both ipv4 and ipv6 are disabled, nothing to do\n");
        goto PRINT_HELP_AND_EXIT;
    }

    if (opt_auth_username && !opt_auth_password) {
        printf("[parse_command_args] username specified, but password is not provided\n");
        goto PRINT_HELP_AND_EXIT;
    }
    if (!opt_auth_username && opt_auth_password) {
        printf("[parse_command_args] password specified, but username is not provided\n");
        goto PRINT_HELP_AND_EXIT;
    }
    if (opt_auth_username && opt_auth_password) {
        /* change auth method to usrpwd */
        g_socks5_auth_request.method = SOCKS5_METHOD_USRPWD;

        /* socks5-usrpwd-request version */
        socks5_usrpwdreq_t *usrpwdreq = (void *)g_usrpwd_reqbuf;
        usrpwdreq->version = SOCKS5_USRPWD_VERSION;

        /* socks5-usrpwd-request usernamelen */
        uint8_t *usrlenptr = (void *)usrpwdreq + 1;
        *usrlenptr = strlen(opt_auth_username);

        /* socks5-usrpwd-request usernamestr */
        char *usrbufptr = (void *)usrlenptr + 1;
        memcpy(usrbufptr, opt_auth_username, *usrlenptr);

        /* socks5-usrpwd-request passwordlen */
        uint8_t *pwdlenptr = (void *)usrbufptr + *usrlenptr;
        *pwdlenptr = strlen(opt_auth_password);

        /* socks5-usrpwd-request passwordstr */
        char *pwdbufptr = (void *)pwdlenptr + 1;
        memcpy(pwdbufptr, opt_auth_password, *pwdlenptr);

        /* socks5-usrpwd-request total_length */
        g_usrpwd_reqlen = 1 + 1 + *usrlenptr + 1 + *pwdlenptr;
    }

    if (!(g_options & OPTION_TCP)) g_nthreads = 1;

    build_ipv4_addr(&g_bind_skaddr4, g_bind_ipstr4, g_bind_portno);
    build_ipv6_addr(&g_bind_skaddr6, g_bind_ipstr6, g_bind_portno);

    if (g_server_isipv4) {
        build_ipv4_addr((void *)&g_server_skaddr, g_server_ipstr, g_server_portno);
    } else {
        build_ipv6_addr((void *)&g_server_skaddr, g_server_ipstr, g_server_portno);
    }
    return;

PRINT_HELP_AND_EXIT:
    print_command_help();
    exit(1);
}

/* main entry */
int main(int argc, char* argv[]) {
    signal(SIGPIPE, SIG_IGN);
    setvbuf(stdout, NULL, _IOLBF, 256);
    parse_command_args(argc, argv);

    LOGINF("[main] server address: %s#%hu", g_server_ipstr, g_server_portno);
    if (g_options & OPTION_IPV4) LOGINF("[main] listen address: %s#%hu", g_bind_ipstr4, g_bind_portno);
    if (g_options & OPTION_IPV6) LOGINF("[main] listen address: %s#%hu", g_bind_ipstr6, g_bind_portno);
    LOGINF("[main] number of worker threads: %hhu", g_nthreads);
    LOGINF("[main] udp socket idle timeout: %hu", g_udpidletmo);
    LOGINF("[main] udp cache maximum size: %hu", lrucache_get_maxsize());
    LOGINF("[main] tcp socket buffer size: %u", g_tcpbufsiz);
    if (g_options & OPTION_TCP) LOGINF("[main] enable tcp transparent proxy");
    if (g_options & OPTION_UDP) LOGINF("[main] enable udp transparent proxy");
    if (g_options & OPTION_DNAT) LOGINF("[main] use redirect instead of tproxy");
    if (g_options & OPTION_HFCLS) LOGINF("[main] gracefully close tcp connection");
    IF_VERBOSE LOGINF("[main] verbose mode (affect performance)");

    for (int i = 0; i < g_nthreads - 1; ++i) {
        if (pthread_create(&(pthread_t){0}, NULL, run_event_loop, NULL)) {
            LOGERR("[main] failed to create thread: (%d) %s", errno, errstring(errno));
            return errno;
        }
    }
    run_event_loop((void *)1); /* blocking here */

    return 0;
}

/* event loop */
static void* run_event_loop(void *is_main_thread) {
    uv_loop_t *evloop = &(uv_loop_t){0};
    uv_loop_init(evloop);

    if (g_options & OPTION_TCP) {
        if (g_options & OPTION_IPV4) {
            uv_tcp_t *tcplistener = malloc(sizeof(uv_tcp_t));
            tcplistener->data = (void *)1; /* is_ipv4 */

            uv_tcp_init(evloop, tcplistener);
            uv_tcp_open(tcplistener, (g_options & OPTION_DNAT) ? new_tcp4_bindsock() : new_tcp4_bindsock_tproxy());

            int retval = uv_tcp_bind(tcplistener, (void *)&g_bind_skaddr4, 0);
            if (retval < 0) {
                LOGERR("[run_event_loop] failed to bind address for tcp4 socket: (%d) %s", -retval, uv_strerror(retval));
                exit(-retval);
            }

            retval = uv_listen((void *)tcplistener, SOMAXCONN, tcp_socket_listen_cb);
            if (retval < 0) {
                LOGERR("[run_event_loop] failed to listen address for tcp4 socket: (%d) %s", -retval, uv_strerror(retval));
                exit(-retval);
            }
        }
        if (g_options & OPTION_IPV6) {
            uv_tcp_t *tcplistener = malloc(sizeof(uv_tcp_t));
            tcplistener->data = NULL; /* is_ipv4 */

            uv_tcp_init(evloop, tcplistener);
            uv_tcp_open(tcplistener, (g_options & OPTION_DNAT) ? new_tcp6_bindsock() : new_tcp6_bindsock_tproxy());

            int retval = uv_tcp_bind(tcplistener, (void *)&g_bind_skaddr6, 0);
            if (retval < 0) {
                LOGERR("[run_event_loop] failed to bind address for tcp6 socket: (%d) %s", -retval, uv_strerror(retval));
                exit(-retval);
            }

            retval = uv_listen((void *)tcplistener, SOMAXCONN, tcp_socket_listen_cb);
            if (retval < 0) {
                LOGERR("[run_event_loop] failed to listen address for tcp6 socket: (%d) %s", -retval, uv_strerror(retval));
                exit(-retval);
            }
        }
    }

    if ((g_options & OPTION_UDP) && is_main_thread) {
        if (g_options & OPTION_IPV4) {
            int sockfd = new_udp4_bindsock_tproxy();
            if (bind(sockfd, (void *)&g_bind_skaddr4, sizeof(skaddr4_t)) < 0) {
                LOGERR("[run_event_loop] failed to bind address for udp4 socket: (%d) %s", errno, errstring(errno));
                exit(errno);
            }
            uv_poll_t *udplistener = malloc(sizeof(uv_poll_t));
            udplistener->data = (void *)1; /* is_ipv4 */
            uv_poll_init(evloop, udplistener, sockfd);
            uv_poll_start(udplistener, UV_READABLE, udp_socket_listen_cb);
        }
        if (g_options & OPTION_IPV6) {
            int sockfd = new_udp6_bindsock_tproxy();
            if (bind(sockfd, (void *)&g_bind_skaddr6, sizeof(skaddr6_t)) < 0) {
                LOGERR("[run_event_loop] failed to bind address for udp6 socket: (%d) %s", errno, errstring(errno));
                exit(errno);
            }
            uv_poll_t *udplistener = malloc(sizeof(uv_poll_t));
            udplistener->data = NULL; /* is_ipv4 */
            uv_poll_init(evloop, udplistener, sockfd);
            uv_poll_start(udplistener, UV_READABLE, udp_socket_listen_cb);
        }
    }

    /* run event loop (blocking here) */
    uv_run(evloop, UV_RUN_DEFAULT);
    return NULL;
}

/* handling new tcp client connections */
static void tcp_socket_listen_cb(uv_stream_t *listener, int status) {
    bool isipv4 = listener->data != NULL;

    if (status < 0) {
        LOGERR("[tcp_socket_listen_cb] failed to accept tcp%c socket: (%d) %s", isipv4 ? '4' : '6', -status, uv_strerror(status));
        return;
    }

    uv_tcp_t *client_stream = malloc(sizeof(uv_tcp_t));
    uv_tcp_init(listener->loop, client_stream);
    uv_tcp_nodelay(client_stream, 1);
    client_stream->data = NULL;

    status = uv_accept(listener, (void *)client_stream);
    if (status < 0) {
        LOGERR("[tcp_socket_listen_cb] failed to accept tcp%c socket: (%d) %s", isipv4 ? '4' : '6', -status, uv_strerror(status));
        uv_close((void *)client_stream, tcp_stream_close_cb);
        return;
    }

    int sockfd = -1;
    uv_fileno((void *)client_stream, &sockfd);
    if (g_options & OPTION_HFCLS) set_keepalive(sockfd);
    skaddr6_t skaddr; char ipstr[IP6STRLEN]; portno_t portno;

    IF_VERBOSE {
        getpeername(sockfd, (void *)&skaddr, &(socklen_t){sizeof(skaddr)});
        if (isipv4) {
            parse_ipv4_addr((void *)&skaddr, ipstr, &portno);
        } else {
            parse_ipv6_addr((void *)&skaddr, ipstr, &portno);
        }
        LOGINF("[tcp_socket_listen_cb] accept new tcp connection: %s#%hu", ipstr, portno);
    }

    if (g_options & OPTION_DNAT) {
        if (!(isipv4 ? get_tcp_origdstaddr4(sockfd, (void *)&skaddr) : get_tcp_origdstaddr6(sockfd, (void *)&skaddr))) {
            uv_close((void *)client_stream, tcp_stream_close_cb);
            return;
        }
    } else {
        getsockname(sockfd, (void *)&skaddr, &(socklen_t){sizeof(skaddr)});
    }

    IF_VERBOSE {
        if (isipv4) {
            parse_ipv4_addr((void *)&skaddr, ipstr, &portno);
        } else {
            parse_ipv6_addr((void *)&skaddr, ipstr, &portno);
        }
        LOGINF("[tcp_socket_listen_cb] original destination addr: %s#%hu", ipstr, portno);
    }

    uv_tcp_t *socks5_stream = malloc(sizeof(uv_tcp_t));
    uv_tcp_init(listener->loop, socks5_stream);
    uv_tcp_nodelay(socks5_stream, 1);
    socks5_stream->data = NULL;

    IF_VERBOSE LOGINF("[tcp_socket_listen_cb] try to connect to socks5 server: %s#%hu", g_server_ipstr, g_server_portno);
    uv_connect_t *connreq = malloc(sizeof(uv_connect_t));
    status = uv_tcp_connect(connreq, socks5_stream, (void *)&g_server_skaddr, tcp_socks5_tcp_connect_cb);
    if (status < 0) {
        LOGERR("[tcp_socket_listen_cb] failed to connect to socks5 server: (%d) %s", -status, uv_strerror(status));
        uv_close((void *)client_stream, tcp_stream_close_cb);
        uv_close((void *)socks5_stream, tcp_stream_close_cb);
        free(connreq);
        return;
    }

    uv_fileno((void *)socks5_stream, &sockfd);
    if (g_options & OPTION_HFCLS) set_keepalive(sockfd);

    tcpcontext_t *context = malloc(sizeof(tcpcontext_t));
    context->client_stream = client_stream;
    context->socks5_stream = socks5_stream;
    context->client_buffer = malloc(g_tcpbufsiz);
    context->socks5_buffer = malloc(g_tcpbufsiz);
    context->client_wrtreq = malloc(sizeof(uv_write_t));
    context->socks5_wrtreq = malloc(sizeof(uv_write_t));
    context->is_half_close = g_options & OPTION_HFCLS ? false : true;
    client_stream->data = context;
    socks5_stream->data = context;

    if (isipv4) {
        socks5_ipv4req_t *proxyreq = context->client_buffer;
        proxyreq->version = SOCKS5_VERSION;
        proxyreq->command = SOCKS5_COMMAND_CONNECT;
        proxyreq->reserved = 0;
        proxyreq->addrtype = SOCKS5_ADDRTYPE_IPV4;
        proxyreq->ipaddr4 = ((skaddr4_t *)&skaddr)->sin_addr.s_addr;
        proxyreq->portnum = ((skaddr4_t *)&skaddr)->sin_port;
    } else {
        socks5_ipv6req_t *proxyreq = context->client_buffer;
        proxyreq->version = SOCKS5_VERSION;
        proxyreq->command = SOCKS5_COMMAND_CONNECT;
        proxyreq->reserved = 0;
        proxyreq->addrtype = SOCKS5_ADDRTYPE_IPV6;
        memcpy(&proxyreq->ipaddr6, &skaddr.sin6_addr.s6_addr, IP6BINLEN);
        proxyreq->portnum = skaddr.sin6_port;
    }
}

/* successfully connected to the socks5 server */
static void tcp_socks5_tcp_connect_cb(uv_connect_t *connreq, int status) {
    uv_stream_t *socks5_stream = connreq->handle;
    tcpcontext_t *context = socks5_stream->data;
    uv_stream_t *client_stream = (void *)context->client_stream;
    free(connreq);

    if (status < 0) {
        LOGERR("[tcp_socks5_tcp_connect_cb] failed to connect to socks5 server: (%d) %s", -status, uv_strerror(status));
        goto CLOSE_STREAM_PAIR;
    }
    IF_VERBOSE LOGINF("[tcp_socks5_tcp_connect_cb] connected to socks5 server: %s#%hu", g_server_ipstr, g_server_portno);

    IF_VERBOSE LOGINF("[tcp_socks5_tcp_connect_cb] send authreq to socks5 server: %s#%hu", g_server_ipstr, g_server_portno);
    uv_buf_t uvbufs[] = {{.base = (void *)&g_socks5_auth_request, .len = sizeof(socks5_authreq_t)}};
    status = uv_try_write(socks5_stream, uvbufs, 1);
    if (status < 0) {
        LOGERR("[tcp_socks5_tcp_connect_cb] failed to send authreq to socks5 server: (%d) %s", -status, uv_strerror(status));
        goto CLOSE_STREAM_PAIR;
    } else if (status < (int)sizeof(socks5_authreq_t)) {
        LOGERR("[tcp_socks5_tcp_connect_cb] socks5 authreq was not completely sent: %d < %zu", status, sizeof(socks5_authreq_t));
        goto CLOSE_STREAM_PAIR;
    }
    uv_read_start(socks5_stream, tcp_common_alloc_cb, tcp_socks5_auth_read_cb);
    return;

CLOSE_STREAM_PAIR:
    uv_close((void *)socks5_stream, tcp_stream_close_cb);
    uv_close((void *)client_stream, tcp_stream_close_cb);
}

/* populate the uvbuf structure before the read_cb call */
static void tcp_common_alloc_cb(uv_handle_t *stream, size_t sugsize, uv_buf_t *uvbuf) {
    (void) sugsize;
    tcpcontext_t *context = stream->data;
    bool is_socks5_stream = (void *)stream == (void *)context->socks5_stream;
    uvbuf->base = is_socks5_stream ? context->socks5_buffer : context->client_buffer;
    uvbuf->len = g_tcpbufsiz;
}

/* receive authentication response from the socks5 server */
static void tcp_socks5_auth_read_cb(uv_stream_t *socks5_stream, ssize_t nread, const uv_buf_t *uvbuf) {
    if (nread == 0) return;
    uv_read_stop(socks5_stream);
    tcpcontext_t *context = socks5_stream->data;
    uv_stream_t *client_stream = (void *)context->client_stream;
    bool is_usrpwd_auth = g_socks5_auth_request.method == SOCKS5_METHOD_USRPWD;

    if (nread < 0) {
        LOGERR("[tcp_socks5_auth_read_cb] failed to read data from socks5 server: (%zd) %s", -nread, uv_strerror(nread));
        goto CLOSE_STREAM_PAIR;
    }

    if (nread != sizeof(socks5_authresp_t)) {
        LOGERR("[tcp_socks5_auth_read_cb] auth response length is incorrect: %zd != %zu", nread, sizeof(socks5_authresp_t));
        goto CLOSE_STREAM_PAIR;
    }

    socks5_authresp_t *authresp = (void *)uvbuf->base;
    if (authresp->version != SOCKS5_VERSION) {
        LOGERR("[tcp_socks5_auth_read_cb] auth response version is not SOCKS5: %#hhx", authresp->version);
        goto CLOSE_STREAM_PAIR;
    }
    if (authresp->method != g_socks5_auth_request.method) {
        LOGERR("[tcp_socks5_auth_read_cb] auth response method is not %s: %#hhx", is_usrpwd_auth ? "USRPWD" : "NOAUTH", authresp->method);
        goto CLOSE_STREAM_PAIR;
    }

    IF_VERBOSE LOGINF("[tcp_socks5_auth_read_cb] send %sreq to socks5 server: %s#%hu", is_usrpwd_auth ? "usrpwd" : "proxy", g_server_ipstr, g_server_portno);
    uv_buf_t uvbufs[] = {{.base = NULL, .len = 0}};
    if (is_usrpwd_auth) {
        uvbufs[0].base = (void *)g_usrpwd_reqbuf;
        uvbufs[0].len = g_usrpwd_reqlen;
    } else {
        socks5_ipv4req_t *proxyreq = context->client_buffer;
        uvbufs[0].base = (void *)proxyreq;
        uvbufs[0].len = proxyreq->addrtype == SOCKS5_ADDRTYPE_IPV4 ? sizeof(socks5_ipv4req_t) : sizeof(socks5_ipv6req_t);
    }
    nread = uv_try_write(socks5_stream, uvbufs, 1);
    if (nread < 0) {
        LOGERR("[tcp_socks5_auth_read_cb] failed to send %sreq to socks5 server: (%zd) %s", is_usrpwd_auth ? "usrpwd" : "proxy", -nread, uv_strerror(nread));
        goto CLOSE_STREAM_PAIR;
    } else if ((size_t)nread < uvbufs[0].len) {
        LOGERR("[tcp_socks5_auth_read_cb] socks5 %sreq was not completely sent: %zd < %zu", is_usrpwd_auth ? "usrpwd" : "proxy", nread, uvbufs[0].len);
        goto CLOSE_STREAM_PAIR;
    }
    uv_read_start(socks5_stream, tcp_common_alloc_cb, is_usrpwd_auth ? tcp_socks5_usrpwd_read_cb : tcp_socks5_resp_read_cb);
    return;

CLOSE_STREAM_PAIR:
    uv_close((void *)socks5_stream, tcp_stream_close_cb);
    uv_close((void *)client_stream, tcp_stream_close_cb);
}

/* receive authentication result of username-password */
static void tcp_socks5_usrpwd_read_cb(uv_stream_t *socks5_stream, ssize_t nread, const uv_buf_t *uvbuf) {
    if (nread == 0) return;
    uv_read_stop(socks5_stream);
    tcpcontext_t *context = socks5_stream->data;
    uv_stream_t *client_stream = (void *)context->client_stream;

    if (nread < 0) {
        LOGERR("[tcp_socks5_usrpwd_read_cb] failed to read data from socks5 server: (%zd) %s", -nread, uv_strerror(nread));
        goto CLOSE_STREAM_PAIR;
    }

    if (nread != sizeof(socks5_usrpwdresp_t)) {
        LOGERR("[tcp_socks5_usrpwd_read_cb] usrpwd response length is incorrect: %zd != %zu", nread, sizeof(socks5_usrpwdresp_t));
        goto CLOSE_STREAM_PAIR;
    }

    socks5_usrpwdresp_t *usrpwdresp = (void *)uvbuf->base;
    if (usrpwdresp->version != SOCKS5_USRPWD_VERSION) {
        LOGERR("[tcp_socks5_usrpwd_read_cb] usrpwd response version is not VER01: %#hhx", usrpwdresp->version);
        goto CLOSE_STREAM_PAIR;
    }
    if (usrpwdresp->respcode != SOCKS5_USRPWD_AUTHSUCC) {
        LOGERR("[tcp_socks5_usrpwd_read_cb] usrpwd response respcode is not SUCC: %#hhx", usrpwdresp->respcode);
        goto CLOSE_STREAM_PAIR;
    }

    IF_VERBOSE LOGINF("[tcp_socks5_usrpwd_read_cb] send proxyreq to socks5 server: %s#%hu", g_server_ipstr, g_server_portno);
    socks5_ipv4req_t *proxyreq = context->client_buffer;
    bool isipv4 = proxyreq->addrtype == SOCKS5_ADDRTYPE_IPV4;
    int length = isipv4 ? sizeof(socks5_ipv4req_t) : sizeof(socks5_ipv6req_t);
    uv_buf_t uvbufs[] = {{.base = context->client_buffer, .len = length}};
    nread = uv_try_write(socks5_stream, uvbufs, 1);
    if (nread < 0) {
        LOGERR("[tcp_socks5_usrpwd_read_cb] failed to send proxyreq to socks5 server: (%zd) %s", -nread, uv_strerror(nread));
        goto CLOSE_STREAM_PAIR;
    } else if (nread < length) {
        LOGERR("[tcp_socks5_usrpwd_read_cb] socks5 proxyreq was not completely sent: %zd < %d", nread, length);
        goto CLOSE_STREAM_PAIR;
    }
    uv_read_start(socks5_stream, tcp_common_alloc_cb, tcp_socks5_resp_read_cb);
    return;

CLOSE_STREAM_PAIR:
    uv_close((void *)socks5_stream, tcp_stream_close_cb);
    uv_close((void *)client_stream, tcp_stream_close_cb);
}

/* receive socks5-proxy response from the socks5 server */
static void tcp_socks5_resp_read_cb(uv_stream_t *socks5_stream, ssize_t nread, const uv_buf_t *uvbuf) {
    if (nread == 0) return;
    uv_read_stop(socks5_stream);
    tcpcontext_t *context = socks5_stream->data;
    uv_stream_t *client_stream = (void *)context->client_stream;

    if (nread < 0) {
        LOGERR("[tcp_socks5_resp_read_cb] failed to read data from socks5 server: (%zd) %s", -nread, uv_strerror(nread));
        goto CLOSE_STREAM_PAIR;
    }

    if (nread != sizeof(socks5_ipv4resp_t) && nread != sizeof(socks5_ipv6resp_t)) {
        LOGERR("[tcp_socks5_resp_read_cb] proxy response length is incorrect: %zd != %zu/%zu", nread, sizeof(socks5_ipv4resp_t), sizeof(socks5_ipv6resp_t));
        goto CLOSE_STREAM_PAIR;
    }

    bool isipv4 = nread == sizeof(socks5_ipv4resp_t);
    socks5_ipv4resp_t *proxyresp = (void *)uvbuf->base;
    if (proxyresp->version != SOCKS5_VERSION) {
        LOGERR("[tcp_socks5_resp_read_cb] proxy response version is not SOCKS5: %#hhx", proxyresp->version);
        goto CLOSE_STREAM_PAIR;
    }
    if (proxyresp->respcode != SOCKS5_RESPCODE_SUCCEEDED) {
        LOGERR("[tcp_socks5_resp_read_cb] proxy response respcode is not SUCC: (%#hhx) %s", proxyresp->respcode, socks5_rcode2string(proxyresp->respcode));
        goto CLOSE_STREAM_PAIR;
    }
    if (proxyresp->reserved != 0) {
        LOGERR("[tcp_socks5_resp_read_cb] proxy response reserved is not zero: %#hhx", proxyresp->reserved);
        goto CLOSE_STREAM_PAIR;
    }
    if (proxyresp->addrtype != (isipv4 ? SOCKS5_ADDRTYPE_IPV4 : SOCKS5_ADDRTYPE_IPV6)) {
        LOGERR("[tcp_socks5_resp_read_cb] proxy response addrtype is not ipv%c: %#hhx", isipv4 ? '4' : '6', proxyresp->addrtype);
        goto CLOSE_STREAM_PAIR;
    }

    IF_VERBOSE LOGINF("[tcp_socks5_resp_read_cb] connected to target host, start forwarding");
    uv_read_start(socks5_stream, tcp_common_alloc_cb, tcp_stream_read_cb);
    uv_read_start(client_stream, tcp_common_alloc_cb, tcp_stream_read_cb);
    return;

CLOSE_STREAM_PAIR:
    uv_close((void *)socks5_stream, tcp_stream_close_cb);
    uv_close((void *)client_stream, tcp_stream_close_cb);
}

/* read data from one end and forward it to the other end */
static void tcp_stream_read_cb(uv_stream_t *selfstream, ssize_t nread, const uv_buf_t *uvbuf) {
    if (nread == 0) return;
    tcpcontext_t *context = selfstream->data;
    bool is_socks5_stream = (void *)selfstream == (void *)context->socks5_stream;
    uv_stream_t *peerstream = is_socks5_stream ? (void *)context->client_stream : (void *)context->socks5_stream;

    if (nread == UV_EOF) {
        if (context->is_half_close) {
            IF_VERBOSE LOGINF("[tcp_stream_read_cb] tcp connection has been closed in both directions");
            goto CLOSE_STREAM_PAIR;
        } else {
            int sockfd = -1;
            uv_fileno((void *)peerstream, &sockfd);
            if (shutdown(sockfd, SHUT_WR) < 0) {
                LOGERR("[tcp_stream_read_cb] failed to send EOF to peer stream: (%d) %s", errno, errstring(errno));
                goto CLOSE_STREAM_PAIR;
            }
            uv_read_stop(selfstream);
            context->is_half_close = true;
            return;
        }
    }

    if (nread < 0) {
        LOGERR("[tcp_stream_read_cb] failed to read data from tcp stream: (%zd) %s", -nread, uv_strerror(nread));
        goto CLOSE_STREAM_PAIR;
    }

    uv_buf_t uvbufs[] = {{.base = uvbuf->base, .len = nread}};
    nread = uv_try_write(peerstream, uvbufs, 1);
    if (nread < (ssize_t)uvbufs[0].len) {
        if (nread > 0) {
            uvbufs[0].base += nread;
            uvbufs[0].len -= (size_t)nread;
        }
        uv_write_t *writereq = is_socks5_stream ? context->socks5_wrtreq : context->client_wrtreq;
        nread = uv_write(writereq, peerstream, uvbufs, 1, tcp_stream_write_cb);
        if (nread < 0) {
            LOGERR("[tcp_stream_read_cb] failed to write data to peer stream: (%zd) %s", -nread, uv_strerror(nread));
            goto CLOSE_STREAM_PAIR;
        }
        uv_read_stop(selfstream);
    }
    return;

CLOSE_STREAM_PAIR:
    uv_close((void *)selfstream, tcp_stream_close_cb);
    uv_close((void *)peerstream, tcp_stream_close_cb);
}

/* tcp data stream is sent, restart read */
static void tcp_stream_write_cb(uv_write_t *writereq, int status) {
    if (status == UV_ECANCELED) return;

    uv_stream_t *selfstream = writereq->handle;
    tcpcontext_t *context = selfstream->data;
    bool is_socks5_stream = (void *)selfstream == (void *)context->socks5_stream;
    uv_stream_t *peerstream = is_socks5_stream ? (void *)context->client_stream : (void *)context->socks5_stream;

    if (status < 0) {
        LOGERR("[tcp_stream_write_cb] failed to write data to tcp stream: (%d) %s", -status, uv_strerror(status));
        uv_close((void *)selfstream, tcp_stream_close_cb);
        uv_close((void *)peerstream, tcp_stream_close_cb);
        return;
    }

    uv_read_start(peerstream, tcp_common_alloc_cb, tcp_stream_read_cb);
}

/* close tcp connection and release resources */
static void tcp_stream_close_cb(uv_handle_t *stream) {
    tcpcontext_t *context = stream->data;
    if (context) {
        context->client_stream->data = NULL;
        context->socks5_stream->data = NULL;
        free(context->client_buffer);
        free(context->socks5_buffer);
        free(context->client_wrtreq);
        free(context->socks5_wrtreq);
        free(context);
    }
    free(stream);
}

/* handling udp tproxy packets from listening socket */
static void udp_socket_listen_cb(uv_poll_t *listener, int status, int events) {
    (void) events;
    bool isipv4 = listener->data != NULL;
    size_t udpmsghdrlen = isipv4 ? sizeof(socks5_udp4msg_t) : sizeof(socks5_udp6msg_t);

    if (status < 0) {
        LOGERR("[udp_socket_listen_cb] failed to recv data from udp%c socket: (%d) %s", isipv4 ? '4' : '6', -status, uv_strerror(status));
        return;
    }

    skaddr6_t source_skaddr = {0};
    char *packetbuf = g_udp_packetbuf;
    struct iovec iov = {
        .iov_base = packetbuf + udpmsghdrlen,
        .iov_len = UDP_PACKET_MAXSIZE - udpmsghdrlen,
    };
    char cntl_buffer[UDP_MSGCTL_BUFSIZE] = {0};
    struct msghdr msg = {
        .msg_name = &source_skaddr,
        .msg_namelen = sizeof(source_skaddr),
        .msg_iov = &iov,
        .msg_iovlen = 1,
        .msg_control = cntl_buffer,
        .msg_controllen = UDP_MSGCTL_BUFSIZE,
    };

    int sockfd = -1;
    uv_fileno((void *)listener, &sockfd);

    ssize_t nread = recvmsg(sockfd, &msg, 0);
    if (nread < 0) {
        LOGERR("[udp_socket_listen_cb] failed to recv data from udp%c socket: (%d) %s", isipv4 ? '4' : '6', errno, errstring(errno));
        return;
    }

    IF_VERBOSE {
        portno_t portno = 0;
        if (isipv4) {
            parse_ipv4_addr((void *)&source_skaddr, g_udp_ipstrbuf, &portno);
        } else {
            parse_ipv6_addr((void *)&source_skaddr, g_udp_ipstrbuf, &portno);
        }
        LOGINF("[udp_socket_listen_cb] recv %zd bytes data from %s#%hu", nread, g_udp_ipstrbuf, portno);
    }

    skaddr6_t target_skaddr = {0};
    if (!(isipv4 ? get_udp_origdstaddr4(&msg, (void *)&target_skaddr): get_udp_origdstaddr6(&msg, (void *)&target_skaddr))) {
        LOGERR("[udp_socket_listen_cb] failed to get original ipv%c destination address", isipv4 ? '4' : '6');
        return;
    }

    socks5_udp4msg_t *udp4msghdr = (void *)packetbuf;
    udp4msghdr->reserved = 0;
    udp4msghdr->fragment = 0;
    udp4msghdr->addrtype = isipv4 ? SOCKS5_ADDRTYPE_IPV4 : SOCKS5_ADDRTYPE_IPV6;
    if (isipv4) {
        udp4msghdr->ipaddr4 = ((skaddr4_t *)&target_skaddr)->sin_addr.s_addr;
        udp4msghdr->portnum = ((skaddr4_t *)&target_skaddr)->sin_port;
    } else {
        socks5_udp6msg_t *udp6msghdr = (void *)packetbuf;
        memcpy(&udp6msghdr->ipaddr6, &target_skaddr.sin6_addr.s6_addr, IP6BINLEN);
        udp6msghdr->portnum = target_skaddr.sin6_port;
    }

    ip_port_t client_key = {{0}, 0};
    if (isipv4) {
        client_key.ip.ip4 = ((skaddr4_t *)&source_skaddr)->sin_addr.s_addr;
        client_key.port = ((skaddr4_t *)&source_skaddr)->sin_port;
    } else {
        memcpy(&client_key.ip.ip6, &source_skaddr.sin6_addr.s6_addr, IP6BINLEN);
        client_key.port = source_skaddr.sin6_port;
    }

    cltentry_t *client_entry = cltcache_get(&g_udp_cltcache, &client_key);
    if (!client_entry) {
        uv_tcp_t *tcp_handle = malloc(sizeof(uv_tcp_t));
        uv_tcp_init(listener->loop, tcp_handle);
        uv_tcp_nodelay(tcp_handle, 1);

        IF_VERBOSE LOGINF("[udp_socket_listen_cb] try to connect to socks5 server: %s#%hu", g_server_ipstr, g_server_portno);
        uv_connect_t *connreq = malloc(sizeof(uv_connect_t));
        status = uv_tcp_connect(connreq, tcp_handle, (void *)&g_server_skaddr, udp_socks5_tcp_connect_cb);
        if (status < 0) {
            LOGERR("[udp_socket_listen_cb] failed to connect to socks5 server: (%d) %s", -status, uv_strerror(status));
            uv_close((void *)tcp_handle, (void *)free);
            free(connreq);
            return;
        }

        client_entry = malloc(sizeof(cltentry_t));
        memcpy(&client_entry->clt_ipport, &client_key, sizeof(ip_port_t));
        client_entry->free_timer = NULL; /* NULL means that the udp tunnel has not yet been opened */

        client_entry->tcp_handle = tcp_handle;
        tcp_handle->data = client_entry;

        client_entry->udp_handle = malloc(2 + udpmsghdrlen + nread); /* udpmsg */
        *(uint16_t *)client_entry->udp_handle = udpmsghdrlen + nread; /* msglen */
        memcpy((void *)client_entry->udp_handle + 2, packetbuf, udpmsghdrlen + nread); /* payload */

        cltentry_t *deleted_entry = cltcache_put(&g_udp_cltcache, client_entry);
        if (deleted_entry) udp_cltentry_release(deleted_entry);
        return;
    }
    if (!client_entry->free_timer) {
        IF_VERBOSE LOGINF("[udp_socket_listen_cb] connection is in progress, udp packet is ignored");
        return;
    }
    uv_timer_start(client_entry->free_timer, udp_cltentry_timer_cb, g_udpidletmo * 1000, 0);

    uv_buf_t uvbufs[] = {{.base = packetbuf, .len = udpmsghdrlen + nread}};
    status = uv_udp_try_send(client_entry->udp_handle, uvbufs, 1, NULL);
    if (status < 0) {
        LOGERR("[udp_socket_listen_cb] failed to send data to socks5 server: (%d) %s", -status, uv_strerror(status));
        return;
    }

    IF_VERBOSE {
        portno_t portno = 0;
        if (isipv4) {
            parse_ipv4_addr((void *)&target_skaddr, g_udp_ipstrbuf, &portno);
        } else {
            parse_ipv6_addr((void *)&target_skaddr, g_udp_ipstrbuf, &portno);
        }
        LOGINF("[udp_socket_listen_cb] send %zd bytes data to %s#%hu via socks5", nread, g_udp_ipstrbuf, portno);
    }
}

/* successfully connected to the socks5 server */
static void udp_socks5_tcp_connect_cb(uv_connect_t *connreq, int status) {
    if (status == UV_ECANCELED) {
        free(connreq);
        return;
    }
    uv_stream_t *tcp_handle = connreq->handle;
    cltentry_t *client_entry = tcp_handle->data;
    free(connreq);

    if (status < 0) {
        LOGERR("[udp_socks5_tcp_connect_cb] failed to connect to socks5 server: (%d) %s", -status, uv_strerror(status));
        goto RELEASE_CLIENT_ENTRY;
    }
    IF_VERBOSE LOGINF("[udp_socks5_tcp_connect_cb] connected to socks5 server: %s#%hu", g_server_ipstr, g_server_portno);

    IF_VERBOSE LOGINF("[udp_socks5_tcp_connect_cb] send authreq to socks5 server: %s#%hu", g_server_ipstr, g_server_portno);
    uv_buf_t uvbufs[] = {{.base = (void *)&g_socks5_auth_request, .len = sizeof(socks5_authreq_t)}};
    status = uv_try_write(tcp_handle, uvbufs, 1);
    if (status < 0) {
        LOGERR("[udp_socks5_tcp_connect_cb] failed to send authreq to socks5 server: (%d) %s", -status, uv_strerror(status));
        goto RELEASE_CLIENT_ENTRY;
    } else if (status < (int)sizeof(socks5_authreq_t)) {
        LOGERR("[udp_socks5_tcp_connect_cb] socks5 authreq was not completely sent: %d < %zu", status, sizeof(socks5_authreq_t));
        goto RELEASE_CLIENT_ENTRY;
    }
    uv_read_start(tcp_handle, udp_socks5_tcp_alloc_cb, udp_socks5_auth_read_cb);
    return;

RELEASE_CLIENT_ENTRY:
    cltcache_del(&g_udp_cltcache, client_entry);
    udp_cltentry_release(client_entry);
}

/* populate the uvbuf structure before the read_cb call */
static void udp_socks5_tcp_alloc_cb(uv_handle_t *stream, size_t sugsize, uv_buf_t *uvbuf) {
    (void) stream; (void) sugsize;
    uvbuf->base = g_udp_socks5buf;
    uvbuf->len = SOCKS5_HDR_MAXSIZE;
}

/* receive authentication response from the socks5 server */
static void udp_socks5_auth_read_cb(uv_stream_t *tcp_handle, ssize_t nread, const uv_buf_t *uvbuf) {
    if (nread == 0) return;
    uv_read_stop(tcp_handle);
    cltentry_t *client_entry = tcp_handle->data;
    bool is_usrpwd_auth = g_socks5_auth_request.method == SOCKS5_METHOD_USRPWD;

    if (nread < 0) {
        LOGERR("[udp_socks5_auth_read_cb] failed to read data from socks5 server: (%zd) %s", -nread, uv_strerror(nread));
        goto RELEASE_CLIENT_ENTRY;
    }

    if (nread != sizeof(socks5_authresp_t)) {
        LOGERR("[udp_socks5_auth_read_cb] auth response length is incorrect: %zd != %zu", nread, sizeof(socks5_authresp_t));
        goto RELEASE_CLIENT_ENTRY;
    }

    socks5_authresp_t *authresp = (void *)uvbuf->base;
    if (authresp->version != SOCKS5_VERSION) {
        LOGERR("[udp_socks5_auth_read_cb] auth response version is not SOCKS5: %#hhx", authresp->version);
        goto RELEASE_CLIENT_ENTRY;
    }
    if (authresp->method != g_socks5_auth_request.method) {
        LOGERR("[udp_socks5_auth_read_cb] auth response method is not %s: %#hhx", is_usrpwd_auth ? "USRPWD" : "NOAUTH", authresp->method);
        goto RELEASE_CLIENT_ENTRY;
    }

    IF_VERBOSE LOGINF("[udp_socks5_auth_read_cb] send %sreq to socks5 server: %s#%hu", is_usrpwd_auth ? "usrpwd" : "proxy", g_server_ipstr, g_server_portno);
    uv_buf_t uvbufs[] = {{.base = NULL, .len = 0}};
    if (is_usrpwd_auth) {
        uvbufs[0].base = (void *)g_usrpwd_reqbuf;
        uvbufs[0].len = g_usrpwd_reqlen;
    } else {
        socks5_udp4msg_t *udpmsghdr = (void *)client_entry->udp_handle + 2;
        bool isipv4 = udpmsghdr->addrtype == SOCKS5_ADDRTYPE_IPV4;
        uvbufs[0].base = isipv4 ? (void *)&G_SOCKS5_UDP4_REQUEST : (void *)&G_SOCKS5_UDP6_REQUEST;
        uvbufs[0].len = isipv4 ? sizeof(socks5_ipv4req_t) : sizeof(socks5_ipv6req_t);
    }
    nread = uv_try_write(tcp_handle, uvbufs, 1);
    if (nread < 0) {
        LOGERR("[udp_socks5_auth_read_cb] failed to send %sreq to socks5 server: (%zd) %s", is_usrpwd_auth ? "usrpwd" : "proxy", -nread, uv_strerror(nread));
        goto RELEASE_CLIENT_ENTRY;
    } else if ((size_t)nread < uvbufs[0].len) {
        LOGERR("[udp_socks5_auth_read_cb] socks5 %sreq was not completely sent: %zd < %zu", is_usrpwd_auth ? "usrpwd" : "proxy", nread, uvbufs[0].len);
        goto RELEASE_CLIENT_ENTRY;
    }
    uv_read_start(tcp_handle, udp_socks5_tcp_alloc_cb, is_usrpwd_auth ? udp_socks5_usrpwd_read_cb : udp_socks5_resp_read_cb);
    return;

RELEASE_CLIENT_ENTRY:
    cltcache_del(&g_udp_cltcache, client_entry);
    udp_cltentry_release(client_entry);
}

/* receive authentication result of username-password */
static void udp_socks5_usrpwd_read_cb(uv_stream_t *tcp_handle, ssize_t nread, const uv_buf_t *uvbuf) {
    if (nread == 0) return;
    uv_read_stop(tcp_handle);
    cltentry_t *client_entry = tcp_handle->data;

    if (nread < 0) {
        LOGERR("[udp_socks5_usrpwd_read_cb] failed to read data from socks5 server: (%zd) %s", -nread, uv_strerror(nread));
        goto RELEASE_CLIENT_ENTRY;
    }

    if (nread != sizeof(socks5_usrpwdresp_t)) {
        LOGERR("[udp_socks5_usrpwd_read_cb] usrpwd response length is incorrect: %zd != %zu", nread, sizeof(socks5_usrpwdresp_t));
        goto RELEASE_CLIENT_ENTRY;
    }

    socks5_usrpwdresp_t *usrpwdresp = (void *)uvbuf->base;
    if (usrpwdresp->version != SOCKS5_USRPWD_VERSION) {
        LOGERR("[udp_socks5_usrpwd_read_cb] usrpwd response version is not VER01: %#hhx", usrpwdresp->version);
        goto RELEASE_CLIENT_ENTRY;
    }
    if (usrpwdresp->respcode != SOCKS5_USRPWD_AUTHSUCC) {
        LOGERR("[udp_socks5_usrpwd_read_cb] usrpwd response respcode is not SUCC: %#hhx", usrpwdresp->respcode);
        goto RELEASE_CLIENT_ENTRY;
    }

    IF_VERBOSE LOGINF("[udp_socks5_usrpwd_read_cb] send proxyreq to socks5 server: %s#%hu", g_server_ipstr, g_server_portno);
    socks5_udp4msg_t *udpmsghdr = (void *)client_entry->udp_handle + 2;
    bool isipv4 = udpmsghdr->addrtype == SOCKS5_ADDRTYPE_IPV4;
    void *buffer = isipv4 ? (void *)&G_SOCKS5_UDP4_REQUEST : (void *)&G_SOCKS5_UDP6_REQUEST;
    int length = isipv4 ? sizeof(socks5_ipv4req_t) : sizeof(socks5_ipv6req_t);
    uv_buf_t uvbufs[] = {{.base = buffer, .len = length}};
    nread = uv_try_write(tcp_handle, uvbufs, 1);
    if (nread < 0) {
        LOGERR("[udp_socks5_usrpwd_read_cb] failed to send proxyreq to socks5 server: (%zd) %s", -nread, uv_strerror(nread));
        goto RELEASE_CLIENT_ENTRY;
    } else if (nread < length) {
        LOGERR("[udp_socks5_usrpwd_read_cb] socks5 proxyreq was not completely sent: %zd < %d", nread, length);
        goto RELEASE_CLIENT_ENTRY;
    }
    uv_read_start(tcp_handle, udp_socks5_tcp_alloc_cb, udp_socks5_resp_read_cb);
    return;

RELEASE_CLIENT_ENTRY:
    cltcache_del(&g_udp_cltcache, client_entry);
    udp_cltentry_release(client_entry);
}

/* receive socks5-proxy response from the socks5 server */
static void udp_socks5_resp_read_cb(uv_stream_t *tcp_handle, ssize_t nread, const uv_buf_t *uvbuf) {
    if (nread == 0) return;
    uv_read_stop(tcp_handle);
    cltentry_t *client_entry = tcp_handle->data;

    if (nread < 0) {
        LOGERR("[udp_socks5_resp_read_cb] failed to read data from socks5 server: (%zd) %s", -nread, uv_strerror(nread));
        goto RELEASE_CLIENT_ENTRY;
    }

    if (nread != sizeof(socks5_ipv4resp_t) && nread != sizeof(socks5_ipv6resp_t)) {
        LOGERR("[udp_socks5_resp_read_cb] proxy response length is incorrect: %zd != %zu/%zu", nread, sizeof(socks5_ipv4resp_t), sizeof(socks5_ipv6resp_t));
        goto RELEASE_CLIENT_ENTRY;
    }

    bool isipv4 = nread == sizeof(socks5_ipv4resp_t);
    socks5_ipv4resp_t *proxyresp = (void *)uvbuf->base;
    if (proxyresp->version != SOCKS5_VERSION) {
        LOGERR("[udp_socks5_resp_read_cb] proxy response version is not SOCKS5: %#hhx", proxyresp->version);
        goto RELEASE_CLIENT_ENTRY;
    }
    if (proxyresp->respcode != SOCKS5_RESPCODE_SUCCEEDED) {
        LOGERR("[udp_socks5_resp_read_cb] proxy response respcode is not SUCC: (%#hhx) %s", proxyresp->respcode, socks5_rcode2string(proxyresp->respcode));
        goto RELEASE_CLIENT_ENTRY;
    }
    if (proxyresp->reserved != 0) {
        LOGERR("[udp_socks5_resp_read_cb] proxy response reserved is not zero: %#hhx", proxyresp->reserved);
        goto RELEASE_CLIENT_ENTRY;
    }
    if (proxyresp->addrtype != (isipv4 ? SOCKS5_ADDRTYPE_IPV4 : SOCKS5_ADDRTYPE_IPV6)) {
        LOGERR("[udp_socks5_resp_read_cb] proxy response addrtype is not ipv%c: %#hhx", isipv4 ? '4' : '6', proxyresp->addrtype);
        goto RELEASE_CLIENT_ENTRY;
    }

    skaddr6_t server_skaddr = {0};
    if (isipv4) {
        skaddr4_t *skaddr = (void *)&server_skaddr;
        skaddr->sin_family = AF_INET;
        skaddr->sin_addr.s_addr = proxyresp->ipaddr4;
        skaddr->sin_port = proxyresp->portnum;
    } else {
        socks5_ipv6resp_t *proxy6resp = (void *)uvbuf->base;
        server_skaddr.sin6_family = AF_INET6;
        memcpy(&server_skaddr.sin6_addr.s6_addr, &proxy6resp->ipaddr6, IP6BINLEN);
        server_skaddr.sin6_port = proxy6resp->portnum;
    }

    uv_udp_t *udp_handle = malloc(sizeof(uv_udp_t));
    uv_udp_init(tcp_handle->loop, udp_handle);
    udp_handle->data = client_entry;

    nread = uv_udp_connect(udp_handle, (void *)&server_skaddr);
    if (nread < 0) {
        LOGERR("[udp_socks5_resp_read_cb] failed to create udp%c socket: (%zd) %s", isipv4 ? '4' : '6', -nread, uv_strerror(nread));
        uv_close((void *)udp_handle, (void *)free);
        goto RELEASE_CLIENT_ENTRY;
    }

    void *udpmsgbuf = client_entry->udp_handle;
    client_entry->udp_handle = udp_handle;

    client_entry->free_timer = malloc(sizeof(uv_timer_t));
    uv_timer_t *free_timer = client_entry->free_timer;
    uv_timer_init(tcp_handle->loop, free_timer);
    free_timer->data = client_entry;

    cltcache_use(&g_udp_cltcache, client_entry);
    uv_read_start(tcp_handle, udp_socks5_tcp_alloc_cb, udp_socks5_tcp_read_cb);
    uv_udp_recv_start(udp_handle, udp_client_alloc_cb, udp_client_recv_cb);
    uv_timer_start(free_timer, udp_cltentry_timer_cb, g_udpidletmo * 1000, 0);

    IF_VERBOSE LOGINF("[udp_socks5_resp_read_cb] udp tunnel is open, try to send packet via socks5");
    uv_buf_t uvbufs[] = {{.base = udpmsgbuf + 2, .len = *(uint16_t *)udpmsgbuf}};
    nread = uv_udp_try_send(udp_handle, uvbufs, 1, NULL);
    if (nread < 0) {
        LOGERR("[udp_socks5_resp_read_cb] failed to send data to socks5 server: (%zd) %s", -nread, uv_strerror(nread));
    } else {
        IF_VERBOSE {
            socks5_udp4msg_t *udp4msghdr = udpmsgbuf + 2;
            bool isipv4 = udp4msghdr->addrtype == SOCKS5_ADDRTYPE_IPV4;
            portno_t portno = 0;
            if (isipv4) {
                inet_ntop(AF_INET, &udp4msghdr->ipaddr4, g_udp_ipstrbuf, IP4STRLEN);
                portno = ntohs(udp4msghdr->portnum);
            } else {
                socks5_udp6msg_t *udp6msghdr = udpmsgbuf + 2;
                inet_ntop(AF_INET6, &udp6msghdr->ipaddr6, g_udp_ipstrbuf, IP6STRLEN);
                portno = ntohs(udp6msghdr->portnum);
            }
            size_t length = uvbufs[0].len - (isipv4 ? sizeof(socks5_udp4msg_t) : sizeof(socks5_udp6msg_t));
            LOGINF("[udp_socks5_resp_read_cb] send %zu bytes data to %s#%hu via socks5", length, g_udp_ipstrbuf, portno);
        }
    }
    free(udpmsgbuf);
    return;

RELEASE_CLIENT_ENTRY:
    cltcache_del(&g_udp_cltcache, client_entry);
    udp_cltentry_release(client_entry);
}

/* read data from the socks5 server (close connection) */
static void udp_socks5_tcp_read_cb(uv_stream_t *tcp_handle, ssize_t nread, const uv_buf_t *uvbuf) {
    (void) uvbuf;
    if (nread == 0) return;
    uv_read_stop(tcp_handle);
    cltentry_t *client_entry = tcp_handle->data;

    if (nread == UV_EOF) {
        IF_VERBOSE LOGINF("[udp_socks5_tcp_read_cb] udp tunnel closed by server, release resources");
    } else if (nread < 0) {
        LOGERR("[udp_socks5_tcp_read_cb] socket error occurred in udp tunnel: (%zd) %s", -nread, uv_strerror(nread));
    } else if (nread > 0) {
        LOGERR("[udp_socks5_tcp_read_cb] received undefined protocol data from udp tunnel");
    }

    cltcache_del(&g_udp_cltcache, client_entry);
    udp_cltentry_release(client_entry);
}

/* populate the uvbuf structure before the read_cb call */
static void udp_client_alloc_cb(uv_handle_t *client, size_t sugsize, uv_buf_t *uvbuf) {
    (void) client; (void) sugsize;
    uvbuf->base = g_udp_packetbuf;
    uvbuf->len = UDP_PACKET_MAXSIZE;
}

/* receive udpmsg from the udp tunnel of the socks5 server */
static void udp_client_recv_cb(uv_udp_t *udp_handle, ssize_t nread, const uv_buf_t *uvbuf, const skaddr_t *addr, unsigned flags) {
    (void) addr;
    if (nread == 0) return;
    cltentry_t *client_entry = udp_handle->data;

    if (nread < 0) {
        LOGERR("[udp_client_recv_cb] failed to recv data from socks5 server: (%zd) %s", -nread, uv_strerror(nread));
        goto RELEASE_CLIENT_ENTRY;
    }

    if (nread < (ssize_t)sizeof(socks5_udp4msg_t)) {
        LOGERR("[udp_client_recv_cb] udp message length is too small: %zd < %zu", nread, sizeof(socks5_udp4msg_t));
        goto RELEASE_CLIENT_ENTRY;
    }

    socks5_udp4msg_t *udp4msg = (void *)uvbuf->base;
    bool isipv4 = udp4msg->addrtype == SOCKS5_ADDRTYPE_IPV4;
    size_t msghdrlen = isipv4 ? sizeof(socks5_udp4msg_t) : sizeof(socks5_udp6msg_t);
    if (udp4msg->reserved != 0) {
        LOGERR("[udp_client_recv_cb] udp message reserved is not zero: %#hx", udp4msg->reserved);
        goto RELEASE_CLIENT_ENTRY;
    }
    if (udp4msg->fragment != 0) {
        LOGERR("[udp_client_recv_cb] udp message fragment is not zero: %#hhx", udp4msg->fragment);
        goto RELEASE_CLIENT_ENTRY;
    }
    if (!isipv4 && nread < (ssize_t)sizeof(socks5_udp6msg_t)) {
        LOGERR("[udp_client_recv_cb] udp message length is too small: %zd < %zu", nread, sizeof(socks5_udp6msg_t));
        goto RELEASE_CLIENT_ENTRY;
    }

    if (flags & UV_UDP_PARTIAL) {
        IF_VERBOSE LOGINF("[udp_client_recv_cb] received a partial packet, receive buffer is too small");
    }

    cltcache_use(&g_udp_cltcache, client_entry);
    uv_timer_start(client_entry->free_timer, udp_cltentry_timer_cb, g_udpidletmo * 1000, 0);

    ip_port_t server_key = {{0}, 0};
    if (isipv4) {
        server_key.ip.ip4 = udp4msg->ipaddr4;
        server_key.port = udp4msg->portnum;
    } else {
        socks5_udp6msg_t *udp6msg = (void *)uvbuf->base;
        memcpy(&server_key.ip.ip6, &udp6msg->ipaddr6, IP6BINLEN);
        server_key.port = udp6msg->portnum;
    }

    IF_VERBOSE {
        if (isipv4) {
            inet_ntop(AF_INET, &server_key.ip.ip4, g_udp_ipstrbuf, IP4STRLEN);
        } else {
            inet_ntop(AF_INET6, &server_key.ip.ip6, g_udp_ipstrbuf, IP6STRLEN);
        }
        portno_t portno = ntohs(server_key.port);
        LOGINF("[udp_client_recv_cb] recv %zd bytes data from %s#%hu via socks5", nread - msghdrlen, g_udp_ipstrbuf, portno);
    }

    svrentry_t *server_entry = svrcache_get(&g_udp_svrcache, &server_key);
    if (!server_entry) {
        skaddr6_t bind_skaddr = {0};
        if (isipv4) {
            skaddr4_t *skaddr = (void *)&bind_skaddr;
            skaddr->sin_family = AF_INET;
            skaddr->sin_addr.s_addr = server_key.ip.ip4;
            skaddr->sin_port = server_key.port;
        } else {
            bind_skaddr.sin6_family = AF_INET6;
            memcpy(&bind_skaddr.sin6_addr.s6_addr, &server_key.ip.ip6, IP6BINLEN);
            bind_skaddr.sin6_port = server_key.port;
        }

        int svr_sockfd = isipv4 ? new_udp4_respsock_tproxy() : new_udp6_respsock_tproxy();
        if (bind(svr_sockfd, (void *)&bind_skaddr, isipv4 ? sizeof(skaddr4_t) : sizeof(skaddr6_t)) < 0) {
            LOGERR("[udp_client_recv_cb] failed to bind address for udp%c socket: (%d) %s", isipv4 ? '4' : '6', errno, errstring(errno));
            close(svr_sockfd);
            return;
        }

        server_entry = malloc(sizeof(svrentry_t));
        memcpy(&server_entry->svr_ipport, &server_key, sizeof(ip_port_t));
        server_entry->svr_sockfd = svr_sockfd;
        server_entry->free_timer = malloc(sizeof(uv_timer_t));
        uv_timer_init(udp_handle->loop, server_entry->free_timer);
        server_entry->free_timer->data = server_entry;

        svrentry_t *deleted_entry = svrcache_put(&g_udp_svrcache, server_entry);
        if (deleted_entry) udp_svrentry_release(deleted_entry);
    }
    uv_timer_start(server_entry->free_timer, udp_svrentry_timer_cb, g_udpidletmo * 1000, 0);

    ip_port_t *client_keyptr = &client_entry->clt_ipport;
    skaddr6_t client_skaddr = {0};
    if (isipv4) {
        skaddr4_t *skaddr = (void *)&client_skaddr;
        skaddr->sin_family = AF_INET;
        skaddr->sin_addr.s_addr = client_keyptr->ip.ip4;
        skaddr->sin_port = client_keyptr->port;
    } else {
        client_skaddr.sin6_family = AF_INET6;
        memcpy(&client_skaddr.sin6_addr.s6_addr, &client_keyptr->ip.ip6, IP6BINLEN);
        client_skaddr.sin6_port = client_keyptr->port;
    }

    if (sendto(server_entry->svr_sockfd, (void *)uvbuf->base + msghdrlen, nread - msghdrlen, 0, (void *)&client_skaddr, isipv4 ? sizeof(skaddr4_t) : sizeof(skaddr6_t)) < 0) {
        LOGERR("[udp_client_recv_cb] failed to send data to local client: (%d) %s", errno, errstring(errno));
    } else {
        IF_VERBOSE {
            if (isipv4) {
                inet_ntop(AF_INET, &client_keyptr->ip.ip4, g_udp_ipstrbuf, IP4STRLEN);
            } else {
                inet_ntop(AF_INET6, &client_keyptr->ip.ip6, g_udp_ipstrbuf, IP6STRLEN);
            }
            portno_t portno = ntohs(client_keyptr->port);
            LOGINF("[udp_client_recv_cb] send %zd bytes data to %s#%hu via tproxy", nread - msghdrlen, g_udp_ipstrbuf, portno);
        }
    }
    return;

RELEASE_CLIENT_ENTRY:
    cltcache_del(&g_udp_cltcache, client_entry);
    udp_cltentry_release(client_entry);
}

/* udp client idle timer expired */
static void udp_cltentry_timer_cb(uv_timer_t *timer) {
    IF_VERBOSE LOGINF("[udp_cltentry_timer_cb] udp client idle timeout, release related resources");
    udp_cltentry_release(timer->data);
}

/* udp server idle timer expired */
static void udp_svrentry_timer_cb(uv_timer_t *timer) {
    IF_VERBOSE LOGINF("[udp_svrentry_timer_cb] udp server idle timeout, release related resources");
    udp_svrentry_release(timer->data);
}

/* release udp client related resources */
static void udp_cltentry_release(cltentry_t *entry) {
    uv_close((void *)entry->tcp_handle, (void *)free);
    if (entry->free_timer) {
        uv_close((void *)entry->udp_handle, (void *)free);
        uv_close((void *)entry->free_timer, (void *)free);
    } else {
        free(entry->udp_handle);
    }
    free(entry);
}

/* release udp server related resources */
static void udp_svrentry_release(svrentry_t *entry) {
    uv_close((void *)entry->free_timer, (void *)free);
    close(entry->svr_sockfd);
    free(entry);
}
