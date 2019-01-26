/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: mhl
 *
 * Created on October 10, 2018, 2:51 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>  
#include <sys/stat.h> 
#include <sys/ioctl.h> 
#include <sys/socket.h>
#include <sys/wait.h> 
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h> 
#include <stdio.h>  
#include <stdlib.h>
#include <string.h>  
#include <pthread.h>   
#include <dirent.h> 
#include <time.h>
#include <fcntl.h> 
#include <errno.h> 

#include <sys/types.h>  
#include <sys/stat.h> 
#include <sys/ioctl.h> 
#include <sys/socket.h>
#include <sys/wait.h> 
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h> 
#include <stdio.h>  
#include <stdlib.h>
#include <string.h>  
#include <pthread.h>   
#include <dirent.h> 
#include <time.h>
#include <fcntl.h> 
#include <errno.h>

#ifdef debugprintf
#define debugpri(mesg, args...) fprintf(stderr, "[NetRate print:%s:%d:] " mesg "\n", __FILE__, __LINE__, ##args) 
#else
#define debugpri(mesg, args...)
#endif

int GetNetRate(FILE* fd, char *interface, long long *recv, long long *send) {
    char buf[8192]={0};
    char *p;
    char flow[32];
    int i = 0;
    char tempstr[16][16] = {0};

    //memset(buf, 0, sizeof (buf));
    //memset(tempstr, 0, sizeof (tempstr));
    
    fseek(fd, 0, SEEK_SET);
    int nBytes = fread(buf, 1, sizeof (buf) - 1, fd);
    if (-1 == nBytes) {
        debugpri("fread error");
        fclose(fd);
        return -1;
    }
    buf[nBytes] = '\0';
    char* pDev = strstr(buf, interface);
    if (NULL == pDev) {
        printf("count not find dev %s\n%s\n", interface,buf);
        fclose(fd);
        exit(0);
        return -1;
    }

	sscanf(pDev, "%*[^:]:%s%*s%*s%*s%*s%*s%*s%*s%s%*s",
	tempstr[0], tempstr[1]);
        
	if(0){
		printf("values:%s-%s %lld %lld \n", 
            tempstr[0],tempstr[1],*recv,*send );
	}

    *recv = atoll(tempstr[0]);
    *send = atoll(tempstr[1]);
}

int main(int argc, char** argv) {

    struct timeval tv_now, tv_pre;
    char netdevice[16] = {0};
    int nDevLen;
    long long recvpre = 0, recvcur = 0;
    long long sendpre = 0, sendcur = 0;
    double sendrate;
    double recvrate;
	double deltatime=0;

    if (argc != 2) {
        printf("Usage: netrate <network device>\n");
        exit(0);
    }

    nDevLen = strlen(argv[1]);
    if (nDevLen < 1 || nDevLen > 10) {
        printf("unkown device\n");
        exit(0);
    }
    
    sprintf(netdevice, "%s:", argv[1]);
    FILE* fd = fopen("/proc/net/dev", "r");
    if (NULL == fd) {
        perror("open error!");
        //debugpri("/proc/net/dev not exists!\n");
        return -1;
    }

    //system("clear");
    printf("NetWorkRate Statistic Verson 0.0.1\n");
    printf("Net Device	receive rate	send rate\n");

    gettimeofday(&tv_pre, NULL);
    GetNetRate(fd, netdevice, &recvpre, &sendpre);

    while (1) {
        sleep(1);
        gettimeofday(&tv_now, NULL);
        GetNetRate(fd, netdevice, &recvcur, &sendcur);
        
		deltatime =tv_now.tv_sec + tv_now.tv_usec * 0.000001 - tv_pre.tv_sec - tv_pre.tv_usec * 0.000001;
		
		recvrate = (recvcur - recvpre) / (1024 * deltatime);        
		if (recvrate < 0) recvrate = 0; 
		
        sendrate = (sendcur - sendpre) / (1024 * deltatime);
        if (sendrate < 0) sendrate = 0;
		
        printf("%s\tRX:%llu s:%8.2fKB/sec | TX:%llu s:%8.2fKB/sec  %g\n", 
                netdevice, 
                recvcur,recvrate, 
                sendcur,sendrate,deltatime);
				
		recvpre = recvcur;
		sendpre = sendcur;
		
		gettimeofday(&tv_pre, NULL);
    }
    fclose(fd);
    return 0;
}

/*
 Inter-|   Receive                                                |  Transmit
 face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
  eth0:1079672451902 2150711986    0    0    0     0          0         0 2425991534968 2422512602    0  368    0     0       0          0
  eth1:264716875407 458687580   35    0    0    35          0     71708 8133007988 38430618    0    0    0     0       0          0
  eth2:2389917498315 2393300594    0    0    0     0          0         0 1207640814893 2138708200    0    0    0     0       0          0
  ppp0:3326839576 3131895    0    0    0     0          0         0 295463550 2075881    0    0    0     0       0          0 
 
 */
 