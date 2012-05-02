/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/*
    Copyright 2001, ASUSTeK Inc.
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of ASUSTeK Inc.;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of ASUSTeK Inc..
*/
/*
 * This module creates an array of name, value pairs
 * and supports updating the nvram space. 
 *
 * This module requires the following support routines
 *
 *      malloc, free, strcmp, strncmp, strcpy, strtol, strchr, printf and sprintf
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <nvram/typedefs.h>
#include <nvram/bcmutils.h>
#include <nvram/bcmnvram.h>
#include <osl.h>

#define MAX_LINE_SIZE 512
#define MAX_FILE_NAME 64
#define MAX_HEADER_SIZE 32
#define MAX_SID	16

#define IMAGE_KERNEL_LEN 786432

char *sidMap[MAX_SID];
char *workingDir="/etc/linuxigd";

/*
 * NOTE : The mutex must be initialized in the OS previous to the point at
 *	   which multiple entry points to the nvram code are enabled 
 *
 */
#define MAX_NVRAM_SIZE 4096
#define EPI_VERSION_STR "2.4.5"

char envBuf[1024*32];

/* Remove CR/LF/Space/'"' in the end of string 
 * 
 */
 
char *strtrim(char *str)
{
   int i;
	 
   if (*str == '"')
     str++;
     
   i=strlen(str)-1;  
   	
   while (i>=0)
   {   
      if (*(str+i)==13 || *(str+i)==10 || *(str+i)==' ' || *(str+i)=='"')
      {
   	*(str+i)=0x0; 
      }	 
      else break;
      i--;
   }   
   return (str);
}



void nvram_read_sid(char *envBuf)
{
   char *p, *v;
   int i, sid=0;
      
   for (i=0; i<MAX_SID; i++)
      sidMap[sid] = NULL;
   
   p  = envBuf;
	 
   
   while (*p!=0x0)
   {
   	/*printf("Read SID: %s\n", p);*/
   	
   	if (strncmp(p, "sid_", 4)==0)
   	{
   	    v = strchr(p, '=');	
   	    v++;   	    
   	    sid = 0;
   	    
   	    if (p[4]>='0'&&p[4]<='9')
   	       sid = p[4]-'0';   	       
   	    if (p[5]>='0'&&p[5]<='9')
   	       sid = sid*10 + p[5]-'0';   
   	       
   	    if (sid<MAX_SID)      	       
   	       sidMap[sid] = v;  
   	       
   	    /*printf("Sid %d: %s\n", sid, v);*/
   	}   	
   	p = p + strlen(p) + 1;
   }	
}

void nvram_read_file(char *file, int sid)
{
   FILE *fl;
   char filename[MAX_FILE_NAME];
   char buf[MAX_LINE_SIZE];
   char item[MAX_LINE_SIZE];
   unsigned char *v, *sp;

   sprintf(filename, "%s/%s", workingDir, file);
   
   if ((fl=fopen(filename, "r+"))==NULL) 
   {
      /*printf("Open fail: %s\n", filename);*/
      return;
   }     
	
   while (fgets(buf, MAX_LINE_SIZE, fl)!=NULL)
   {    		 
      v = strchr(buf, '=');
      if (v != NULL && ((sp = strchr(buf, ' ')) == NULL || (sp > v))) 
      {
	    /* change the "name=val" string to "set name val" */
	    *v++ = '\0';
	    if (*v=='"')
	       v++;
	       
	    if (sid!=-1)
	    {	
		sprintf(item, "%d_%s", sid, buf);			  
		nvram_set(item, strtrim(v));
		/*printf("Set: %s %s\n", item, v);*/
	    }
	    else
	    {
		nvram_set(buf, strtrim(v));
		/*printf("Set: %s %s\n", buf, v);*/
	    }   
      }     
   }     
   fclose(fl);				
}

void nvram_write_files(char *envBuf, char *sid)
{
   FILE *fl;
   char filename[MAX_FILE_NAME];
   char buf[MAX_LINE_SIZE];
   char header[MAX_HEADER_SIZE];
   char name[MAX_HEADER_SIZE];
   char *p, *v;
   int i;
   
   for (i=0; i<MAX_SID; i++)
   {
   	if (sidMap[i]!=NULL && (sid==NULL || strcmp(sid, sidMap[i])==0))
   	{   	   
   	     sprintf(header, "%d_", i);	 
   	     sprintf(filename, "%s/%s", workingDir, sidMap[i]);  
   
   	     if ((fl=fopen(filename, "w+"))==NULL) return;
   	 
   	     /* printf("Open: %s\n", filename);*/
   	     
   	     p = envBuf;       		   
   
   	     while (*p!=0x0)
   	     {	 	     			     	
       		if (strncmp(p, header, strlen(header))==0)	
       		{       		   
       		   v = strchr(p, '=');		
       		   strncpy(name, p+strlen(header), v-p-strlen(header));
       		   name[v-p-strlen(header)] = 0x0;       		   		       		   
       		   v++;
       		   sprintf(buf, "%s=\"%s\"\n", name, v);       		   
       		   fputs(buf, fl);			       			  			 			 		   
       		}   
       		p = p + strlen(p) + 1;
   	     }
   	     
   	     if (fl!=NULL) fclose(fl);		       	       	    	
   	}
   }
  
}

void nvram_read_files(char *sid)
{   
   int i;
   
   for (i=0; i<MAX_SID; i++)
   {
   	if (sidMap[i]!=NULL && (sid==NULL || strcmp(sid, sidMap[i])==0))
   	{
   	     nvram_read_file(sidMap[i], i);
   	}	
   }
}

#define MIN(a, b) \
({ typedef _ta = (a), _tb = (b); \
   _ta _a = (a); _tb _b = (b); \
   _a < _b ? _a : _b; \
})

void mtd_dump_image(char *dev, unsigned long offset, unsigned long len)
{
   FILE *fdev=NULL;
   unsigned long count;
   char buf[4096];
   int i;
     
   if ((fdev = fopen(dev, "rb"))==NULL) goto err;	
   /*printf("Image open ok!\n");*/
   if (fseek(fdev, offset, SEEK_SET)!=0) goto err;
   /*printf("Image seek ok!\n");*/   
      
   printf("Dump: ");   
   
   while (len>0) 
   {
	count = fread(buf, 1, MIN(len, sizeof(buf)), fdev);
	
	if (count==0) break; 
	
	len -= count;
	for (i=0; i<count; i++)
	   printf("%2x ", (int )buf[i]);	   
   }   
   printf("\n");
   
err:   
   if (fdev) fclose(fdev);
}

void mtd_dump_ver(char *type)
{   
   FILE *fdev=NULL;
   char buf[1];
   unsigned long *imagelen;
   char dataPtr[4]; 
   char verPtr[64];
   char productid[13];
   int i;
     
   if ((fdev = fopen("/dev/mtd/1", "rb"))==NULL) return;
   /*printf("Image open ok!\n");*/
   if (fseek(fdev, 4, SEEK_SET)!=0) goto err;
   /*printf("Image seek ok!\n");*/   
	     
   fread(dataPtr, 1, 4, fdev);
		
   imagelen = dataPtr;
   //printf("Image len: %x %x %x %x\n" ,dataPtr[0], dataPtr[1], dataPtr[2], dataPtr[3]);
   
   if (fseek(fdev, *imagelen-64, SEEK_SET)!=0) goto err;
   fread(verPtr, 1, 64, fdev);
   
   if (strcmp(type, "hw")==0)
   {
      i=0; 
      while (verPtr[i]!=0)
      {	
	 printf("%d.%d ",verPtr[16+i], verPtr[17+i]); 
	 i = i+2;
      }   
   }   
   else if (strcmp(type, "fw")==0)
   {
      printf("%d.%d.%d.%d",verPtr[0], verPtr[1], verPtr[2], verPtr[3]); 
   }   
   else if (strcmp(type, "product")==0)
   {
      strncpy(productid, verPtr+4, 12);
      productid[12]=0;	
      printf("%s",productid); 
   }      
err:   
   fclose(fdev);
}


int
main(int argc, char **argv)
{
    int restore_default;
    char *s;
		
    if (argc==1)
    {
    	printf(" nvram [command] [service] 						\n");
    	printf(" [command]								\n");
    	printf("   get 				: get all variables from flash		\n");
    	printf("   get [service] 		: get variables of service from flash   \n");
    	printf("   set 				: get all variables to flash		\n");
    	printf("   set [service] 		: set variables of service to flash	\n");    
    	printf("   default			: set defalut variables to flash	\n");     	
    	printf("   dump [dev] [offset] [len]    : dump flash content 			\n"); 	
    	printf("   verinfo [type]     		: dump version information		\n"); 			
    	return 0;
    }	
    
    s = nvram_get("boardnum");
       
  
    if (strcmp(argv[1], "dump") == 0)
    {
    	if (argc==5)
    	{      	    	  	    	   
    	    mtd_dump_image(argv[2], atol(argv[3]), atol(argv[4]));
    	}    	
	else 
    	{
    	    printf("   dump [dev] [offset] [len]    : dump flash content 			\n"); 		
    	}
    	return 0;
    }
    else if (strcmp(argv[1], "verinfo") == 0)
    {
    	if (argc==3)
    	{      	    	  	    	   
    	    mtd_dump_ver(argv[2]);
    	}    	
	else 
    	{
    	    printf("   verinfo [type]     	     : dump version information\n"); 		
    	}
    	return 0;
    }
	   
    restore_default = strcmp(nvram_safe_get("restore_defaults"), "0");
    
    /*printf("Restore? %d\n", restore_default);*/
    
    if (restore_default || strcmp(argv[1], "default") == 0)
    {
	nvram_read_file("flash.default", -1);
	nvram_set("restore_defaults", "0");
    }    
	  
    /*printf("Read sid!\n");*/
    
    nvram_getall(envBuf, sizeof(envBuf));	   
    nvram_read_sid(envBuf);   
    
    if (strcmp(argv[1], "get") == 0)
    {
    	if (argc==2) /* Get all variables */
    	{    	    
    	    nvram_write_files(envBuf, NULL);   
	}
	else if (argc==3)
	{
	    nvram_write_files(envBuf, argv[2]);	
	}	
    }
    else if (strcmp(argv[1], "set") == 0)
    {
    	if (argc==2)
    	{
    	   nvram_read_files(NULL);	
    	   nvram_commit();    	  
	}
	else if (argc==3)
    	{
    	   nvram_read_files(argv[2]);
    	   nvram_commit();
    	}   
    }	
    else if (strcmp(argv[1], "default") == 0)
    {
    	nvram_commit();
    }
    return 0;
}
