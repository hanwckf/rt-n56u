
#include <stdlib.h>
#include <stdio.h>


const char *strings[]={
	/* some simple stuff */
	"0", "1", "10",
	"100", "1000", "10000", "100000", "1000000",
	"10000000", "100000000", "1000000000",

	/* negative */
	"-0", "-1", "-10",
	"-100", "-1000", "-10000", "-100000", "-1000000",
	"-10000000", "-100000000", "-1000000000",

	/* test base>10 */
	"a", "b", "f", "g", "z",

	/* test hex */
	"0x0", "0x1", "0xa", "0xf", "0x10",

	/* test octal */
	"00", "01", "07", "08", "0a", "010",

	/* other */
	"0x8000000",

	/* check overflow cases: (for 32 bit) */
	"2147483645",
	"2147483646",
	"2147483647",
	"2147483648",
	"2147483649",
	"-2147483645",
	"-2147483646",
	"-2147483647",
	"-2147483648",
	"-2147483649",
	"4294967293",
	"4294967294",
	"4294967295",
	"4294967296",
	"4294967297",
	"-4294967293",
	"-4294967294",
	"-4294967295",
	"-4294967296",
	"-4294967297",

	/* bad input tests */
	"",
	"00",
	"0x",
	"0x0",
	"-",
	"+",
	" ",
	" -",
	" - 0",
};
int n_tests=sizeof(strings)/sizeof(strings[0]);



void do_test(int base);
void do_test(int base)
{
	int i;
	quad_t n;
	char *endptr;

	for(i=0;i<n_tests;i++){
		n=strtoq(strings[i],&endptr,base);
		printf("strtoq(\"%s\",%d) len=%lu res=%qd\n",
			strings[i],base,(unsigned long)(endptr-strings[i]),n);
	}
}

int main(int argc,char *argv[])
{
	do_test(0);
	do_test(8);
	do_test(10);
	do_test(16);
	do_test(36);

	return 0;
}
