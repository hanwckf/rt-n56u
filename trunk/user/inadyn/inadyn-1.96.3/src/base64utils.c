/*
Copyright (C) 2001 Clay Sampson

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/*****************************************************************************
FILENAME:   base64utils.cpp
PURPOSE:    Utility functions for base64 encoding and decodong
LOG:        11/14/2001, ces: initial
            11/27/2001, go : linux port
******************************************************************************/

/**
*** NOTE: THE CODE CONTAINED IN THIS
*** SOURCE FILE IS:
*** Copyright (c) 1994 - 2001
*** Marc Niegowski
*** Connectivity, Inc.
*** All rights reserved.
**/
#include "base64.h"

static  
const                                       /* Base64 Index into encoding*/
MY_UCHAR  pIndex[]     =   {                   /* and decoding table.*/
                        0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
                        0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50,
                        0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
                        0x59, 0x5a, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
                        0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e,
                        0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
                        0x77, 0x78, 0x79, 0x7a, 0x30, 0x31, 0x32, 0x33,
                        0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x2b, 0x2f
                        };

static
const                                       
MY_UCHAR   pBase64[]   =   {                  
                        0x3e, 0x7f, 0x7f, 0x7f, 0x3f, 0x34, 0x35, 0x36,
                        0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x7f,
                        0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x00, 0x01,
                        0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
                        0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11,
                        0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
                        0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x1a, 0x1b,
                        0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
                        0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
                        0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33
                        };
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
char* b64encode(char *s)
{
    int     l   = strlen(s);                
    int     x   = 0;                        
    char   *b, *p;                          

    while   (x < l)                         
    {                                       
        if (!b64is7bit((MY_UCHAR) *(s + x)))   
        {
            return false;                   
        }
        x++;                                
    }

    if (!(b = b64buffer(s, true)))          
        return false;                       

    memset(b, 0x3d, b64blocks(l) - 1);      

    p = b;                                  
    x = 0;                                  

    while   (x < (l - (l % 3)))             
    {
        *b++   = pIndex[  s[x]             >> 2];
        *b++   = pIndex[((s[x]     & 0x03) << 4) + (s[x + 1] >> 4)];
        *b++   = pIndex[((s[x + 1] & 0x0f) << 2) + (s[x + 2] >> 6)];
        *b++   = pIndex[  s[x + 2] & 0x3f];
         x    += 3;                         
    }

    if (l - x)                              
    {
        *b++        = pIndex[s[x] >> 2];    

        if  (l - x == 1)                    
            *b      = pIndex[ (s[x] & 0x03) << 4];
        else                            
        {                                   
            *b++    = pIndex[((s[x]     & 0x03) << 4) + (s[x + 1] >> 4)];
            *b      = pIndex[ (s[x + 1] & 0x0f) << 2];
        }
    }

    return p;                            
}

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
char* b64decode(char *s)
{
    int     l = strlen(s);                  
    char   *b, *p;                          
    MY_UCHAR   c = 0;                          
    int     x = 0;                          
    int     y = 0;

    static                                  
    const                                   
    char    pPad[]  =   {0x3d, 0x3d, 0x3d, 0x00};

    if  (l % 4)                             
        return b64isnot(s, NULL);           

    if  ((b = strchr(s, pPad[0])) != 0)            
    {                                       
        if  ((b - s) < (l - 3))             
            return b64isnot(s, NULL);       
        else                                
            if  (strncmp(b, (char *) pPad + 3 - (s + l - b), s + l - b))
                return b64isnot(s, NULL);
    }

    if  (!(b = b64buffer(s, false)))        
        return false;                       

    p = s;                                  
    x = 0;                                  

    while ((c = *s++))                      
    {                                       
        if  (c == pPad[0])                  
            break;

        if (!b64valid(&c))                  
            return b64isnot(s, b);          
        
        switch(x % 4)                       
        {                                   
        case    0:                          
            b[y]    =  c << 2;
            break;                          
        case    1:                          
            b[y]   |=  c >> 4;

            if (!b64is7bit((MY_UCHAR) b[y++])) 
                return b64isnot(s, b);      

            b[y]    = (c & 0x0f) << 4;
            break;
        case    2:                          
            b[y]   |=  c >> 2;

            if (!b64is7bit((MY_UCHAR) b[y++])) 
                return b64isnot(s, b);      

            b[y]    = (c & 0x03) << 6;
            break;
        case    3:                          
            b[y]   |=  c;

            if (!b64is7bit((MY_UCHAR) b[y++])) 
                return b64isnot(s, b);      
        }
        x++;                                
    }

    return b;                            
}

/*---------------------------------------------------------------------------*/
/* b64valid - validate the character to decode.                              */
/* ============================================                              */
/*                                                                           */
/* Checks whether the character to decode falls within the boundaries of the */
/* Base64 decoding table.                                                    */
/*                                                                           */
/* Call with:   char    - The Base64 character to decode.                    */
/*                                                                           */
/* Returns:     bool    - True (!0) if the character is valid.               */
/*                        False (0) if the character is not valid.           */
/*---------------------------------------------------------------------------*/
bool b64valid(MY_UCHAR *c)
{
    if ((*c < 0x2b) || (*c > 0x7a))         
        return false;                       
    
    if ((*c = pBase64[*c - 0x2b]) == 0x7f)  
        return false;                       

    return true;                            
}

/*---------------------------------------------------------------------------*/
/* b64isnot - Display an error message and clean up.                         */
/* =================================================                         */
/*                                                                           */
/* Call this routine to display a message indicating that the string being   */
/* decoded is an invalid Base64 string and de-allocate the decoding buffer.  */
/*                                                                           */
/* Call with:   char *  - Pointer to the Base64 string being decoded.        */
/*              char *  - Pointer to the decoding buffer or NULL if it isn't */
/*                        allocated and doesn't need to be de-allocated.     */
/*                                                                           */
/* Returns:     bool    - True (!0) if the character is valid.               */
/*                        False (0) if the character is not valid.           */
/*---------------------------------------------------------------------------*/
char* b64isnot(char *p, char *b)
{
    if  (b)                                 
        free(b);                            

    return  NULL;                          
}

/*---------------------------------------------------------------------------*/
/* b64buffer - Allocate the decoding or encoding buffer.                     */
/* =====================================================                     */
/*                                                                           */
/* Call this routine to allocate an encoding buffer in 4 byte blocks or a    */
/* decoding buffer in 3 byte octets.  We use "calloc" to initialize the      */
/* buffer to 0x00's for strings.                                             */
/*                                                                           */
/* Call with:   char *  - Pointer to the string to be encoded or decoded.    */
/*              bool    - True (!0) to allocate an encoding buffer.          */
/*                        False (0) to allocate a decoding buffer.           */
/*                                                                           */
/* Returns:     char *  - Pointer to the buffer or NULL if the buffer        */
/*                        could not be allocated.                            */
/*---------------------------------------------------------------------------*/
char *b64buffer(char *s, bool f)
{
    int     l = strlen(s);                  
    char   *b;                              

    if  (!l)                                
        return  NULL;                       

    b = (char *) calloc((f ? b64blocks(l) : b64octets(l)),sizeof(char));
   
    return  b;                              
}
