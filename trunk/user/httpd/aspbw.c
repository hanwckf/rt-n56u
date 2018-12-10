#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/klog.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <httpd.h>

#include <shutils.h>

#define FW_CREATE	0
#define FW_APPEND	1
#define FW_NEWLINE	2

// for backup =========================================================

int f_write(const char *path, const void *buffer, int len, unsigned flags, unsigned cmode)
{
	static const char nl = '\n';
	int f;
	int r = -1;
	mode_t m;

	m = umask(0);
	if (cmode == 0) cmode = 0666;
	if ((f = open(path, (flags & FW_APPEND) ? (O_WRONLY|O_CREAT|O_APPEND) : (O_WRONLY|O_CREAT|O_TRUNC), cmode)) >= 0) {
		if ((buffer == NULL) || ((r = write(f, buffer, len)) == len)) {
			if (flags & FW_NEWLINE) {
				if (write(f, &nl, 1) == 1) ++r;
			}
		}
		close(f);
	}
	umask(m);
	return r;
}

// for bandwidth =========================================================

int f_exists(const char *path)	// note: anything but a directory
{
	struct stat st;
	return (stat(path, &st) == 0) && (!S_ISDIR(st.st_mode));
}

static int _f_wait_exists(const char *name, int max, int invert)
{
	max *= 40;
	while (max-- > 0) {
		if (f_exists(name) ^ invert)
			return 1;
		usleep(25000);
	}
	return 0;
}

int f_wait_exists(const char *name, int max)
{
	return _f_wait_exists(name, max, 0);
}

int f_read(const char *path, void *buffer, int max)
{
	int f;
	int n;

	if ((f = open(path, O_RDONLY)) < 0) return -1;
	n = read(f, buffer, max);
	close(f);
	return n;
}

int f_read_string(const char *path, char *buffer, int max)
{
	if (max <= 0) return -1;
	int n = f_read(path, buffer, max - 1);
	buffer[(n > 0) ? n : 0] = 0;
	return n;
}

size_t strlcpy(char *d, const char *s, size_t bufsize)
{
	size_t len = strlen(s);
	size_t ret = len;
	if (bufsize <= 0) return 0;
	if (len >= bufsize) len = bufsize-1;
	memcpy(d, s, len);
	d[len] = 0;
	return ret;
}

/**
 * hex char to int
 * @author 荒野无灯
 * @date 2016-08-01 12:30
 * @param hex
 * @return int
 */
static int hex_char_to_int(const uint8_t *hex)
{
	int val;
	if(*hex >= '0' && *hex <='9') {
		val = (*hex - '0') * 16;
	} else {
		val = (*hex - 'A' + 10) * 16;
	}

	if(*(hex+1) >= '0' && *(hex+1) <='9') {
		val += (*(hex+1) - '0');
	} else {
		val += (*(hex+1) - 'A' +10);
	}

	return val;
}


/**
 * @author 荒野无灯
 * @date 2016-08-01 12:30
 * @param str the str to detect
 * @param sz string total bytes
 * @return
 */
static int can_be_chinese_utf8(const uint8_t *str, int sz)
{
	int len = strlen (str);
	if (sz < 6) {
		return 0;
	}

	if ((len >= 6) &&
		(hex_char_to_int(str) >= 0xe4 && hex_char_to_int(str) <= 0xe9)
		&& (hex_char_to_int(str+2) >= 0x80 && hex_char_to_int(str+2) <= 0xbf)
		&& (hex_char_to_int(str+4) >= 0x80 && hex_char_to_int(str+4) <= 0xbf)
	) {
		return 1;
	}

	if (((sz - len >= 2) && (len >=4)) &&
		(hex_char_to_int(str-2) >= 0xe4 && hex_char_to_int(str-2) <= 0xe9)
		&& (hex_char_to_int(str) >= 0x80 && hex_char_to_int(str) <= 0xbf)
		&& (hex_char_to_int(str+2) >= 0x80 && hex_char_to_int(str+2) <= 0xbf)
	) {
		return 1;
	}

	if (((sz - len >= 4) && (len >= 2)) &&
		(hex_char_to_int(str-4) >= 0xe4 && hex_char_to_int(str-4) <= 0xe9)
		&& ((hex_char_to_int(str-2) >= 0x80 && hex_char_to_int(str-2) <= 0xbf)
		|| (hex_char_to_int(str) >= 0x80 && hex_char_to_int(str) <= 0xbf))
	) {
		return 1;
	}

	return 0;
}


/**
 * @author 荒野无灯
 * @date 2016-08-01 12:30
 * @param str the str to detect
 * @param sz string total bytes
 * @return int
 */
static int can_be_ascii_utf8(const uint8_t *str, int sz)

{
	int len = strlen (str);
	uint8_t the_char = hex_char_to_int(str);
	if (( len >= 2) &&
		(the_char >= '0' && the_char <= '9')
		|| (the_char >= 'A' && the_char <= 'Z')
		|| (the_char >= 'a' && the_char <= 'z')
		|| the_char == '!' || the_char == '*'
		|| the_char == '(' || the_char == ')'
		|| the_char == '_' || the_char == '-'
		|| the_char == '\'' || the_char == '.') 
	{
		return 1;
	}
	return 0;
}


/**
 * @param input the string to validate
 * @author 荒野无灯
 * @date 2016-08-01 12:30
 * @return int
 */
static int is_valid_hex_string(uint8_t *input)
{
	int i;
	int input_len, input_hex_len;
	char *input_ptr;

	//detect from index 2, skip char "0x"
	input_ptr = input+2;
	input_len = strlen(input);
	input_hex_len = input_len -2;

	int is_valid_ascii_or_Chinese = 1;

	//0xAA
	if(input_len >4 && input_len % 2 == 0 && input[0] == '0' && input[1] == 'x') {
		for ( i=2; i < input_len; i += 2) {
			if (!( ((*input_ptr>='0' && *input_ptr <='9') || ( *input_ptr >='A' && *input_ptr <='F')) && ((*(input_ptr+1) >='0' && *(input_ptr+1) <='9') || ( *(input_ptr+1) >='A' && *(input_ptr+1) <='F')))) {
				is_valid_ascii_or_Chinese = 0;
				break;
			}
			if (!can_be_chinese_utf8(input_ptr, input_hex_len) && !can_be_ascii_utf8(input_ptr, input_hex_len)) {
				is_valid_ascii_or_Chinese = 0;
				break;
			}
		}
	} else {
		is_valid_ascii_or_Chinese = 0;
	}
	return is_valid_ascii_or_Chinese;
}

void char_to_ascii(char *output, uint8_t *input)
{
	int i;
	char tmp[10];
	char *ptr;
	int input_len;

	ptr = output;
	input_len = strlen(input);

	if (is_valid_hex_string(input)) {
		for ( i=2; i<input_len; i+=2) {
			sprintf(tmp, "%%%c%c", input[i], input[i+1]);
			strcpy(ptr, tmp);
			ptr+=3;
		}
	} else {
		for ( i=0; i<input_len; i++ )
		{
			if ((input[i]>='0' && input[i] <='9')
			   ||(input[i]>='A' && input[i]<='Z')
			   ||(input[i] >='a' && input[i]<='z')
			   || input[i] == '!' || input[i] == '*'
			   || input[i] == '(' || input[i] == ')'
			   || input[i] == '_' || input[i] == '-'
			   || input[i] == '\'' || input[i] == '.')
			{
				*ptr = input[i];
				ptr++;
			}
			else
			{
				sprintf(tmp, "%%%.02X", input[i]);
				strcpy(ptr, tmp);
				ptr+=3;
			}
		}
	}

	*ptr = '\0';
}

int do_f(const char *path, webs_t wp)
{
	FILE *fp;
	char buf[1024];
	int ret = 0;

	fp = fopen(path, "r");
	if (fp) {
		while (fgets(buf, sizeof(buf), fp))
			ret += fputs(buf, wp);
		fclose(fp);
	} else {
		ret += fputs("", wp);
	}

	fflush(wp);

	return ret;
}

