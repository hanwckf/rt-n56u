/*
 * Layer Two Tunnelling Protocol Daemon Control Utility
 * Copyright (C) 2011 Alexander Dorokhov
 *
 * This software is distributed under the terms
 * of the GPL, which you should have received
 * along with this source.
 *
 * xl2tpd-control client main file
 *
 */
 
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

#include "l2tp.h"

/* Paul: Alex: can we change this to use stdout, and let applications using
 * xl2tpd-control capture the output, instead of creating tmp files?
 */
/* result filename format including absolute path and formatting %i for pid */
#define RESULT_FILENAME_FORMAT "/var/run/xl2tpd-control-%i.out"

#define ERROR_LEVEL 1
#define DEBUG_LEVEL 2

int log_level = ERROR_LEVEL;

void print_error (int level, const char *fmt, ...);

int read_result(int result_fd, char* buf, ssize_t size);

/* Definition of a command */
struct command_t
{
    char *name;
    int (*handler) (FILE*, char* tunnel, int optc, char *optv[]);
};

int command_add (FILE*, char* tunnel, int optc, char *optv[]);
int command_connect (FILE*, char* tunnel, int optc, char *optv[]);
int command_disconnect (FILE*, char* tunnel, int optc, char *optv[]);
int command_remove (FILE*, char* tunnel, int optc, char *optv[]);

struct command_t commands[] = {
    {"add", &command_add},
    {"connect", &command_connect},
    {"disconnect", &command_disconnect},
    {"remove", &command_remove},
    {NULL, NULL}
};

void usage()
{
    printf ("\nxl2tpd server version %s\n", SERVER_VERSION);
    printf ("Usage: xl2tpd-control [-c <PATH>] <command> <tunnel name> [<COMMAND OPTIONS>]\n"
            "\n"
            "    -c\tspecifies xl2tpd control file\n"
            "    -d\tspecify xl2tpd-control to run in debug mode\n"
            "--help\tshows extended help\n"
            "Available commands: add, connect, disconnect, remove\n"
    );
}

void help()
{
    usage();
    printf (
        "\n"
        "Commands help:\n"
        "\tadd\tadds new or modify existing lac configuration.\n"
        "\t\tConfiguration must be specified as command options in\n"
        "\t\t<key>=<value> pairs format.\n"
        "\t\tSee available options in xl2tpd.conf(5)\n"
        "\tconnect\ttries to activate the tunnel.\n"
        "\t\tUsername and secret for the tunnel can be passed as\n"
        "\t\tcommand options.\n"
        "\tdisconnect\tdisconnects the tunnel.\n"
        "\tremove\tremoves lac configuration from xl2tpd.\n"
        "\t\txl2tpd disconnects the tunnel before removing.\n"
        "\n"
        "See xl2tpd-control man page for more help\n"
    );
}

int main (int argc, char *argv[])
{
    char* control_filename = NULL;
    char* tunnel_name = NULL;
    struct command_t* command = NULL;
    int i; /* argv iterator */

    if (argv[1] && !strncmp (argv[1], "--help", 6))
    {
        help();
        return 0;
    }
    /* parse global options */
    for (i = 1; i < argc; i++)
    {

        if (!strncmp (argv[i], "-c", 2))
        {
            control_filename = argv[++i];
        } else if (!strncmp (argv[i], "-d", 2)) {
            log_level = DEBUG_LEVEL;
        } else {
            break;
        }
    }
    if (i >= argc)
    {
        print_error (ERROR_LEVEL, "error: command not specified\n");
        usage();
        return -1;
    }
    if (!control_filename)
    {
        control_filename = strdup (CONTROL_PIPE);
    }
    print_error (DEBUG_LEVEL, "set control filename to %s\n", control_filename);    

    /* parse command name */
    for (command = commands; command->name; command++)
    {
        if (!strcasecmp (argv[i], command->name))
        {
            i++;
            break;
        }
    }
    
    if (command->name)
    {
        print_error (DEBUG_LEVEL, "get command %s\n", command->name);
    } else {
        print_error (ERROR_LEVEL, "error: no such command %s\n", argv[i]);
        return -1;
    }
    
    /* get tunnel name */
    if (i >= argc)
    {
        print_error (ERROR_LEVEL, "error: tunnel name not specified\n");
        usage();
        return -1;
    }
    tunnel_name = argv[i++];    
    /* check tunnel name for whitespaces */
    if (strstr (tunnel_name, " "))
    {
        print_error (ERROR_LEVEL,
            "error: tunnel name shouldn't include spaces\n");
        usage();        
        return -1;
    }
    
    char buf[CONTROL_PIPE_MESSAGE_SIZE] = "";
    FILE* mesf = fmemopen (buf, CONTROL_PIPE_MESSAGE_SIZE, "w");

    /* create result pipe for reading */
    char result_filename[128];
    snprintf (result_filename, 128, RESULT_FILENAME_FORMAT, getpid());
    unlink (result_filename);
    mkfifo (result_filename, 0600);
    int result_fd = open (result_filename, O_RDONLY | O_NONBLOCK, 0600);
    if (result_fd < 0)
    {
        print_error (ERROR_LEVEL,
            "error: unable to open %s for reading.\n", result_filename);
        return -2;
    }
   
    /* turn off O_NONBLOCK */
    if (fcntl (result_fd, F_SETFL, O_RDONLY) == -1) {
        print_error (ERROR_LEVEL,
            "Can not turn off nonblocking mode for result_fd: %s\n",
            strerror(errno));
        return -2;
    }
    
    /* pass result filename to command */
    fprintf (mesf, "@%s ", result_filename);
    if (ferror (mesf))
    {
        print_error (ERROR_LEVEL, "internal error: message buffer to short");
        return -2;
    }
    
    /* format command with remaining arguments */
    int command_res = command->handler (
        mesf, tunnel_name, argc - i, argv + i
    );
    if (command_res < 0)
    {
        print_error (ERROR_LEVEL, "error: command parse error\n");
        return -1;
    }
    
    fflush (mesf);
    
    if (ferror (mesf))
    {
        print_error (ERROR_LEVEL,
            "error: message too long (max = %i ch.)\n",
            CONTROL_PIPE_MESSAGE_SIZE - 1);
        return -1;
    }
    
    print_error (DEBUG_LEVEL, "command to be passed:\n%s\n", buf);

    /* try to open control file for writing */
    int control_fd = open (control_filename, O_WRONLY, 0600);
    if (control_fd < 0)
    {
        int errorno = errno;
        switch (errorno)
        {
        case EACCES:
            print_error (ERROR_LEVEL,
                "Unable to open %s for writing."
                " Is xl2tpd running and you have appropriate permissions?\n",
                control_filename);
            break;
        default:
            print_error (ERROR_LEVEL,
                "Unable to open %s for writing: %s\n",
                control_filename, strerror (errorno));
        }
        return -1;
    }
    
    /* pass command to control pipe */
    if (write (control_fd, buf, ftell (mesf)) < 0)
    {
      int errorno = errno;
      print_error (ERROR_LEVEL,
                "Unable to write to %s: %s\n",
                control_filename, strerror (errorno));
      close (control_fd);
      return -1;
    }
    close (control_fd);
    
    /* read result from pipe */
    char rbuf[CONTROL_PIPE_MESSAGE_SIZE] = "";
    int command_result_code = read_result (
        result_fd, rbuf, CONTROL_PIPE_MESSAGE_SIZE
    );
    printf ("%s", rbuf);
    
    /* cleaning up */
    
    close (result_fd);
    unlink (result_filename);
    
    return command_result_code;
}

void print_error (int level, const char *fmt, ...)
{
    if (level > log_level) return;
    va_list args;
    va_start (args, fmt);
    vfprintf (stderr, fmt, args);
    va_end (args);
}

int read_result(int result_fd, char* buf, ssize_t size)
{
    /* read result from result_fd */
    /*FIXME: there is a chance to hang up reading.
             Should I create watching thread with timeout?
     */
    ssize_t readed;
    do
    {
        readed = read (result_fd, buf, size);
        if (readed < 0)
        {
            print_error (ERROR_LEVEL,
                "error: can't read command result: %s\n", strerror (errno));
            break;
        }
    } while (readed == 0);
    buf[readed] = '\0';
    
    /* scan result code */
    int command_result_code = -3;
    sscanf (buf, "%i", &command_result_code);
    
    return command_result_code;
}

int command_add
(FILE* mesf, char* tunnel, int optc, char *optv[])
{
    if (optc <= 0)
    {
        print_error (ERROR_LEVEL, "error: tunnel configuration expected\n");
        return -1;
    }
    fprintf (mesf, "a %s ", tunnel);
    int i;
    int wait_key = 1;
    for (i = 0; i < optc; i++)
    {
        fprintf (mesf, "%s", optv[i]);
        if (wait_key)
        {
            /* try to find '=' */
            char* eqv = strstr (optv[i], "=");
            if (eqv)
            {
                /* check is it not last symbol */
                if (eqv != (optv[i] + strlen(optv[i]) - 1))
                {
                    fprintf (mesf, ";"); /* end up option */
                } else {
                    wait_key = 0; /* now we waiting for value */
                }
            } else { /* two-word key */
                fprintf (mesf, " "); /* restore space */
            }
        } else {
            fprintf (mesf, ";"); /* end up option */        
            wait_key = 1; /* now we again waiting for key */
        }
    }
    return 0;
}

int command_connect
(FILE* mesf, char* tunnel, int optc, char *optv[])
{
    fprintf (mesf, "c %s", tunnel);
    /* try to read authname and password from opts */
    if (optc > 0) {
        if (optc == 1)
            fprintf (mesf, " %s", optv[0]);
        else // optc >= 2
            fprintf (mesf, " %s %s", optv[0], optv[1]);
    }
    return 0;
}

int command_disconnect
(FILE* mesf, char* tunnel, int optc, char *optv[])
{
    fprintf (mesf, "d %s", tunnel);
    return 0;
}

int command_remove
(FILE* mesf, char* tunnel, int optc, char *optv[])
{
    fprintf (mesf, "r %s", tunnel);
    return 0;
}

