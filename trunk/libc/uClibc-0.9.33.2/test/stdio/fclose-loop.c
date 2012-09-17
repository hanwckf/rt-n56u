/* From: Denis Vlasenko <vda.linux@googlemail.com>
 *	With certain combination of .config options fclose() does not
 *	remove FILE* pointer from _stdio_openlist.  As a result, subsequent
 *	fopen() may allocate new FILE structure exactly in place of one
 *	freed by previous fclose(), which then makes _stdio_openlist
 *	circularlt looped. The following program will enter infinite loop
 *	trying to walk _stdio_openlist in exit():
 */

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	FILE* fp;
	fp = fopen("/dev/null", "r");
	fclose(fp);
	fp = fopen("/dev/zero", "r");
	fclose(fp);
	return 0;
}
