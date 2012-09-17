/* Since the reentrant gethost functions take a char * buffer,
 * we have to make sure they internally do not assume alignment.
 * The actual return values are not relevant.  If the test fails,
 * it'll be due to an alignment exception which means the test
 * app is killed by the kernel.
 */

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(int argc, char *argv[])
{
	size_t i;
	char buf[1024];
	in_addr_t addr;

	addr = inet_addr("127.0.0.1");

	for (i = 0; i < sizeof(size_t) * 2; ++i) {
		struct hostent hent, *hentres;
		int ret, herr;

		printf("Testing misalignment of %2zi bytes: ", i);

		memset(&hent, 0x00, sizeof(hent));
		ret = gethostent_r(&hent, buf + i, sizeof(buf) - i, &hentres, &herr);
		printf("%sgethostent_r() ", (ret ? "!!!" : ""));

		memset(&hent, 0x00, sizeof(hent));
		ret = gethostbyname_r("localhost", &hent, buf + i, sizeof(buf) - i, &hentres, &herr);
		printf("%sgethostbyname_r() ", (ret ? "!!!" : ""));

		memset(&hent, 0x00, sizeof(hent));
		ret = gethostbyname2_r("localhost", AF_INET, &hent, buf + i, sizeof(buf) - i, &hentres, &herr);
		printf("%sgethostbyname2_r() ", (ret ? "!!!" : ""));

		memset(&hent, 0x00, sizeof(hent));
		ret = gethostbyaddr_r(&addr, sizeof(addr), AF_INET, &hent, buf + i, sizeof(buf) - i, &hentres, &herr);
		printf("%sgethostbyaddr_r() ", (ret ? "!!!" : ""));

		puts("OK!");
	}

	return 0;
}
