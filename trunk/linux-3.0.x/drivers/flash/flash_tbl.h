#ifndef FLASH_TBL_H
#define FLASH_TBL_H

#include "flash.h"

struct flsh_sector {
    unsigned int size;
    unsigned short count;
};

struct flsh_dev {
    unsigned int man_id;
    unsigned int dev_id;
    unsigned int max_sector;
    struct flsh_sector sector[MAX_SECTOR_GROUPS];
};


static struct flsh_dev flsh_tbl[] = 
{
#ifdef CONFIG_FLASH_F49L320
 { .man_id= 0x8C, /* ESMT */
   .dev_id= 0x22F9, /* F49L320BA */
   .max_sector = 70,
   .sector= {
             {.size=KBYTES(8),.count=8},
	     {.size=KBYTES(64),.count=63}
	    }
 },
 { .man_id= 0x8C, /* ESMT */
   .dev_id= 0x22F6, /* F49L320UA */
   .max_sector = 70,
   .sector= {
	     {.size=KBYTES(64),.count=63},
             {.size=KBYTES(8),.count=8}
	    }
 },
#endif
#ifdef CONFIG_FLASH_MX29LV320
 { .man_id= 0xc2, /* Mxic */
   .dev_id= 0x22A8, /* MX29LV320B */
   .max_sector = 70,
   .sector= {
	     {.size=KBYTES(8),.count=8},
             {.size=KBYTES(64),.count=63}
	    }
 },
#endif
#ifdef CONFIG_FLASH_MX29LV160
 { .man_id= 0xc2, /* Mxic */
   .dev_id= 0x22c4, /* MX29LV160CT */
   .max_sector = 70,
   .sector= {
             {.size=KBYTES(64),.count=31},
             {.size=KBYTES(32),.count=1},
	     {.size=KBYTES(8),.count=2},
	     {.size=KBYTES(16),.count=1}
	    }
 },

 { .man_id= 0xc2, /* Mxic */
   .dev_id= 0x2249, /* MX29LV160CB */
   .max_sector = 34,
   .sector= {
	     {.size=KBYTES(16),.count=1},
	     {.size=KBYTES(8),.count=2},
             {.size=KBYTES(32),.count=1},
             {.size=KBYTES(64),.count=31}
	    }
 },
#endif
#ifdef CONFIG_FLASH_MX29LV640
 { .man_id= 0xC2, /* Mxic */
   .dev_id= 0x22C9, /* MX29LV640CT */
   .max_sector = 134,
   .sector= {
             {.size=KBYTES(64),.count=127},
             {.size=KBYTES(8),.count=8}
	    }
 },

 { .man_id= 0xC2, /* Mxic */
   .dev_id= 0x22CB, /* MX29LV640CB */
   .max_sector = 134,
   .sector= {
	     {.size=KBYTES(8),.count=8},
	     {.size=KBYTES(64),.count=127}
	    }
 },
#endif
#ifdef CONFIG_FLASH_MX29LV128
 { .man_id= 0xC2, /* Mxic */
   .dev_id= 0x227E, /* MX29LV128DT */
   .max_sector = 262,
   .sector= {
             {.size=KBYTES(64),.count=255},
             {.size=KBYTES(8),.count=8}
	    }
 },

 { .man_id= 0xC2, /* Mxic */
   .dev_id= 0x227A, /* MX29LV128DB */
   .max_sector = 262,
   .sector= {
	     {.size=KBYTES(8),.count=8},
	     {.size=KBYTES(64),.count=255}
	    }
 },
#endif
#ifdef CONFIG_FLASH_EN29LV320
 { .man_id= 0x7F, /* EON */
   .dev_id= 0x22F6, /* EN29LV320T */
   .max_sector = 70,
   .sector= {
	     {.size=KBYTES(64),.count=63},
             {.size=KBYTES(8),.count=8}
	    }
 },
 { .man_id= 0x7F, /* EON */
   .dev_id= 0x22F9, /* EN29LV320B */
   .max_sector = 70,
   .sector= {
	     {.size=KBYTES(8),.count=8},
             {.size=KBYTES(64),.count=63}
	    }
 },
#endif
#ifdef CONFIG_FLASH_EN29LV160
 { .man_id= 0x7F, /* EON */
   .dev_id= 0x22C4, /* EN29LV160AT */
   .max_sector = 34,
   .sector= {
	     {.size=KBYTES(64),.count=31},
	     {.size=KBYTES(32),.count=1},
	     {.size=KBYTES(8),.count=2},
             {.size=KBYTES(16),.count=1}
	    }
 },
 { .man_id= 0x7F, /* EON */
   .dev_id= 0x2249, /* EN29LV160AB */
   .max_sector = 34,
   .sector= {
	     {.size=KBYTES(16),.count=1},
	     {.size=KBYTES(8),.count=2},
	     {.size=KBYTES(32),.count=1},
             {.size=KBYTES(64),.count=31}
	    }
 },
#endif
#ifdef CONFIG_FLASH_K8D3X16U
 { .man_id= 0xEC, /* Samsung */
   .dev_id= 0x22A0, /* K8D3216UT */
   .max_sector = 70,
   .sector= {
	     {.size=KBYTES(64),.count=63},
             {.size=KBYTES(8),.count=8}
	    }
 },
 { .man_id= 0xEC, /* Samsung */
   .dev_id= 0x22A2, /* K8D3216UB */
   .max_sector = 70,
   .sector= {
	     {.size=KBYTES(8),.count=8},
             {.size=KBYTES(64),.count=63}
	    }
 },
 { .man_id= 0xEC, /* Samsung */
   .dev_id= 0x22A1, /* K8D3316UT */
   .max_sector = 70,
   .sector= {
	     {.size=KBYTES(64),.count=63},
             {.size=KBYTES(8),.count=8}
	    }
 },
 { .man_id= 0xEC, /* Samsung */
   .dev_id= 0x22A3, /* K8D3316UB */
   .max_sector = 70,
   .sector= {
	     {.size=KBYTES(8),.count=8},
             {.size=KBYTES(64),.count=63}
	    }
 },
#endif
#ifdef CONFIG_FLASH_SST39VF320X 
 { .man_id= SST_FL_MANUFACT, /* SST */
	 .dev_id= 0x235B, /* SST39VF3201 */
	 .max_sector = 1023,
	 .sector= {
		 {.size=KBYTES(4),.count=1024}
	 }
 },
 { .man_id= SST_FL_MANUFACT, /* SST */
	 .dev_id= 0x235A, /* SST39VF3202 */
	 .max_sector = 1023,
	 .sector= {
		 {.size=KBYTES(4),.count=1024}
	 }
 },
#endif
#ifdef CONFIG_FLASH_S29AL032X
{ .man_id= 0x1, /* SPANSION */
   .dev_id= 0x22F9, /* S29AL032D */
   .max_sector = 70,
   .sector= {
	     {.size=KBYTES(64),.count=63},
             {.size=KBYTES(8),.count=8}
	    }
 },
 { .man_id= 0x1, /* SPANSION */
   .dev_id= 0x22F6, 
   .max_sector = 70,
   .sector= {
             {.size=KBYTES(8),.count=8},
	     {.size=KBYTES(64),.count=63}
	    }
 },
#endif 
#ifdef CONFIG_FLASH_S29AL016X
{ .man_id= 0x1, /* SPANSION */
   .dev_id= 0x22C4, /* S29AL016D */
   .max_sector = 35,
   .sector= {
	     {.size=KBYTES(64),.count=31},
	     {.size=KBYTES(32),.count=1},
             {.size=KBYTES(8),.count=2},
             {.size=KBYTES(16),.count=1}
	    }
 },
 { .man_id= 0x1, /* SPANSION */
   .dev_id= 0x2249, 
   .max_sector = 35,
   .sector= {
             {.size=KBYTES(16),.count=1},
             {.size=KBYTES(8),.count=2},
	     {.size=KBYTES(32),.count=1},
	     {.size=KBYTES(64),.count=31}
	    }
 },
#endif 

#ifdef CONFIG_FLASH_S29GL032N
 /* 01,02,V1,V2:
  * 	Cycle 1 : 0x227E 
  * 	Cycle 2 : 0x221D
  * 	Cycle 3 : 0x2200
  * 03,04:
  * 	Cycle 1 : 0x227E 
  * 	Cycle 2 : 0x221A
  * 	Cycle 3 : 0x2200
  *
  * Device ID= ((Cycle1&0xFF)<<16) | ((Cycle2&0xFF<<8) | (Cycle3&0xFF)
  *
  */ 
{ .man_id= 0x1, /* SPANSION */
   .dev_id= 0x7E1A00, /* TOP 03,04 */
   .max_sector = 70,
   .sector= {
	     {.size=KBYTES(64),.count=63},
             {.size=KBYTES(8),.count=8}
	    }
},
{ .man_id= 0x1, /* SPANSION */
   .dev_id= 0x7E1D00, /* Bottom 01, 02, V1, V2 */
   .max_sector = 70,
   .sector= {
             {.size=KBYTES(8),.count=8},
	     {.size=KBYTES(64),.count=63}
	    }
},
#endif
#ifdef CONFIG_FLASH_S29GL064N
 /* 01,02,V1,V2:
  * 	Cycle 1 : 0x227E 
  * 	Cycle 2 : 0x220C
  * 	Cycle 3 : 0x2201
  * 03,04:
  * 	Cycle 1 : 0x227E 
  * 	Cycle 2 : 0x2210
  * 	Cycle 3 : 0x2200
  *
  * Device ID= ((Cycle1&0xFF)<<16) | ((Cycle2&0xFF<<8) | (Cycle3&0xFF)
  *
  */ 
{ .man_id= 0x1, /* SPANSION */
   .dev_id= 0x7E1000, /* 03,04 */
   .max_sector = 134,
   .sector= {
             {.size=KBYTES(64),.count=127},
	     {.size=KBYTES(8),.count=8}
	    }
},
{ .man_id= 0x1, /* SPANSION */
   .dev_id= 0x7E0C01, /* 01, 02, V1, V2 */
   .max_sector = 134,
   .sector= {
             {.size=KBYTES(8),.count=8},
	     {.size=KBYTES(64),.count=127}
	    }
},
#endif

#ifdef CONFIG_FLASH_S29GL256
 /* 
  * 	Cycle 1 : 0x227E 
  * 	Cycle 2 : 0x2222
  * 	Cycle 3 : 0x2201
  *
  * Device ID= ((Cycle1&0xFF)<<16) | ((Cycle2&0xFF<<8) | (Cycle3&0xFF)
  *
  */ 
{ .man_id= 0x1, /* SPANSION */
   .dev_id= 0x7E2201, 
   .max_sector = 255,
   .sector= {
             {.size=KBYTES(128),.count=256}
	    }
},
#endif

 { .man_id= 0x00,  /* END_MARK */
	 .dev_id= 0x0000, 
	 .max_sector = 0,
	 .sector= {
		 {.size=KBYTES(0),.count=0},
		 {.size=KBYTES(0),.count=0}
	 }

 }

};

#endif
