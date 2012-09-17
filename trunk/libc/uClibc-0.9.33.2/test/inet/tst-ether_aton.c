#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <netinet/ether.h>

static struct tests
{
  const char *input;
  int valid;
  uint8_t result[6];
} tests[] =
{
  { "", 0, {0, 0, 0, 0, 0, 0} },
  { "AB:CD:EF:01:23:45", 1, {171, 205, 239, 1, 35, 69} },
  { "\022B:BB:BB:BB:BB:BB", 0, {0, 0, 0, 0, 0, 0} }
};


int
main (int argc, char *argv[])
{
  int result = 0;
  size_t cnt;

  for (cnt = 0; cnt < sizeof (tests) / sizeof (tests[0]); ++cnt)
    {
      struct ether_addr *addr;

      if (!!(addr = ether_aton (tests[cnt].input)) != tests[cnt].valid)
      {
        if (tests[cnt].valid)
          printf ("\"%s\" not seen as valid MAC address\n", tests[cnt].input);
        else
          printf ("\"%s\" seen as valid MAC address\n", tests[cnt].input);
        result = 1;
      }
      else if (tests[cnt].valid
               && memcmp(addr, &tests[cnt].result, sizeof(struct ether_addr)))
      {
        printf ("\"%s\" not converted correctly\n", tests[cnt].input);
        result = 1;
      }
    }

  return result;
}
