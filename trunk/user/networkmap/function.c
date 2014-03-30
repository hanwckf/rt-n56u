#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <linux/if_ether.h>
#include <net/if.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include "networkmap.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include "endianness.h"

extern int scan_count;//from networkmap;

static int
is_invalid_char_for_hostname(char c)
{
	int ret = 0;

	if (c < 0x20)
		ret = 1;
	else if (c >= 0x21 && c <= 0x2c)
		ret = 1;
	else if (c >= 0x2e && c <= 0x2f)
		ret = 1;
	else if (c >= 0x3a && c <= 0x40)
		ret = 1;
#if 0
	else if (c >= 0x5b && c <= 0x60)
		ret = 1;
#else
	else if (c >= 0x5b && c <= 0x5e)
		ret = 1;
	else if (c == 0x60)
		ret = 1;
#endif
	else if (c >= 0x7b)
		ret = 1;

	return ret;
}

static int
is_valid_hostname(const char *name)
{
	int ret = 1, len, i;

	if (!name)
		return 0;

	len = strlen(name);
	if (len == 0)
		return 0;

	for (i = 0; i < len ; i++)
		if (is_invalid_char_for_hostname(name[i]))
		{
			ret = 0;
			break;
		}

	return ret;
}

/* remove space in the end of string */
void trim_r(char buf[18])
{
	int i;
	char *p = (char *) buf;
	
	i = strlen(buf);
	
	while (i >= 1)
	{
		if (*(p+i-1) == ' ' || *(p+i-1) == 0x0a || *(p+i-1) == 0x0d)
			*(p+i-1)=0x0;
		else
			break;
		i--;
	}
}

void fixstr(char buf[18])
{
	int i;
	char *p = (char *) buf;
	buf[17] = '\0';
	
	for (i = 0; i < 17; i++)
	{
		if (*p < 0x20)
			*p = 0x0;
		p++;
	}
	
	if (is_valid_hostname(buf))
		trim_r(buf);
	else
		buf[0] = '\0';
}

/***** Http Server detect function *****/
int Http_query(unsigned char *des_ip)
{
        int getlen, sock_http;
        struct sockaddr_in dest;
        char buffer[1024] = {0};
        char *dest_ip_ptr;
        struct timeval timeout={1, 0};

        /* create socket */
        sock_http = socket(AF_INET, SOCK_STREAM, 0);

        /* initialize value in dest */
        bzero(&dest, sizeof(dest));
        dest.sin_family = AF_INET;
        dest.sin_port = htons(HTTP_PORT);
        memcpy(&dest.sin_addr, des_ip, 4);
        dest_ip_ptr = inet_ntoa(dest.sin_addr);

        setsockopt(sock_http, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));//set connect timeout
        setsockopt(sock_http, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));//set receive timeout

        /* Connecting to server */
        if( connect(sock_http, (struct sockaddr*)&dest, sizeof(dest))== -1 )
        {
                close(sock_http);
                NMP_DEBUG_M("HTTP: connect error!\n");
                return 0;
        }

        sprintf(buffer,  "GET / HTTP/1.1\r\nHost: %s\r\n\r\n", dest_ip_ptr);

        if (send(sock_http, buffer, strlen(buffer), 0) == -1)
        {
                close(sock_http);
                NMP_DEBUG_M("HTTP: send error!\n");
                return 0;
        }

        bzero(buffer, sizeof(buffer));
        getlen = recv(sock_http, buffer, sizeof(buffer)-1, 0);
        if (getlen > 12)
        {
                NMP_DEBUG_M("Check http response: %s\n", buffer);
                if (!memcmp(buffer, "HTTP/1.", 7) &&
                   (!memcmp(buffer+9, "2", 1) || !memcmp(buffer+9, "3", 1) || !memcmp(buffer+9, "401", 3)) )
                {
                        close(sock_http);
                        NMP_DEBUG("Found HTTP!\n");
                        return 1;
                }
        }

        close(sock_http);
        return 0;
}

/***** NBNS Name Query function *****/
int Nbns_query(unsigned char *src_ip, unsigned char *dest_ip, NET_CLIENT* pnet_client)
{
    struct sockaddr_in my_addr, other_addr1, other_addr2;
    int sock_nbns, status, other_addr_len1, other_addr_len2, sendlen, recvlen, retry1, retry2;
    char sendbuf[50] = {0x87, 0x96, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x20, 0x43, 0x4b, 0x41,
                        0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
                        0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
                        0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
                        0x41, 0x41, 0x41, 0x41, 0x41, 0x00, 0x00, 0x21,
                        0x00, 0x01};
    char recvbuf[512] = {0};
    char *other_ptr;
    NBNS_RESPONSE *nbns_response;
    int nbns_result;
    int flags;
    struct timeval timeout = {1, 500};

    sock_nbns = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_nbns == -1)
    {
        NMP_DEBUG_M("NBNS: socket error.\n");
        return 0;
    }

    flags = 1;
    setsockopt(sock_nbns, SOL_SOCKET, SO_REUSEADDR, (char*)&flags, sizeof(flags));

    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(NBNS_PORT);    // my port
    memcpy(&my_addr.sin_addr, src_ip, 4);

    status = bind(sock_nbns, (struct sockaddr *)&my_addr, sizeof(my_addr));
    if (status < 0)
    {
        NMP_DEBUG_M("NBNS: bind error.\n");
        close(sock_nbns);
        return 0;
    }

    setsockopt(sock_nbns, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));//set timeout
    memset(&other_addr1, 0, sizeof(other_addr1));
    other_addr1.sin_family = AF_INET;
    other_addr1.sin_port = htons(NBNS_PORT);  // other port
    memcpy(&other_addr1.sin_addr, dest_ip, 4);  // other ip
    other_ptr = inet_ntoa(other_addr1.sin_addr);
    other_addr_len1 = sizeof(other_addr1);

    retry1 = 0;
    retry2 = 0;
    nbns_result = 0;
    while(1)
    {
	memset(&other_addr2, 0, sizeof(other_addr2));
	other_addr_len2 = sizeof(other_addr2);
	bzero(recvbuf, sizeof(recvbuf));
	sendlen = sendto(sock_nbns, sendbuf, sizeof(sendbuf), 0, (struct sockaddr*)&other_addr1, other_addr_len1);
	recvlen = recvfrom(sock_nbns, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&other_addr2, &other_addr_len2);
	if( recvlen > 74 )
	{
		nbns_response =(NBNS_RESPONSE *)recvbuf;
		if (((nbns_response->flags[0]>>4) == 8) && 
		     (nbns_response->number_of_names > 0) && 
		     (other_addr2.sin_addr.s_addr = other_addr1.sin_addr.s_addr) )
		{
			// check Group indicator (0x80)
			if ( !(nbns_response->name_flags1[0] & 0x80) )
			{
				memcpy(pnet_client->device_name, nbns_response->device_name1, 16);
				nbns_result = 1;
			}
			else if ( nbns_response->number_of_names > 1 && !(nbns_response->name_flags2[0] & 0x80) )
			{
				memcpy(pnet_client->device_name, nbns_response->device_name2, 16);
				nbns_result = 1;
			}
			else if ( nbns_response->number_of_names > 2 && !(nbns_response->name_flags3[0] & 0x80) )
			{
				memcpy(pnet_client->device_name, nbns_response->device_name3, 16);
				nbns_result = 1;
			}
			else if ( nbns_response->number_of_names > 3 && !(nbns_response->name_flags4[0] & 0x80) )
			{
				memcpy(pnet_client->device_name, nbns_response->device_name4, 16);
				nbns_result = 1;
			}
			
			if (nbns_result)
			{
				pnet_client->device_name[16] = 0;
				fixstr(pnet_client->device_name);
				
				NMP_DEBUG("NBNS Name: %s\n", pnet_client->device_name);
			}
			
			break;
		}
		else
		{
			retry2++;
			if(retry2 > 2)
			{
				break;
			}
		}
	}
	else
	{
		retry1++;
		if(retry1 > 1)
		{
			NMP_DEBUG_M("NBNS timeout...\n");
			break;
		}
	}
	
	sleep(1);
    }
    close(sock_nbns);

    return nbns_result;
}


int FindAllApp(unsigned char *src_ip, unsigned char *dest_ip, NET_CLIENT* pnet_client)
{
	// nbns name query
	if (Nbns_query(src_ip, dest_ip, pnet_client))
	{
		pnet_client->type = 1; // PC
	}
	
	if(scan_count==0)
		return 0;
	
	// http service detect
	if(Http_query(dest_ip))
	{
		pnet_client->http = 1;
	}
	
	return 0;
}
