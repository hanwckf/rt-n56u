/*
 *broadcast_client.c - 多播的客户端
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/select.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/time.h>

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <syslog.h>
#include <time.h>
#include <sys/uio.h>

#include "ifaddr.h"

//CCTV8HD 239.93.1.14.2225
//239.93.0.1:8000
//8000, "239.93.0.1"
//CCTV1HD 239.93.0.184.5140 239.93.0.184.5140

//CCTV-6-HD 239.93.1.14.2225

//CCTV-12高清 239.93.1.30:8124

#define MCAST_PORT    8000
#define MCAST_ADDR    "239.93.0.1"

#define LOCAL_ADDR    "eth2"

/*一个局部连接多播地址，路由器不进行转发*/

#define MCAST_INTERVAL 1 /*发送间隔时间*/

#define BUFF_SIZE 1500 /*接收缓冲区大小*/

/* set/clear file/socket's mode as non-blocking
 */
int
set_nblock(int fd, int set) {
    int flags = 0;

    flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        //perror(   errno, "%s: fcntl() getting flags on fd=[%d]",
        //            __func__, fd );
        return -1;
    }

    if (set)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;

    if (fcntl(fd, F_SETFL, flags) < 0) {
        //perror(   errno, "%s: fcntl() %s non-blocking mode "
        //        "on fd=[%d]", __func__, (set ? "setting" : "clearing"),
        //        fd );
        return -1;
    }

    return 0;
}

int main(int argc, char*argv[]) {

    int s; /*套接字文件描述符*/

    struct sockaddr_in local_addr; /*本地地址*/
    struct sockaddr_in rcv_addr; /*本地地址*/

    int err = -1;

    char maddr[64];
    int mport = 0;
    char mladdr[64];

	printf("usage: mcast x.x.x.x port a.a.a.a \nor\n");
	printf("usage: mcast x.x.x.x port ifname.\n\n");
		
    if (argc == 4) {
        strcpy(maddr, argv[1]);
        mport = atoi(argv[2]);
        strcpy(mladdr, argv[3]);
    } else {
        strcpy(maddr, MCAST_ADDR);
        mport = MCAST_PORT;
        strcpy(mladdr, LOCAL_ADDR);
    }

    if (0 != get_ipv4_address(mladdr, mladdr, sizeof (mladdr))) {	
        perror("get multicast address error.");
        return -1;
    }else{
		printf("addr:%s.\n",mladdr);
	}	

    // MCAST_PORT    MCAST_ADDR   
    printf("MCast:%s:%d\n", maddr, mport);

    s = socket(AF_INET, SOCK_DGRAM, 0); /*建立套接字*/

    if (s == -1) {
        perror("socket()");
        return -1;
    }

    /*初始化地址*/

    memset(&local_addr, 0, sizeof (local_addr));

    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = inet_addr(maddr); //htonl(INADDR_ANY);
    local_addr.sin_port = htons(mport);

    /*绑定socket*/

    err = bind(s, (struct sockaddr*) &local_addr, sizeof (local_addr));

    if (err < 0) {
        perror("bind()");
        return -2;
    }

    /*设置回环许可*/

    int loop = 1;

    err = setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof (loop));

    if (err < 0) {
        perror("setsockopt():IP_MULTICAST_LOOP");
        return -3;
    }

    struct ip_mreq mreq; /*加入广播组*/

    mreq.imr_multiaddr.s_addr = inet_addr(maddr); /*广播地址*/

    printf("%08X -- %08X\n", inet_addr(maddr), inet_addr(mladdr));

    mreq.imr_interface.s_addr = inet_addr(mladdr); //htonl(INADDR_ANY); 
    //mreq.imr_interface.s_addr    = htonl(INADDR_ANY); 

    /*网络接口为默认*/

    /*将本机加入广播组*/

    err = setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof (mreq));

    if (err < 0) {
        perror("setsockopt():IP_ADD_MEMBERSHIP");
        return -4;
    }

    printf("setsockopt():IP_ADD_MEMBERSHIP OK. ---\n");

    int times = 0;

    int addr_len = 0;

    char buff[BUFF_SIZE];

    int n = 0;

    set_nblock(s, 0);

    /*循环接收广播组的消息，5000次后退出*/

    for (times = 0; 1; times++) //times<50000
    {
        addr_len = sizeof (rcv_addr);
        memset(buff, 0, BUFF_SIZE); /*清空接收缓冲区*/

        memset(&rcv_addr, 0, sizeof (rcv_addr));

        /*接收数据*/
        n = recvfrom(s, buff, BUFF_SIZE, 0, (struct sockaddr*) &rcv_addr,
            &addr_len);

        if (n == -1) {
            perror("recvfrom()");
        } else /*打印信息*/ {
            if (n > 0)buff[n - 1] = 0;
            if (n > 60)buff[59] = 0;

            printf("Recv %4d st, %d bytes from server: %08X:%d [%s]\n",
                times, n,
                rcv_addr.sin_addr.s_addr, htons(local_addr.sin_port),
                buff);
        }

        //sleep(MCAST_INTERVAL);
    }

    /*退出广播组*/

    err = setsockopt(s, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof
        (mreq));

    close(s);

    return 0;

}
