#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc ,char *argv[])
{
    FILE *infile, *outfile;	
    char infname[1024];
    char outfname[1024];
    char *rt28xxdir;
    char *chipset;
	char *wow, *rt28xx_mode; /* for WOW firmware */
    int i=0;//,n=0;
    unsigned char c;
   
    memset(infname,0,1024);
    memset(outfname,0,1024);
    
    rt28xxdir = (char *)getenv("RT28xx_DIR");
    chipset = (char *)getenv("CHIPSET");
	wow = (char *)getenv("HAS_WOW_SUPPORT"); /* for WOW firmware */
	rt28xx_mode = (char *)getenv("RT28xx_MODE");

    if(!rt28xxdir)
    {
         printf("Environment value \"RT28xx_DIR\" not export \n");
	 return -1;
    }
    if(!chipset)
    {
	 printf("Environment value \"CHIPSET\" not export \n");
	 return -1;
    }	    
	if (strlen(rt28xxdir) > (sizeof(infname)-100))
	{
		printf("Environment value \"RT28xx_DIR\" is too long!\n");
		return -1;
	}

    strcat(infname,rt28xxdir);
    if(strncmp(chipset, "2860",4)==0)
	    strcat(infname,"/common/rt2860.bin");
    else if(strncmp(chipset, "2870",4)==0)
	    strcat(infname,"/common/rt2870.bin");
	else if(strncmp(chipset, "3090",4)==0)
	    strcat(infname,"/common/rt2860.bin");
	else if(strncmp(chipset, "2070",4)==0)
	    strcat(infname,"/common/rt2870.bin");
	else if(strncmp(chipset, "3070",4)==0)
	    strcat(infname,"/common/rt2870.bin");
	else if(strncmp(chipset, "3572",4)==0)
	    strcat(infname,"/common/rt2870.bin");
	else if(strncmp(chipset, "3573",4)==0)
	    strcat(infname,"/common/rt2870.bin");
	else if(strncmp(chipset, "3370",4)==0)
            strcat(infname,"/common/rt2870.bin");
	else if(strncmp(chipset, "5370",4)==0)
            strcat(infname,"/common/rt2870.bin");
	else if(strncmp(chipset, "5572",4)==0)
            strcat(infname,"/common/rt2870.bin");
	else if(strncmp(chipset, "5592",4)==0)
            strcat(infname,"/common/rt2860.bin");
	else if(strncmp(chipset, "USB",3)==0)
	    strcat(infname,"/common/rt2870.bin");
	else if(strncmp(chipset, "PCI",3)==0)
	    strcat(infname,"/common/rt2860.bin");
    else
    	strcat(infname,"/common/rt2860.bin");

	/* for WOW support firmware */
	if ((wow != NULL) && (strncmp(wow, "y", 1) == 0) && (strncmp(rt28xx_mode, "STA", 3) == 0))
	{
		if ((wow = strstr(infname, "rt2870")) != NULL)
		{
			strcpy(wow, "rt2870_wow.bin");	
			fprintf(stderr, "infname %s\n", infname);
		}
	}
	    
    strcat(outfname,rt28xxdir);
    strcat(outfname,"/include/firmware.h");
     
    infile = fopen(infname,"r");
    if (infile == (FILE *) NULL)
    {
         printf("Can't read file %s \n",infname);
	 return -1;
    }
    outfile = fopen(outfname,"w");
    
    if (outfile == (FILE *) NULL)
    {
         printf("Can't open write file %s \n",outfname);
        return -1;
    }
    
    fputs("/* AUTO GEN PLEASE DO NOT MODIFY IT */ \n",outfile);
    fputs("/* AUTO GEN PLEASE DO NOT MODIFY IT */ \n",outfile);
    fputs("\n",outfile);
    fputs("\n",outfile);
    fputs("UCHAR FirmwareImage [] = { \n",outfile);
    while(1)
    {
	char cc[3];    

	c = getc(infile);
	
	if (feof(infile))
	    break;
	
	memset(cc,0,2);
	
	if (i>=16)
	{	
	    fputs("\n", outfile);	
	    i = 0;
	}    
	fputs("0x", outfile); 
	sprintf(cc,"%02x",c);
	fputs(cc, outfile);
	fputs(", ", outfile);
	i++;
    } 
    
    fputs("} ;\n", outfile);
    fclose(infile);
    fclose(outfile);
    exit(0);
}	
