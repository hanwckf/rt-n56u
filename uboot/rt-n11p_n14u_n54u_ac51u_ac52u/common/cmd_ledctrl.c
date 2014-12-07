#include "cmd_tftpServer.h"

#if(CONFIG_COMMANDS & CFG_CMD_TFTPSERVER)

int do_ledon(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_ledoff(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);


U_BOOT_CMD(
    ledon, 1, 1, do_ledon,
	"ledon\t -set led on\n",
	NULL
);

U_BOOT_CMD(
    ledoff, 1, 1, do_ledoff,
	"ledoff\t -set led off\n",
	NULL
);


int do_ledon(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    LEDON();

    return 0;
}

int do_ledoff(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    LEDOFF();

    return 0;
}

#if defined(ALL_LED_OFF)
int do_all_ledon(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    ALL_LEDON();

    return 0;
}

int do_all_ledoff(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    ALL_LEDOFF();

    return 0;
}
U_BOOT_CMD(
    all_ledon, 1, 1, do_all_ledon,
	"all_ledon\t -set all_led on\n",
	NULL
);

U_BOOT_CMD(
    all_ledoff, 1, 1, do_all_ledoff,
	"all_ledoff\t -set all_led off\n",
	NULL
);
#endif

#endif
