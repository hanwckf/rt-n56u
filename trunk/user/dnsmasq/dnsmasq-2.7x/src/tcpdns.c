#include "dnsmasq.h"
#include <pthread.h>

// TCPDNS session (per query)
typedef struct _TCPDNS_SESSION {
	int fd; // Original UDP socket to server (but skip sendto by us)
	size_t len; // UDP DNS payload length
	socklen_t tolen; // Server address length
	union mysockaddr to; // Server address
	unsigned short request; // TCPDNS request = length + DNS payload
	unsigned char reqbuf[]; // DNS request payload
	//unsigned short response; // TCPDNS response
	//unsigned char resbuf[]; // DNS response payload
} TCPDNS_SESSION;

// TCPDNS session worker
static void tcpdns_worker(TCPDNS_SESSION *session)
{
	session->request = htons(session->len);
	unsigned short *response = (unsigned short *)(session->reqbuf + session->len);

	int server = socket(session->to.sa.sa_family, SOCK_STREAM, IPPROTO_TCP);
	for (int i = 0; i < 3; i++) {
		if (connect(server, &session->to.sa, session->tolen) == 0 ) {
			ssize_t nsend = session->len + 2;
			int success = send(server, &session->request, nsend, 0) == nsend;
			if (success) {
				ssize_t nrecv = recv(server, response, 2 + daemon->packet_buff_sz, 0);
				success = (nrecv - 2 == htons(*response));
				if (success) {
					union mysockaddr loopback;
					socklen_t looplen = sizeof(loopback);
					getsockname(session->fd, &loopback.sa, &looplen);
					sendto(session->fd, &response[1], nrecv - 2, 0, &loopback.sa, looplen);
					break;
				}
			}
			shutdown(server, SHUT_RDWR);
		}
		sleep(1);
	}
	close(server);

	free(session);
	pthread_detach(pthread_self());
}

// Send to TCPDNS (instead of UDPDNS)
ssize_t tcpdns_sendto(int fd, const void *buf, size_t len, int flags __attribute__((unused)), const struct sockaddr *to, socklen_t tolen)
{
	TCPDNS_SESSION *session = safe_malloc(sizeof(TCPDNS_SESSION) + 2 + len + 2 + daemon->packet_buff_sz);
	session->fd = fd;
	session->len = len;
	session->tolen = tolen;
	memcpy(&session->to, to, tolen);
	memcpy(session->reqbuf, buf, len);

	pthread_t tid;
	return pthread_create(&tid, NULL, (void *(*)(void *))tcpdns_worker, session) ? (size_t)-1 : len;
}
