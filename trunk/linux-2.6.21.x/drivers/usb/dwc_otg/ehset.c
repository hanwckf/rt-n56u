
/*
 *  Test program for EHSET test.
 *
 *  mipsel-linux-gcc -I/xxx/xxx/xxx/RT288x_SDK/source/linux-2.6.21.x/include ehset.c
 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/usbdevice_fs.h>
#include <linux/usb_ch9.h>

#define USB_RT_HUB		(USB_TYPE_CLASS | USB_RECIP_OTHER)
#define USB_PORT_FEAT_TEST	21

main(int argc,char*argv[])
{
	int fd, ret;
	static int port = 0;
	static int test = 0;
	struct usbdevfs_ctrltransfer ctrl_req = {0};
	
	if (argc != 4) {
		fprintf(stderr, "Usage: %s /proc/bus/usb/<BusNo>/<HubID>"
				" [port] [test]\n", argv[0]);
		exit(1);
	}

	if (argc > 2) {
		port = atoi( argv[2] );
		printf( "set port to %d\n", port );
	}

	if (argc > 3) {
		test = atoi( argv[3] );
		printf( "set test to %d\n", test );
	}

	ctrl_req.bRequestType = USB_RT_HUB;
	ctrl_req.bRequest = USB_REQ_SET_FEATURE;
	ctrl_req.wValue = USB_PORT_FEAT_TEST;
	ctrl_req.wIndex = (test << 8)|port;

	errno = 0;
	fd = open(argv[1],O_RDWR);
	if (fd < 0) {
		perror("open failed:"); 
		exit(errno);
	}

	errno = 0;
	ret = ioctl(fd, USBDEVFS_CONTROL, &ctrl_req);
	printf("IOCTL return status:%d\n",ret);
	if (ret<0) {
		perror("IOCTL failed:");
		close(fd);
		exit(3);
	} else {
		close(fd);
		exit(0);
	}

   return 0;
}
