#ifndef __IPV6_NAT_H__
#define __IPV6_NAT_H__

#define NAT66_DEBUG

#define PREFIX_LEN 48     /* Global Prefix Length, more see GSE draft  */

#ifdef  NAT66_DEBUG
#define DBUG(fmt,args...) printk(KERN_DEBUG fmt,##args)
#else
#define DBUG(fmt,args...) 
#endif

#define NIP6_48(addr) \
        ntohs((addr).s6_addr16[0]), \
        ntohs((addr).s6_addr16[1]), \
        ntohs((addr).s6_addr16[2])

#define NIP6(addr) \
        ntohs((addr).s6_addr16[0]), \
        ntohs((addr).s6_addr16[1]), \
        ntohs((addr).s6_addr16[2])

#define NIP6_FMT_48 "%04x:%04x:%04x"

#define NIP6_FMT "%04x:%04x:%04x"

#endif //end of __IPV6_NAT_H__
