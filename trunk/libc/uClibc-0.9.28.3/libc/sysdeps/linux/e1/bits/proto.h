#ifndef _E1_PROTO_H_
#define _E1_PROTO_H_
int kprintf( char *msg, int len);
#define KPRINTF(msg)  kprintf(msg, strlen(msg)+1)
#endif
