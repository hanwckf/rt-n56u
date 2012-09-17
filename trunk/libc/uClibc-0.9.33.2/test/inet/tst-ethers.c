#include <netinet/ether.h>
#include <stdio.h>

#define ETHER_LINE_LEN 256

/* This test requires /etc/ethers to exist
 * and to have host "teeth". For example:
 * 00:11:22:33:44:55 teeth
 */

int main(void)
{
	struct ether_addr addr;
	char host[ETHER_LINE_LEN];
	int i;
	int res = ether_hostton("teeth", &addr);

	if (res)
		return 1;

	for (i = 0; i < 6; i++) {
		printf("%02x", addr.ether_addr_octet[i]);
		if (i < 5)
			printf(":");
	}

	res = ether_ntohost(host, &addr);
	if (res)
		return 1;
	printf(" %s\n", host);

	return 0;
}
