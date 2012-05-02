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
 * ASUS Home Gateway Reference Design
 * Web Page Configuration Support Routines
 *
 * Copyright 2001, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of ASUSTeK Inc.;
 * the contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of ASUSTeK Inc..
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <syslog.h>
#include <sys/klog.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <netinet/in.h>
#include <shutils.h>
#include <bcmnvram_f.h>
#include <common.h>

#define sys_upgrade(url) (1) 
#define sys_stats(url)   (1)

#define MAX_LINE_SIZE 512
#define UPNP_E_SUCCESS 0
#define UPNP_E_INVALID_ARGUMENT -1

char *serviceId;

static unsigned long
inet_atoul(char *cp)
{
	struct in_addr in;

	inet_aton(cp, &in);
	return in.s_addr;
}

static int
validate_portrange(char *value, struct variable *v)
{
	return UPNP_E_SUCCESS;
}

static int
validate_wlchannel(char *value, struct variable *v)
{
}

static int
validate_wlwep(char *value, struct variable *v)
{
}

static int
validate_wlkey(char *value, struct variable *v)
{
}

static int
validate_wlrate(char *value, struct variable *v)
{
}

static int
validate_wlphrase(char *value, struct variable *v)
{
}


static int
validate_ipaddr(char *value, struct variable *v)
{
#ifndef WL520G
	struct in_addr ipaddr;

	if (!inet_aton(value, &ipaddr)) {		
		return UPNP_E_INVALID_ARGUMENT;
	}

	if (v->argv) 
	{
	}
#endif
	return UPNP_E_SUCCESS;
}

static int
validate_choice(char *value, struct variable *v)
{
#ifndef WL520G
	char **choice;

	for (choice = v->argv; *choice; choice++) {
		if (!strcmp(value, *choice))
			return UPNP_E_SUCCESS;
	}
	
	return UPNP_E_INVALID_ARGUMENT;
#else
	return UPNP_E_SUCCESS;
#endif
}

static int
validate_range(char *value, struct variable *v)
{
#ifndef WL520G
	int n, start, end;

	n = atoi(value);
	start = atoi(v->argv[0]);
	end = atoi(v->argv[1]);
    
	if (n < start || n > end) {		
		return UPNP_E_INVALID_ARGUMENT;
	}
#endif
	return UPNP_E_SUCCESS;
}


static int
validate_string(char *value, struct variable *v)
{
#ifndef WL520G
	int n, max;

	n = atoi(value);
	max = atoi(v->argv[0]);
	syslog(LOG_INFO, "Validate string: %s %x %x\n", value, strlen(value), max);
	if (strlen(value) > max) {		
		return UPNP_E_INVALID_ARGUMENT;
	}
#endif
	return UPNP_E_SUCCESS;
}

static void
validate_group(char *value, struct variable *v)
{
   return (UPNP_E_SUCCESS);
}

static int
validate_hwaddr(char *value, struct variable *v)
{
#ifndef WL520G
	unsigned int hwaddr[6];

	/* Make exception for "NOT IMPLELEMENTED" string */
	if (!strcmp(value,"NOT_IMPLEMENTED")) 
		return (UPNP_E_SUCCESS);

	/* Check for bad, multicast, broadcast, or null address */
	
	if (sscanf(value, "%x:%x:%x:%x:%x:%x",
		   &hwaddr[0], &hwaddr[1], &hwaddr[2],
		   &hwaddr[3], &hwaddr[4], &hwaddr[5]) != 6 ||
	    (hwaddr[0] & 1) ||
	    (hwaddr[0] & hwaddr[1] & hwaddr[2] & hwaddr[3] & hwaddr[4] & hwaddr[5]) == 0xff ||
	    (hwaddr[0] | hwaddr[1] | hwaddr[2] | hwaddr[3] | hwaddr[4] | hwaddr[5]) == 0x00) {		
		return UPNP_E_INVALID_ARGUMENT;
	}
#endif
	return UPNP_E_SUCCESS;
}

#include "variables.c"

void InitVariables(void)
{
	/*nvram_init("all");*/
}

/* API export for UPnP function */
int LookupServiceId(char *serviceId)
{    	
    int sid;
    
    sid = 0;
    	
    while (svcLinks[sid].serviceId!=NULL)
    {      
	if ( strcmp(serviceId, svcLinks[sid].serviceId) == 0)
	   break;
	sid ++;   
    } 
    
    if (svcLinks[sid].serviceId==NULL)
       return (-1);
    else return (sid);
}   

char *GetServiceId(int sid)
{	
    return (svcLinks[sid].serviceId);   
} 

struct variable *GetVariables(int sid)
{	
    return (svcLinks[sid].variables);   
}

#ifndef WL520G
struct action *GetActions(int sid)
{	
    return (svcLinks[sid].actions);   
}

char *GetServiceType(int sid)
{
    return (svcLinks[sid].serviceType);
}

struct action *CheckActions(int sid, char *name)
{
   struct action *a;
   	
   for (a = GetActions(sid); a->name != NULL; a++) 
   {
      if (strcmp(a->name, name)==0)	
      {
      	 return (a);      	
      }
   }
   return NULL; 	
}
#endif

int CheckVariables(int sid, char *name, char *var)
{	
   struct variable *v;
   	
   for (v = GetVariables(sid); v->name != NULL; v++) 
   {
      syslog(LOG_INFO, "Check variables: %s %s\n", v->name, name);	
      if (strcmp(v->name, name)==0)	
      {
      	if (v->validate!=NULL)
      	{ 
	   if ((v->validate(var, v))==UPNP_E_SUCCESS)
      	      return 1;
      	   else return 0;   
      	}      
      	else return 1;   
      }
   }
   return 0;
}

struct variable *LookupGroupVariables(int sid, char *groupName)
{    	
   struct variable *v;
     	
   /* Find group */	
   for (v = GetVariables(sid); v->name != NULL; v++) 
   {      
      if (strcmp(v->name, groupName)==0)	
      {
      	  break;
      }	    
   }
   
   if (v->name==NULL||v->argv[0]==NULL) return NULL; 
   
   return (v);	     
}   


#ifndef WL520G
int CheckGroupVariables(int sid, struct variable *gvs, char *name, char *var)
{  
   struct variable *gv;
	    
   /* Find member of group */
   for (gv = gvs->argv[0]; gv->name!=NULL; gv++)
   {     
      if (strcmp(gv->name, name)==0)
      {	   	
	 if (gv->validate!=NULL)
      	 { 
	   if ((gv->validate(var, gv))==UPNP_E_SUCCESS)
      	      return 1;
      	   else return 0;   
      	 }      
      }      
   }      
   return 0;
}
#endif

char *GetVariable(int sid, char *name)
{
   struct variable *v;
   char buf[MAX_LINE_SIZE];
   	
   for (v = GetVariables(sid); v->name != NULL; v++) 
   {     	
      if (strcmp(v->name, name)==0)	
      {
      	 if (strcmp(v->longname, "Status")!=0)
      	 {
      	    /* syslog(LOG_INFO, "Get variables: %s %s\n", v->name, name, nvram_safe_get_x(svcLinks[sid].serviceId, name));	*/
      	    return (nvram_safe_get_x(svcLinks[sid].serviceId, name));
      	 }   
      	 else   
      	 {
      	    strcpy(buf, nvram_safe_get_f(v->argv[0], v->argv[1]));	
      	    /*syslog(LOG_INFO, "Get variables from file: %s %s %s\n", v->argv[0], v->argv[1], buf);	      	    */
	    return (buf);	
	 }   
      }
   }
   return ("");
   
}

void SetVariable(int sid, char *name, char *value)
{
   nvram_set_x(svcLinks[sid].serviceId, name, value);	
}
   
#ifdef REMOVE_NVRAM   
void SetGroupVariable(int sid, struct variable *gvs, char *name, char *value, char *action)
{
   int groupCount;     
   int i;
   char buf[MAX_LINE_SIZE];
		 
   if (strcmp(action, "Add")==0)
   {	       	
      nvram_add_list_x(svcLinks[sid].serviceId, name, value);
   }
   else if (strcmp(action, "Del")==0)
   {  
      for (i=0; i< groupCount; i++)
      {
      	 strcpy(buf, nvram_get_list_x(svcLinks[sid].serviceId, name, i+1));      
      	 if (strcmp(buf,value)==0)
	 {
	       syslog(LOG_INFO, "Del group variables: %d %d %s %s\n", i, groupCount, buf, value);      	    	      			   		 	
	       nvram_del_list_x(svcLinks[sid].serviceId, name, i);  
	       break;
	 }      
      }	 
   }	      
}

char *GetGroupVariable(int sid, struct variable *gvs, char *name)
{
   struct variable *gv;
   	
   for (gv = gvs->argv[0]; gv->name != NULL; gv++)
   {      
      if (strcmp(gv->name, name)==0)	
      {
      	 if (strcmp(gv->longname, "Status")!=0)
      	 {
      	    /*syslog(LOG_INFO, "Get group variables: %s %s\n", gv->name, name);		*/
      	    return (nvram_safe_get_x(svcLinks[sid].serviceId, name));
      	 }   
      	 else   
      	 {
      	    /*syslog(LOG_INFO, "Get group variables from file: %s %s\n", gv->argv[0], gv->argv[1]);		*/
	    return (nvram_safe_get_f(gv->argv[0], gv->argv[1]));	
	 }   
      }
   }
   return ("");   
}
#endif
