#include <stdio.h>
#include <netdb.h>
int main(void)
{
	struct netent *net;
	setnetent(0);
	while ((net = getnetent())) {
		while (net->n_net && !((net->n_net >> 24) & 0xff)) {
			net->n_net <<= 8;
		}
		printf("%lu.%lu.%lu.%lu\n",
			   (net->n_net >> 24) & 0xff, (net->n_net >> 16) & 0xff,
			   (net->n_net >> 8) & 0xff, net->n_net & 0xff);
	}
	endnetent();
	return 0;
}
