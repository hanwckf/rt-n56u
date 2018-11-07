/*
    mtr  --  a network diagnostic tool
    Copyright (C) 2016  Matt Kimball

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cmdpipe.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef HAVE_ERROR_H
#include <error.h>
#else
#include "portability/error.h"
#endif

#include "packet/cmdparse.h"
#include "display.h"


/*  Set a file descriptor to non-blocking  */
static
void set_fd_nonblock(
    int fd)
{
    int flags;

    /*  Get the current flags of the file descriptor  */
    flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        error(EXIT_FAILURE, errno, "F_GETFL failure");
        exit(1);
    }

    /*  Add the O_NONBLOCK bit to the current flags  */
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        error(EXIT_FAILURE, errno, "Failure to set O_NONBLOCK");
        exit(1);
    }
}


/*
    Send a command synchronously to mtr-packet, blocking until a result
    is available.  This is intended to be used at start-up to check the
    capabilities of mtr-packet, but probes should be sent asynchronously
    to avoid blocking other probes and the user interface.
*/
static
int send_synchronous_command(
    struct mtr_ctl *ctl,
    struct packet_command_pipe_t *cmdpipe,
    const char *cmd,
    struct command_t *result)
{
    char reply[PACKET_REPLY_BUFFER_SIZE];
    int command_length;
    int write_length;
    int read_length;

    /*  Query send-probe support  */
    command_length = strlen(cmd);
    write_length = write(cmdpipe->write_fd, cmd, command_length);

    if (write_length == -1) {
        return -1;
    }

    if (write_length != command_length) {
        errno = EIO;
        return -1;
    }

    /*  Read the reply to our query  */
    read_length =
        read(cmdpipe->read_fd, reply, PACKET_REPLY_BUFFER_SIZE - 1);

    if (read_length < 0) {
        return -1;
    }

    /*  Parse the query reply  */
    reply[read_length] = 0;
    if (parse_command(result, reply)) {
        return -1;
    }

    return 0;
}


/*  Check support for a particular feature with the mtr-packet we invoked  */
static
int check_feature(
    struct mtr_ctl *ctl,
    struct packet_command_pipe_t *cmdpipe,
    const char *feature)
{
    char check_command[COMMAND_BUFFER_SIZE];
    struct command_t reply;

    snprintf(check_command, COMMAND_BUFFER_SIZE,
             "1 check-support feature %s\n", feature);

    if (send_synchronous_command(ctl, cmdpipe, check_command, &reply) ==
        -1) {
        return -1;
    }

    /*  Check that the feature is supported  */
    if (!strcmp(reply.command_name, "feature-support")
        && reply.argument_count >= 1
        && !strcmp(reply.argument_name[0], "support")
        && !strcmp(reply.argument_value[0], "ok")) {

        /*  Looks good  */
        return 0;
    }

    errno = ENOTSUP;
    return -1;
}


/*
    Check the protocol selected against the mtr-packet we are using.
    Returns zero if everything is fine, or -1 with errno for either
    a failure during the check, or for an unsupported feature.
*/
static
int check_packet_features(
    struct mtr_ctl *ctl,
    struct packet_command_pipe_t *cmdpipe)
{
    /*  Check the IP protocol version  */
    if (ctl->af == AF_INET6) {
        if (check_feature(ctl, cmdpipe, "ip-6")) {
            return -1;
        }
    } else if (ctl->af == AF_INET) {
        if (check_feature(ctl, cmdpipe, "ip-4")) {
            return -1;
        }
    } else {
        errno = EINVAL;
        return -1;
    }

    /*  Check the transport protocol  */
    if (ctl->mtrtype == IPPROTO_ICMP) {
        if (check_feature(ctl, cmdpipe, "icmp")) {
            return -1;
        }
    } else if (ctl->mtrtype == IPPROTO_UDP) {
        if (check_feature(ctl, cmdpipe, "udp")) {
            return -1;
        }
    } else if (ctl->mtrtype == IPPROTO_TCP) {
        if (check_feature(ctl, cmdpipe, "tcp")) {
            return -1;
        }
#ifdef HAS_SCTP
    } else if (ctl->mtrtype == IPPROTO_SCTP) {
        if (check_feature(ctl, cmdpipe, "sctp")) {
            return -1;
        }
#endif
    } else {
        errno = EINVAL;
        return -1;
    }

#ifdef SO_MARK
    if (ctl->mark) {
        if (check_feature(ctl, cmdpipe, "mark")) {
            return -1;
        }
    }
#endif

    return 0;
}


/*
    Execute mtr-packet, allowing the MTR_PACKET evironment to override
    the PATH when locating the executable.
*/
static
void execute_packet_child(
    void)
{
    /*
       Allow the MTR_PACKET environment variable to override
       the path to the mtr-packet executable.  This is necessary
       for debugging changes for mtr-packet.
     */
    char *mtr_packet_path = getenv("MTR_PACKET");
    if (mtr_packet_path == NULL) {
        mtr_packet_path = "mtr-packet";
    }

    /*
       First, try to execute mtr-packet from PATH
       or MTR_PACKET environment variable.
     */
    execlp(mtr_packet_path, "mtr-packet", (char *) NULL);

    /*
       If mtr-packet is not found, try to use mtr-packet from current directory
     */
    execl("./mtr-packet", "./mtr-packet", (char *) NULL);

    /*  Both exec attempts failed, so nothing to do but exit  */
    exit(1);
}


/*  Create the command pipe to a new mtr-packet subprocess  */
int open_command_pipe(
    struct mtr_ctl *ctl,
    struct packet_command_pipe_t *cmdpipe)
{
    int stdin_pipe[2];
    int stdout_pipe[2];
    pid_t child_pid;
    int i;

    /*
       We actually need two Unix pipes.  One for stdin and one for
       stdout on the new process.
     */
    if (pipe(stdin_pipe) || pipe(stdout_pipe)) {
        return errno;
    }

    child_pid = fork();
    if (child_pid == -1) {
        return errno;
    }

    if (child_pid == 0) {
        /*
           In the child process, attach our created pipes to stdin
           and stdout
         */
        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);

        /*  Close all unnecessary fds  */
        for (i = STDERR_FILENO + 1; i <= stdout_pipe[1]; i++) {
            close(i);
        }

        execute_packet_child();
    } else {
        memset(cmdpipe, 0, sizeof(struct packet_command_pipe_t));

        /*
           In the parent process, save the opposite ends of the pipes
           attached as stdin and stdout in the child.
         */
        cmdpipe->pid = child_pid;
        cmdpipe->read_fd = stdout_pipe[0];
        cmdpipe->write_fd = stdin_pipe[1];

        /*  We don't need the child ends of the pipe open in the parent.  */
        close(stdout_pipe[1]);
        close(stdin_pipe[0]);

        /*
           Check that we can communicate with the client.  If we failed to
           execute the mtr-packet binary, we will discover that here.
         */
        if (check_feature(ctl, cmdpipe, "send-probe")) {
            error(EXIT_FAILURE, errno, "Failure to start mtr-packet");
        }

        if (check_packet_features(ctl, cmdpipe)) {
            error(EXIT_FAILURE, errno, "Packet type unsupported");
        }

        /*  We will need non-blocking reads from the child  */
        set_fd_nonblock(cmdpipe->read_fd);
    }

    return 0;
}


/*  Kill the mtr-packet child process and close the command pipe  */
void close_command_pipe(
    struct packet_command_pipe_t *cmdpipe)
{
    int child_exit_value;

    if (cmdpipe->pid) {
        close(cmdpipe->read_fd);
        close(cmdpipe->write_fd);

        kill(cmdpipe->pid, SIGTERM);
        waitpid(cmdpipe->pid, &child_exit_value, 0);
    }

    memset(cmdpipe, 0, sizeof(struct packet_command_pipe_t));
}


/*  Start building the command string for the "send-probe" command  */
static
void construct_base_command(
    struct mtr_ctl *ctl,
    char *command,
    int buffer_size,
    int command_token,
    ip_t * address,
    ip_t * localaddress)
{
    char ip_string[INET6_ADDRSTRLEN];
    char local_ip_string[INET6_ADDRSTRLEN];
    const char *ip_type;
    const char *local_ip_type;
    const char *protocol = NULL;

    /*  Conver the remote IP address to a string  */
    if (inet_ntop(ctl->af, address, ip_string, INET6_ADDRSTRLEN) == NULL) {

        display_close(ctl);
        error(EXIT_FAILURE, errno, "invalid remote IP address");
    }

    if (inet_ntop(ctl->af, localaddress,
                  local_ip_string, INET6_ADDRSTRLEN) == NULL) {

        display_close(ctl);
        error(EXIT_FAILURE, errno, "invalid local IP address");
    }

    if (ctl->af == AF_INET6) {
        ip_type = "ip-6";
        local_ip_type = "local-ip-6";
    } else {
        ip_type = "ip-4";
        local_ip_type = "local-ip-4";
    }

    if (ctl->mtrtype == IPPROTO_ICMP) {
        protocol = "icmp";
    } else if (ctl->mtrtype == IPPROTO_UDP) {
        protocol = "udp";
    } else if (ctl->mtrtype == IPPROTO_TCP) {
        protocol = "tcp";
#ifdef HAS_SCTP
    } else if (ctl->mtrtype == IPPROTO_SCTP) {
        protocol = "sctp";
#endif
    } else {
        display_close(ctl);
        error(EXIT_FAILURE, 0,
              "protocol unsupported by mtr-packet interface");
    }

    snprintf(command, buffer_size,
             "%d send-probe %s %s %s %s protocol %s",
             command_token,
             ip_type, ip_string, local_ip_type, local_ip_string, protocol);
}


/*  Append an argument to the "send-probe" command string  */
static
void append_command_argument(
    char *command,
    int buffer_size,
    char *name,
    int value)
{
    char argument[COMMAND_BUFFER_SIZE];
    int remaining_size;

    remaining_size = buffer_size - strlen(command) - 1;

    snprintf(argument, buffer_size, " %s %d", name, value);
    strncat(command, argument, remaining_size);
}


/*  Request a new probe from the "mtr-packet" child process  */
void send_probe_command(
    struct mtr_ctl *ctl,
    struct packet_command_pipe_t *cmdpipe,
    ip_t * address,
    ip_t * localaddress,
    int packet_size,
    int sequence,
    int time_to_live)
{
    char command[COMMAND_BUFFER_SIZE];
    int remaining_size;
    int timeout;

    construct_base_command(ctl, command, COMMAND_BUFFER_SIZE, sequence,
                           address, localaddress);

    append_command_argument(command, COMMAND_BUFFER_SIZE, "size",
                            packet_size);

    append_command_argument(command, COMMAND_BUFFER_SIZE, "bit-pattern",
                            ctl->bitpattern);

    append_command_argument(command, COMMAND_BUFFER_SIZE, "tos", ctl->tos);

    append_command_argument(command, COMMAND_BUFFER_SIZE, "ttl",
                            time_to_live);

    timeout = ctl->probe_timeout / 1000000;
    append_command_argument(command, COMMAND_BUFFER_SIZE, "timeout",
                            timeout);

    if (ctl->remoteport) {
        append_command_argument(command, COMMAND_BUFFER_SIZE, "port",
                                ctl->remoteport);
    }

    if (ctl->localport) {
        append_command_argument(command, COMMAND_BUFFER_SIZE, "local-port",
                                ctl->localport);
    }
#ifdef SO_MARK
    if (ctl->mark) {
        append_command_argument(command, COMMAND_BUFFER_SIZE, "mark",
                                ctl->mark);
    }
#endif

    remaining_size = COMMAND_BUFFER_SIZE - strlen(command) - 1;
    strncat(command, "\n", remaining_size);

    /*  Send a probe using the mtr-packet subprocess  */
    if (write(cmdpipe->write_fd, command, strlen(command)) == -1) {
        display_close(ctl);
        error(EXIT_FAILURE, errno,
              "mtr-packet command pipe write failure");
    }
}


/*
    Parse a comma separated field of mpls values, filling out the mplslen
    structure with mpls labels.
*/
static
void parse_mpls_values(
    struct mplslen *mpls,
    char *value_str)
{
    char *next_value = value_str;
    char *end_of_value;
    int value;
    int label_count = 0;
    int label_field = 0;

    while (*next_value) {
        value = strtol(next_value, &end_of_value, 10);

        /*  Failure to advance means an invalid numeric value  */
        if (end_of_value == next_value) {
            return;
        }

        /*  If the next character is not a comma or a NUL, we have
           an invalid string  */
        if (*end_of_value == ',') {
            next_value = end_of_value + 1;
        } else if (*end_of_value == 0) {
            next_value = end_of_value;
        } else {
            return;
        }

        /*
           Store the converted value in the next field of the MPLS
           structure.
         */
        if (label_field == 0) {
            mpls->label[label_count] = value;
        } else if (label_field == 1) {
            mpls->exp[label_count] = value;
        } else if (label_field == 2) {
            mpls->s[label_count] = value;
        } else if (label_field == 3) {
            mpls->ttl[label_count] = value;
        }

        label_field++;
        if (label_field > 3) {
            label_field = 0;
            label_count++;
        }

        /*
           If we've used up all MPLS labels in the structure, return with
           what we've got
         */
        if (label_count >= MAXLABELS) {
            break;
        }
    }

    mpls->labels = label_count;
}


/*
    Extract the IP address and round trip time from a reply to a probe.
    Returns true if both arguments are found in the reply, false otherwise.
*/
static
bool parse_reply_arguments(
    struct mtr_ctl *ctl,
    struct command_t *reply,
    ip_t * fromaddress,
    int *round_trip_time,
    struct mplslen *mpls)
{
    bool found_round_trip;
    bool found_ip;
    char *arg_name;
    char *arg_value;
    int i;

    *round_trip_time = 0;
    memset(fromaddress, 0, sizeof(ip_t));
    memset(mpls, 0, sizeof(struct mplslen));

    found_ip = false;
    found_round_trip = false;

    /*  Examine the reply arguments for known values  */
    for (i = 0; i < reply->argument_count; i++) {
        arg_name = reply->argument_name[i];
        arg_value = reply->argument_value[i];

        if (ctl->af == AF_INET6) {
            /*  IPv6 address of the responding host  */
            if (!strcmp(arg_name, "ip-6")) {
                if (inet_pton(AF_INET6, arg_value, fromaddress)) {
                    found_ip = true;
                }
            }
        } else {
            /*  IPv4 address of the responding host  */
            if (!strcmp(arg_name, "ip-4")) {
                if (inet_pton(AF_INET, arg_value, fromaddress)) {
                    found_ip = true;
                }
            }
        }

        /*  The round trip time in microseconds  */
        if (!strcmp(arg_name, "round-trip-time")) {
            errno = 0;
            *round_trip_time = strtol(arg_value, NULL, 10);
            if (!errno) {
                found_round_trip = true;
            }
        }

        /*  MPLS labels  */
        if (!strcmp(arg_name, "mpls")) {
            parse_mpls_values(mpls, arg_value);
        }
    }

    return found_ip && found_round_trip;
}


/*
    If an mtr-packet command has returned an error result,
    report the error and exit.
*/
static
void handle_reply_errors(
    struct mtr_ctl *ctl,
    struct command_t *reply)
{
    char *reply_name;

    reply_name = reply->command_name;

    if (!strcmp(reply_name, "no-route")) {
        display_close(ctl);
        error(EXIT_FAILURE, 0, "No route to host");
    }

    if (!strcmp(reply_name, "network-down")) {
        display_close(ctl);
        error(EXIT_FAILURE, 0, "Network down");
    }

    if (!strcmp(reply_name, "probes-exhausted")) {
        display_close(ctl);
        error(EXIT_FAILURE, 0, "Probes exhausted");
    }

    if (!strcmp(reply_name, "invalid-argument")) {
        display_close(ctl);
        error(EXIT_FAILURE, 0, "mtr-packet reported invalid argument");
    }

    if (!strcmp(reply_name, "permission-denied")) {
        display_close(ctl);
        error(EXIT_FAILURE, 0, "Permission denied");
    }

    if (!strcmp(reply_name, "address-in-use")) {
        display_close(ctl);
        error(EXIT_FAILURE, 0, "Address in use");
    }

    if (!strcmp(reply_name, "address-not-available")) {
        display_close(ctl);
        error(EXIT_FAILURE, 0, "Address not available");
    }

    if (!strcmp(reply_name, "unexpected-error")) {
        display_close(ctl);
        error(EXIT_FAILURE, 0, "Unexpected mtr-packet error");
    }
}


/*
    A complete mtr-packet reply line has arrived.  Parse it and record
    the responding IP and round trip time, if it is a reply that we
    understand.
*/
static
void handle_command_reply(
    struct mtr_ctl *ctl,
    char *reply_str,
    probe_reply_func_t reply_func)
{
    struct command_t reply;
    ip_t fromaddress;
    int seq_num;
    int round_trip_time;
    char *reply_name;
    struct mplslen mpls;

    /*  Parse the reply string  */
    if (parse_command(&reply, reply_str)) {
        /*
           If the reply isn't well structured, something is fundamentally
           wrong, as we might as well exit.  Even if the reply is of an
           unknown type, it should still parse.
         */
        display_close(ctl);
        error(EXIT_FAILURE, errno, "reply parse failure");
        return;
    }

    handle_reply_errors(ctl, &reply);

    seq_num = reply.token;
    reply_name = reply.command_name;

    /*  If the reply type is unknown, ignore it for future compatibility  */
    if (strcmp(reply_name, "reply") && strcmp(reply_name, "ttl-expired")) {
        return;
    }

    /*
       If the reply had an IP address and a round trip time, we can
       record the result.
     */
    if (parse_reply_arguments
        (ctl, &reply, &fromaddress, &round_trip_time, &mpls)) {

        reply_func(ctl, seq_num, &mpls, (void *) &fromaddress,
                   round_trip_time);
    }
}


/*
    Check the command pipe for completed replies to commands
    we have previously sent.  Record the results of those replies.
*/
static
void consume_reply_buffer(
    struct mtr_ctl *ctl,
    struct packet_command_pipe_t *cmdpipe,
    probe_reply_func_t reply_func)
{
    char *reply_buffer;
    char *reply_start;
    char *end_of_reply;
    int used_size;
    int move_size;

    reply_buffer = cmdpipe->reply_buffer;

    /*  Terminate the string storing the replies  */
    assert(cmdpipe->reply_buffer_used < PACKET_REPLY_BUFFER_SIZE);
    reply_buffer[cmdpipe->reply_buffer_used] = 0;

    reply_start = reply_buffer;

    /*
       We may have multiple completed replies.  Loop until we don't
       have any more newlines termininating replies.
     */
    while (true) {
        /*  If no newline is found, our reply isn't yet complete  */
        end_of_reply = index(reply_start, '\n');
        if (end_of_reply == NULL) {
            /*  No complete replies remaining  */
            break;
        }

        /*
           Terminate the reply string at the newline, which
           is necessary in the case where we are able to read
           mulitple replies arriving simultaneously.
         */
        *end_of_reply = 0;

        /*  Parse and record the reply results  */
        handle_command_reply(ctl, reply_start, reply_func);

        reply_start = end_of_reply + 1;
    }

    /*
       After replies have been processed, free the space used
       by the replies, and move any remaining partial reply text
       to the start of the reply buffer.
     */
    used_size = reply_start - reply_buffer;
    move_size = cmdpipe->reply_buffer_used - used_size;
    memmove(reply_buffer, reply_start, move_size);
    cmdpipe->reply_buffer_used -= used_size;

    if (cmdpipe->reply_buffer_used >= PACKET_REPLY_BUFFER_SIZE - 1) {
        /*
           We've overflowed the reply buffer without a complete reply.
           There's not much we can do about it but discard the data
           we've got and hope new data coming in fits.
         */
        cmdpipe->reply_buffer_used = 0;
    }
}


/*
    Read as much as we can from the reply pipe from the child process, and
    process as many replies as are available.
*/
void handle_command_replies(
    struct mtr_ctl *ctl,
    struct packet_command_pipe_t *cmdpipe,
    probe_reply_func_t reply_func)
{
    int read_count;
    int buffer_remaining;
    char *reply_buffer;
    char *read_buffer;

    reply_buffer = cmdpipe->reply_buffer;

    /*
       Read the available reply text, up to the the remaining
       buffer space.  (Minus one for the terminating NUL.)
     */
    read_buffer = &reply_buffer[cmdpipe->reply_buffer_used];
    buffer_remaining =
        PACKET_REPLY_BUFFER_SIZE - cmdpipe->reply_buffer_used;
    read_count = read(cmdpipe->read_fd, read_buffer, buffer_remaining - 1);

    if (read_count < 0) {
        /*
           EAGAIN simply indicates that there is no data currently
           available on our non-blocking pipe.
         */
        if (errno == EAGAIN) {
            return;
        }

        display_close(ctl);
        error(EXIT_FAILURE, errno, "command reply read failure");
        return;
    }

    if (read_count == 0) {
        display_close(ctl);

        errno = EPIPE;
        error(EXIT_FAILURE, EPIPE, "unexpected packet generator exit");
    }

    cmdpipe->reply_buffer_used += read_count;

    /*  Handle any replies completed by this read  */
    consume_reply_buffer(ctl, cmdpipe, reply_func);
}
