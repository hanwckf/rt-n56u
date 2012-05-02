#ifndef _BASE_64_UTILS_INCLUDED
#define _BASE_64_UTILS_INCLUDED

#include    <stdlib.h>                      /* calloc and free prototypes.*/
#include    <stdio.h>                       /* printf prototype.*/
#include    <string.h>                      /* str* and memset prototypes.*/

#ifndef MY_UCHAR
    typedef
    unsigned
    char    MY_UCHAR;                              /* Define unsigned char as MY_UCHAR.*/
#endif

#ifndef __cplusplus
	typedef int bool;
	#define false (1 == 0)
	#define true  !false
#endif


bool    b64help(void);                      /*Displays brief help messages.*/
char*   b64encode(char *);                  /* Encodes a string to Base64.*/
char*   b64decode(char *);                  /* Decodes a string to ASCII.*/
void    b64stats(char *, char *, bool);     /* Display encoding/decoding stats.*/
bool    b64valid(MY_UCHAR *);                  /* Tests for a valid Base64 char.*/
char*   b64isnot(char *, char *);           /* Displays an invalid message.*/
char*   b64buffer(char *, bool);            /* Alloc. encoding/decoding buffer.*/

/* Macro definitions:*/

#define b64is7bit(c)  ((c) > 0x7f ? 0 : 1)  /* Valid 7-Bit ASCII character?*/
#define b64blocks(l) (((l) + 2) / 3 * 4 + 1)/* Length rounded to 4 byte block.*/
#define b64octets(l)  ((l) / 4  * 3 + 1)    /* Length rounded to 3 byte octet.*/ 

/* Note:    Tables are in hex to support different collating sequences*/
/*---------------------------------------------------------------------------*/
/* b64encode - Encode a 7-Bit ASCII string to a Base64 string.               */
/* ===========================================================               */
/*                                                                           */
/* Call with:   char *  - The 7-Bit ASCII string to encode.                  */
/*                                                                           */
/* Returns:     char* - new character array that contains the new base64     */
/*              encoded string -- call free() on the resultant pointer!      */
/*              modified for formatting from original 10/24/2001 by ces      */
/*---------------------------------------------------------------------------*/
char* b64encode(char *s);

/*---------------------------------------------------------------------------*/
/* b64decode - Decode a Base64 string to a 7-Bit ASCII string.               */
/* ===========================================================               */
/*                                                                           */
/* Call with:   char *  - The Base64 string to decode.                       */
/*                                                                           */
/* Returns:     char* - new character array that contains the new base64     */
/*              decoded string -- call free() on the resultant pointer!      */
/*              modified from original 10/24/2001 by ces                     */
/*---------------------------------------------------------------------------*/
char* b64decode(char *s);

#endif
