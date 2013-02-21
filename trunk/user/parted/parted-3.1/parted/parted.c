/*
    parted - a frontend to libparted
    Copyright (C) 1999-2003, 2005-2012 Free Software Foundation, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <config.h>
#include <stdbool.h>

#include "argmatch.h"
#include "closeout.h"
#include "configmake.h"
#include "version-etc.h"
#include "command.h"
#include "ui.h"
#include "progname.h"
#include "table.h"
#include "version.h"

#define AUTHORS \
  "<http://git.debian.org/?p=parted/parted.git;a=blob_plain;f=AUTHORS>"

/* The official name of this program (e.g., no `g' prefix).  */
#define PROGRAM_NAME "parted"

#define N_(String) String
#if ENABLE_NLS
#  include <libintl.h>
#  include <locale.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

#include <parted/parted.h>
#include <parted/debug.h>

#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include "c-ctype.h"
#include "c-strcase.h"
#include "xalloc.h"

#ifdef ENABLE_MTRACE
#include <mcheck.h>
#endif

#include <getopt.h>

/* minimum amount of free space to leave, or maximum amount to gobble up */
#define MIN_FREESPACE           (1000 * 2)      /* 1000k */

static int MEGABYTE_SECTORS (PedDevice* dev)
{
        return PED_MEGABYTE_SIZE / dev->sector_size;
}

/* For long options that have no equivalent short option, use a
   non-character as a pseudo short option, starting with CHAR_MAX + 1.  */
enum
{
  PRETEND_INPUT_TTY = CHAR_MAX + 1,
};

enum
{
        ALIGNMENT_NONE = 2,
        ALIGNMENT_CYLINDER,
        ALIGNMENT_MINIMAL,
        ALIGNMENT_OPTIMAL
};

static char const *const align_args[] =
{
  "none",
  "cylinder",
  "minimal",
  "optimal",
  NULL
};

static int const align_types[] =
{
  ALIGNMENT_NONE,
  ALIGNMENT_CYLINDER,
  ALIGNMENT_MINIMAL,
  ALIGNMENT_OPTIMAL
};
ARGMATCH_VERIFY (align_args, align_types);

typedef struct {
        time_t  last_update;
        time_t  predicted_time_left;
} TimerContext;

static struct option const options[] = {
        /* name, has-arg, string-return-val, char-return-val */
        {"help",        0, NULL, 'h'},
        {"list",        0, NULL, 'l'},
        {"machine",     0, NULL, 'm'},
        {"script",      0, NULL, 's'},
        {"version",     0, NULL, 'v'},
        {"align",       required_argument, NULL, 'a'},
        {"-pretend-input-tty", 0, NULL, PRETEND_INPUT_TTY},
        {NULL,          0, NULL, 0}
};

static const char *const options_help [][2] = {
        {"help",        N_("displays this help message")},
        {"list",        N_("lists partition layout on all block devices")},
        {"machine",     N_("displays machine parseable output")},
        {"script",      N_("never prompts for user intervention")},
        {"version",     N_("displays the version")},
        {"align=[none|cyl|min|opt]", N_("alignment for new partitions")},
        {NULL,          NULL}
};

int     opt_script_mode = 0;
int     pretend_input_tty = 0;
int     opt_machine_mode = 0;
int     disk_is_modified = 0;
int     is_toggle_mode = 0;
int     alignment = ALIGNMENT_OPTIMAL;

static const char* number_msg = N_(
"NUMBER is the partition number used by Linux.  On MS-DOS disk labels, the "
"primary partitions number from 1 to 4, logical partitions from 5 onwards.\n");

static const char* label_type_msg_start = N_("LABEL-TYPE is one of: ");
static const char* flag_msg_start =   N_("FLAG is one of: ");
static const char* unit_msg_start =   N_("UNIT is one of: ");
static const char* min_or_opt_msg = N_("desired alignment: minimum or optimal");
static const char* part_type_msg =    N_("PART-TYPE is one of: primary, logical, "
                                   "extended\n");
static const char* fs_type_msg_start = N_("FS-TYPE is one of: ");
static const char* start_end_msg =    N_("START and END are disk locations, such as "
                "4GB or 10%.  Negative values count from the end of the disk.  "
                "For example, -1s specifies exactly the last sector.\n");
static const char* state_msg =        N_("STATE is one of: on, off\n");
static const char* device_msg =       N_("DEVICE is usually /dev/hda or /dev/sda\n");
static const char* name_msg =         N_("NAME is any word you want\n");

static const char* copyright_msg = N_(
"Copyright (C) 1998 - 2006 Free Software Foundation, Inc.\n"
"This program is free software, covered by the GNU General Public License.\n"
"\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"GNU General Public License for more details.\n\n");

static char* label_type_msg;
static char* flag_msg;
static char* unit_msg;

static char* mkpart_fs_type_msg;

static Command* commands [256] = {NULL};
static PedTimer* g_timer;
static TimerContext timer_context;

static int _print_list ();
static void _done (PedDevice* dev);
static bool partition_align_check (PedDisk const *disk,
                        PedPartition const *part, enum AlignmentType a_type);

static void
_timer_handler (PedTimer* timer, void* context)
{
        TimerContext*   tcontext = (TimerContext*) context;
        int             draw_this_time;

        if (opt_script_mode || !isatty(fileno(stdout)))
                return;

        if (tcontext->last_update != timer->now && timer->now > timer->start) {
                tcontext->predicted_time_left
                        = timer->predicted_end - timer->now;
                tcontext->last_update = timer->now;
                draw_this_time = 1;
        } else {
                draw_this_time = 0;
        }

        if (draw_this_time) {
                wipe_line ();

                if (timer->state_name)
                        printf ("%s... ", timer->state_name);
                printf (_("%0.f%%\t(time left %.2d:%.2d)"),
                        (double) (100.0f * timer->frac),
                        (int) (tcontext->predicted_time_left / 60),
                        (int) (tcontext->predicted_time_left % 60));

                fflush (stdout);
        }
}

static int
_partition_warn_busy (PedPartition* part)
{
        char* path;

        if (ped_partition_is_busy (part)) {
                path = ped_partition_get_path (part);
                ped_exception_throw (
                        PED_EXCEPTION_ERROR,
                        PED_EXCEPTION_CANCEL,
                        _("Partition %s is being used. You must unmount it "
                          "before you modify it with Parted."),
                        path);
                free (path);
                return 0;
        }
        return 1;
}

static int
_disk_warn_busy (PedDisk* disk)
{
        if (ped_device_is_busy (disk->dev))
                return ped_exception_throw (
                        (opt_script_mode
                         ? PED_EXCEPTION_ERROR
                         : PED_EXCEPTION_WARNING),
                        PED_EXCEPTION_IGNORE_CANCEL,
                        _("Partition(s) on %s are being used."),
                        disk->dev->path) == PED_EXCEPTION_IGNORE;

        return 1;
}

static int
_disk_warn_loss (PedDisk* disk)
{
        return ped_exception_throw (
                PED_EXCEPTION_WARNING,
                PED_EXCEPTION_YES_NO,
                _("The existing disk label on %s will be destroyed "
                  "and all data on this disk will be lost. Do you "
                  "want to continue?"),
                disk->dev->path) == PED_EXCEPTION_YES;
}

/* This function changes "sector" to "new_sector" if the new value lies
 * within the required range.
 */
static int
snap (PedSector* sector, PedSector new_sector, PedGeometry* range)
{
        PED_ASSERT (ped_geometry_test_sector_inside (range, *sector));
        if (!ped_geometry_test_sector_inside (range, new_sector))
                return 0;
        *sector = new_sector;
        return 1;
}

typedef enum {
        MOVE_NO         = 0,
        MOVE_STILL      = 1,
        MOVE_UP         = 2,
        MOVE_DOWN       = 4
} EMoves;

enum { /* Don't change these values */
        SECT_START      =  0,
        SECT_END        = -1
};

/* Find the preferred way to adjust the sector s inside range.
 * If a move isn't allowed or is out of range it can't be selected.
 * what contains SECT_START if the sector to adjust is a start sector
 * or SECT_END if it's an end one.
 * The preferred move is to the nearest allowed boundary of the part
 * partition (if at equal distance: to start if SECT_START or to end
 * if SECT_END).
 * The distance is returned in dist.
 */
static EMoves
prefer_snap (PedSector s, int what, PedGeometry* range, EMoves* allow,
             PedPartition* part, PedSector* dist)
{
        PedSector up_dist = -1, down_dist = -1;
        PedSector new_sect;
        EMoves move;

        PED_ASSERT (what == SECT_START || what == SECT_END);

        if (!(*allow & (MOVE_UP | MOVE_DOWN))) {
                *dist = 0;
                return MOVE_STILL;
        }

        if (*allow & MOVE_UP) {
                new_sect = part->geom.end + 1 + what;
                if (ped_geometry_test_sector_inside (range, new_sect))
                        up_dist = new_sect - s;
                else
                        *allow &= ~MOVE_UP;
        }

        if (*allow & MOVE_DOWN) {
                new_sect = part->geom.start + what;
                if (ped_geometry_test_sector_inside (range, new_sect))
                        down_dist = s - new_sect;
                else
                        *allow &= ~MOVE_DOWN;
        }

        move = MOVE_STILL;
        if ((*allow & MOVE_UP) && (*allow & MOVE_DOWN)) {
                if (down_dist < up_dist || (down_dist == up_dist
                                            && what == SECT_START) )
                        move = MOVE_DOWN;
                else if (up_dist < down_dist || (down_dist == up_dist
                                                 && what == SECT_END) )
                        move = MOVE_UP;
                else
                        PED_ASSERT (0);
        } else if (*allow & MOVE_UP)
                move = MOVE_UP;
        else if (*allow & MOVE_DOWN)
                move = MOVE_DOWN;

        *dist = ( move == MOVE_DOWN ? down_dist :
                ( move == MOVE_UP   ? up_dist   :
                  0 ) );
        return move;
}

/* Snaps a partition to nearby partition boundaries.  This is useful for
 * gobbling up small amounts of free space, and also for reinterpreting small
 * changes to a partition as non-changes (eg: perhaps the user only wanted to
 * resize the end of a partition).
 *      Note that this isn't the end of the story... this function is
 * always called before the constraint solver kicks in.  So you don't need to
 * worry too much about inadvertantly creating overlapping partitions, etc.
 */
static void
snap_to_boundaries (PedGeometry* new_geom, PedGeometry* old_geom,
                    PedDisk* disk,
                    PedGeometry* start_range, PedGeometry* end_range)
{
        PedPartition*   start_part;
        PedPartition*   end_part;
        PedSector       start = new_geom->start;
        PedSector       end = new_geom->end;
        PedSector       start_dist = -1, end_dist = -1;
        EMoves          start_allow, end_allow, start_want, end_want;
        int             adjacent;

        start_allow = end_allow = MOVE_STILL | MOVE_UP | MOVE_DOWN;

        start_part = ped_disk_get_partition_by_sector (disk, start);
        end_part = ped_disk_get_partition_by_sector (disk, end);
        adjacent = (start_part->geom.end + 1 == end_part->geom.start);

        /* If we can snap to old_geom, then we will... */
        /* and this will enforce the snapped positions  */
        if (old_geom) {
                if (snap (&start, old_geom->start, start_range))
                        start_allow = MOVE_STILL;
                if (snap (&end, old_geom->end, end_range))
                        end_allow = MOVE_STILL;
        }

        /* If start and end are on the same partition, we */
        /* don't allow them to cross. */
        if (start_part == end_part) {
                start_allow &= ~MOVE_UP;
                end_allow &= ~MOVE_DOWN;
        }

        /* Let's find our way */
        start_want = prefer_snap (start, SECT_START, start_range, &start_allow,
                                  start_part, &start_dist );
        end_want = prefer_snap (end, SECT_END, end_range, &end_allow,
                                end_part, &end_dist );

        PED_ASSERT (start_dist >= 0 && end_dist >= 0);

        /* If start and end are on adjacent partitions,    */
        /* and if they would prefer crossing, then refrain */
        /* the farthest to do so. */
        if (adjacent && start_want == MOVE_UP && end_want == MOVE_DOWN) {
                if (end_dist < start_dist) {
                        start_allow &= ~MOVE_UP;
                        start_want = prefer_snap (start, SECT_START,
                                                  start_range, &start_allow,
                                                  start_part, &start_dist );
                        PED_ASSERT (start_dist >= 0);
                } else {
                        end_allow &= ~MOVE_DOWN;
                        end_want = prefer_snap (end, SECT_END,
                                                end_range, &end_allow,
                                                end_part, &end_dist );
                        PED_ASSERT (end_dist >= 0);
                }
        }

        /* New positions */
        start = ( start_want == MOVE_DOWN ? start_part->geom.start :
                ( start_want == MOVE_UP ? start_part->geom.end + 1 :
                  start ) );
        end = ( end_want == MOVE_DOWN ? end_part->geom.start - 1 :
              ( end_want == MOVE_UP ? end_part->geom.end :
                end ) );
        PED_ASSERT (ped_geometry_test_sector_inside(start_range,start));
        PED_ASSERT (ped_geometry_test_sector_inside (end_range, end));
        PED_ASSERT (start <= end);
        ped_geometry_set (new_geom, start, end - start + 1);
}

static PedConstraint*
constraint_from_start_end (PedDevice* dev, PedGeometry* range_start,
                           PedGeometry* range_end)
{
        return ped_constraint_new (ped_alignment_any, ped_alignment_any,
                range_start, range_end, 1, dev->length);
}

void
help_on (char* topic)
{
        Command*        cmd;

        cmd = command_get (commands, topic);
        if (!cmd) return;

        command_print_help (cmd);
}

void
print_commands_help ()
{
        int             i;

        for (i=0; commands [i]; i++)
                command_print_summary (commands [i]);
}

void
print_options_help ()
{
        int             i;

        for (i=0; options_help [i][0]; i++) {
                printf ("  -%c, --%-25.25s %s\n",
                        options_help [i][0][0],
                        options_help [i][0],
                        _(options_help [i][1]));
        }
}

int
do_help (PedDevice** dev)
{
        if (command_line_get_word_count ()) {
                char*   word = command_line_pop_word ();
                if (word) {
                        help_on (word);
                        free (word);
                }
        } else {
                print_commands_help();
        }
        return 1;
}

static int
do_mklabel (PedDevice** dev)
{
        PedDisk*                disk;
        const PedDiskType*      type = NULL;

        ped_exception_fetch_all ();
        disk = ped_disk_new (*dev);
        if (!disk) ped_exception_catch ();
        ped_exception_leave_all ();

        if (!command_line_get_disk_type (_("New disk label type?"), &type))
                goto error;

        if (disk) {
                if (!_disk_warn_busy (disk))
                        goto error_destroy_disk;
                if (!opt_script_mode && !_disk_warn_loss (disk))
                        goto error_destroy_disk;

                ped_disk_destroy (disk);
        }

        disk = ped_disk_new_fresh (*dev, type);
        if (!disk)
                goto error;

        if (!ped_disk_commit (disk))
                goto error_destroy_disk;
        ped_disk_destroy (disk);

        if ((*dev)->type != PED_DEVICE_FILE)
                disk_is_modified = 1;

        return 1;

error_destroy_disk:
        ped_disk_destroy (disk);
error:
        return 0;
}

/* Strip blanks from the end of string STR, in place.  */
void _strip_trailing_spaces(char *str)
{
        if (!str)
                return;
        size_t i = strlen(str);
        while (i && c_isblank(str[--i]))
                str[i]='\0';
}

/* Return true, if str ends with [kMGTPEZY]iB, i.e. IEC units.  */
static bool
_string_ends_with_iec_unit(const char *str)
{
        /* 3 characters for the IEC unit and at least 1 digit */
        if (!str || strlen(str) < 4)
                return false;

        char const *p = str + strlen(str) - 3;
        return strchr ("kMGTPEZY", *p) && c_strcasecmp (p+1, "iB") == 0;
}

/* Return true if str ends with explicit unit identifier.
 * Do not parse the unit, just check if the unit is specified.
 * This function expects trailing spaces to be already stripped.
 */
static bool
_string_has_unit_suffix(const char *str)
{
        /* At least 1 digit and 1 character to meet the condition */
        if (!str || strlen(str) < 2)
                return false;

        if (!isdigit(str[strlen(str) - 1]))
                return true;

        return false;
}

/* If the selected unit is one of kiB, MiB, GiB or TiB and the partition is not
 * only 1 sector long, then adjust the end so that it is one sector before the
 * given position. Also adjust range_end accordingly. Thus next partition can
 * start immediately after this one.
 *
 * To be called after end sector is read from the user.
 *
 * https://lists.gnu.org/archive/html/bug-parted/2011-10/msg00009.html
 */
static void
_adjust_end_if_iec (PedSector* start, PedSector* end,
                    PedGeometry* range_end, char* end_input)
{
        PED_ASSERT(start);
        PED_ASSERT(end);
        PED_ASSERT(range_end);

        /* 1s partition - do not move the end */
        if (*start == *end)
                return;

        _strip_trailing_spaces(end_input);
        PedUnit unit = ped_unit_get_default();
        if (_string_ends_with_iec_unit(end_input) ||
            (!_string_has_unit_suffix(end_input) &&
             ((unit == PED_UNIT_KIBIBYTE) || (unit == PED_UNIT_MEBIBYTE) ||
              (unit == PED_UNIT_GIBIBYTE) || (unit == PED_UNIT_TEBIBYTE)))) {
                *end -= 1;
                range_end->start -= 1;
                range_end->end -= 1;
        }
}

static int
do_mkpart (PedDevice** dev)
{
        PedDisk*                 disk;
        PedPartition*            part;
        PedPartitionType         part_type;
        const PedFileSystemType* fs_type = ped_file_system_type_get ("ext2");
        PedSector                start = 0, end = 0;
        PedGeometry              *range_start = NULL, *range_end = NULL;
        PedConstraint*           user_constraint;
        PedConstraint*           dev_constraint;
        PedConstraint*           final_constraint;
        char*                    peek_word;
        char*                    part_name = NULL;
        char                     *start_usr = NULL, *end_usr = NULL;
        char                     *start_sol = NULL, *end_sol = NULL;

        disk = ped_disk_new (*dev);
        if (!disk)
                goto error;

        if (ped_disk_is_flag_available(disk, PED_DISK_CYLINDER_ALIGNMENT))
                if (!ped_disk_set_flag(disk, PED_DISK_CYLINDER_ALIGNMENT,
                                       alignment == ALIGNMENT_CYLINDER))
                        goto error_destroy_disk;

        if (!ped_disk_type_check_feature (disk->type, PED_DISK_TYPE_EXTENDED)) {
                part_type = PED_PARTITION_NORMAL;
        } else {
                if (!command_line_get_part_type (_("Partition type?"),
                                                disk, &part_type))
                        goto error_destroy_disk;
        }

        /* The undocumented feature that mkpart sometimes takes a
           partition name is next to useless, at least with a dvh
           partition table, since it makes the "mkpart" command
           fail unconditionally for a primary partition.  E.g.,
           mkpart primary any-name xfs 4096s 5000s
           requires the name, yet always fails, saying that only
           logical partitions may have names.
           If you want a name, use parted's separate "name" command.  */

        if (ped_disk_type_check_feature (disk->type,
                                         PED_DISK_TYPE_PARTITION_NAME)
            && ! (strcmp (disk->type->name, "dvh") == 0
                  && part_type != PED_PARTITION_LOGICAL))
                part_name = command_line_get_word (_("Partition name?"),
                                                   "", NULL, 1);

        peek_word = command_line_peek_word ();
        if (part_type == PED_PARTITION_EXTENDED
            || (peek_word && isdigit (peek_word[0]))) {
                fs_type = NULL;
        } else {
                if (!command_line_get_fs_type (_("File system type?"),
                                               &fs_type))
                        goto error_destroy_disk;
        }
        free (peek_word);

        if (!command_line_get_sector (_("Start?"), *dev, &start, &range_start, NULL))
                goto error_destroy_disk;
        char *end_input;
        if (!command_line_get_sector (_("End?"), *dev, &end, &range_end, &end_input))
                goto error_destroy_disk;

        _adjust_end_if_iec(&start, &end, range_end, end_input);
        free(end_input);

        /* processing starts here */
        part = ped_partition_new (disk, part_type, fs_type, start, end);
        if (!part)
                goto error_destroy_disk;

        snap_to_boundaries (&part->geom, NULL, disk, range_start, range_end);

        /* create constraints */
        user_constraint = constraint_from_start_end (*dev, range_start,
                        range_end);
        PED_ASSERT (user_constraint != NULL);

        if (alignment == ALIGNMENT_OPTIMAL)
                dev_constraint =
                        ped_device_get_optimal_aligned_constraint(*dev);
        else if (alignment == ALIGNMENT_MINIMAL)
                dev_constraint =
                        ped_device_get_minimal_aligned_constraint(*dev);
        else
                dev_constraint = ped_device_get_constraint(*dev);
        PED_ASSERT (dev_constraint != NULL);

        final_constraint = ped_constraint_intersect (user_constraint,
                        dev_constraint);
        ped_constraint_destroy (user_constraint);
        ped_constraint_destroy (dev_constraint);
        if (!final_constraint)
                goto error_destroy_simple_constraints;

        /* subject to partition constraint */
        ped_exception_fetch_all();
        bool added_ok = ped_disk_add_partition (disk, part, final_constraint);
        ped_constraint_destroy (final_constraint);
        if (!added_ok) {
                ped_exception_leave_all();

                PedConstraint *constraint_any = ped_constraint_any (*dev);
                added_ok = ped_disk_add_partition (disk, part,
                                                        constraint_any);
                ped_constraint_destroy (constraint_any);

                if (!added_ok)
                        goto error_remove_part;

                if (!ped_geometry_test_sector_inside(range_start, part->geom.start) ||
                    !ped_geometry_test_sector_inside(range_end, part->geom.end)) {
                        start_usr = ped_unit_format (*dev, start);
                        end_usr   = ped_unit_format (*dev, end);
                        start_sol = ped_unit_format (*dev, part->geom.start);
                        end_sol   = ped_unit_format (*dev, part->geom.end);

                        /* In script mode failure to use specified values is fatal.
                         * However, in interactive mode, it merely elicits a warning
                         * and a prompt for whether to proceed.
                         */
                        switch (ped_exception_throw (
                                (opt_script_mode
                                 ? PED_EXCEPTION_ERROR
                                 : PED_EXCEPTION_WARNING),
                                (opt_script_mode
                                 ? PED_EXCEPTION_CANCEL
                                 : PED_EXCEPTION_YES_NO),
                                _("You requested a partition from %s to %s "
                                  "(sectors %llu..%llu).\n"
                                  "The closest location we can manage is "
                                  "%s to %s (sectors %llu..%llu).%s"),
                                start_usr, end_usr,
                                start, end,
                                start_sol, end_sol,
                                part->geom.start, part->geom.end,
                                (opt_script_mode ? ""
                                 : _("\nIs this still acceptable to you?"))))
                        {
                                case PED_EXCEPTION_YES:
                                        /* all is well in this state */
                                        break;
                                case PED_EXCEPTION_NO:
                                case PED_EXCEPTION_UNHANDLED:
                                default:
                                        /* undo partition addition */
                                        goto error_remove_part;
                        }
                }

                if ((alignment == ALIGNMENT_OPTIMAL &&
                     !partition_align_check(disk, part, PA_OPTIMUM)) ||
                    (alignment == ALIGNMENT_MINIMAL &&
                     !partition_align_check(disk, part, PA_MINIMUM))) {
                        if (ped_exception_throw(
                                PED_EXCEPTION_WARNING,
                                (opt_script_mode
                                 ? PED_EXCEPTION_OK
                                 : PED_EXCEPTION_IGNORE_CANCEL),
                                _("The resulting partition is not properly "
                                  "aligned for best performance.")) ==
                            PED_EXCEPTION_CANCEL) {
                                /* undo partition addition */
                                goto error_remove_part;
                        }
                }
        } else {
                ped_exception_leave_all();
        }
        ped_exception_catch();

        /* set minor attributes */
        if (part_name)
                PED_ASSERT (ped_partition_set_name (part, part_name));
        free (part_name);  /* avoid double-free upon failure */
        part_name = NULL;
        if (!ped_partition_set_system (part, fs_type))
                goto error_destroy_disk;
        if (ped_partition_is_flag_available (part, PED_PARTITION_LBA))
                ped_partition_set_flag (part, PED_PARTITION_LBA, 1);

        if (!ped_disk_commit (disk))
                goto error_destroy_disk;

        /* clean up */
        ped_disk_destroy (disk);

        if (range_start != NULL)
                ped_geometry_destroy (range_start);
        if (range_end != NULL)
                ped_geometry_destroy (range_end);

        free (start_usr);
        free (end_usr);
        free (start_sol);
        free (end_sol);

        if ((*dev)->type != PED_DEVICE_FILE)
                disk_is_modified = 1;

        return 1;

error_remove_part:
        ped_disk_remove_partition (disk, part);
error_destroy_simple_constraints:
        ped_partition_destroy (part);
error_destroy_disk:
        free (part_name);
        ped_disk_destroy (disk);
error:
        if (range_start != NULL)
                ped_geometry_destroy (range_start);
        if (range_end != NULL)
                ped_geometry_destroy (range_end);

        free (start_usr);
        free (end_usr);
        free (start_sol);
        free (end_sol);

        return 0;
}

static int
do_name (PedDevice** dev)
{
        PedDisk*        disk;
        PedPartition*   part = NULL;
        char*           name;

        disk = ped_disk_new (*dev);
        if (!disk)
                goto error;

        if (!command_line_get_partition (_("Partition number?"), disk, &part))
                goto error_destroy_disk;

        name = command_line_get_word (_("Partition name?"),
                        ped_partition_get_name (part), NULL, 0);
        if (!name)
                goto error_destroy_disk;
        if (!ped_partition_set_name (part, name))
                goto error_free_name;
        free (name);

        if (!ped_disk_commit (disk))
                goto error_destroy_disk;
        ped_disk_destroy (disk);
        return 1;

error_free_name:
        free (name);
error_destroy_disk:
        ped_disk_destroy (disk);
error:
        return 0;
}

static char*
partition_print_flags (PedPartition const *part)
{
  char *res = xstrdup ("");
  if (!part)
    return res;

  PedPartitionFlag flag;
  size_t res_buf_len = 1;
  char const *sep = "";
  for (flag = ped_partition_flag_next (0); flag;
       flag = ped_partition_flag_next (flag))
    {
      if (ped_partition_get_flag (part, flag))
        {
          const char *name = _(ped_partition_flag_get_name (flag));
          size_t new_len = res_buf_len + strlen (sep) + strlen (name);
          res = xrealloc (res, new_len);
          stpcpy (stpcpy (res + res_buf_len - 1, sep), name);
          res_buf_len = new_len;
          sep = ", ";
        }
    }

  return res;
}

static int
partition_print (PedPartition* part)
{
        return 1;
}

static char*
disk_print_flags (PedDisk const *disk)
{
  char *res = xstrdup ("");
  if (!disk)
    return res;

  PedDiskFlag flag;
  size_t res_buf_len = 1;
  char const *sep = "";
  for (flag = ped_disk_flag_next (0); flag;
       flag = ped_disk_flag_next (flag))
    {
      if (ped_disk_get_flag (disk, flag))
        {
          const char *name = _(ped_disk_flag_get_name (flag));
          size_t new_len = res_buf_len + strlen (sep) + strlen (name);
          res = xrealloc (res, new_len);
          stpcpy (stpcpy (res + res_buf_len - 1, sep), name);
          res_buf_len = new_len;
          sep = ", ";
        }
    }

  return res;
}

static void
_print_disk_geometry (const PedDevice *dev)
{
        PED_ASSERT (dev != NULL);
        const PedCHSGeometry* chs = &dev->bios_geom;
        char* cyl_size = ped_unit_format_custom (dev,
                                chs->heads * chs->sectors,
                                PED_UNIT_KILOBYTE);

        if (opt_machine_mode) {
            printf ("%d:%d:%d:%s;\n",
                    chs->cylinders, chs->heads, chs->sectors, cyl_size);
        } else {
            printf (_("BIOS cylinder,head,sector geometry: %d,%d,%d.  "
                      "Each cylinder is %s.\n"),
                    chs->cylinders, chs->heads, chs->sectors, cyl_size);
        }

        free (cyl_size);
}

static void
_print_disk_info (const PedDevice *dev, const PedDisk *disk)
{
        char const *const transport[] = {"unknown", "scsi", "ide", "dac960",
                                         "cpqarray", "file", "ataraid", "i2o",
                                         "ubd", "dasd", "viodasd", "sx8", "dm",
                                         "xvd", "sd/mmc", "virtblk", "aoe",
                                         "md", "loopback"};

        char* start = ped_unit_format (dev, 0);
        PedUnit default_unit = ped_unit_get_default ();
        char* end = ped_unit_format_byte (dev, dev->length * dev->sector_size
                                    - (default_unit == PED_UNIT_CHS ||
                                       default_unit == PED_UNIT_CYLINDER));

        const char* pt_name = disk ? disk->type->name : "unknown";
        char *disk_flags = disk_print_flags (disk);

        if (opt_machine_mode) {
            switch (default_unit) {
                case PED_UNIT_CHS:      puts ("CHS;");
                                        break;
                case PED_UNIT_CYLINDER: puts ("CYL;");
                                        break;
                default:                puts ("BYT;");
                                        break;

            }
            printf ("%s:%s:%s:%lld:%lld:%s:%s:%s;\n",
                    dev->path, end, transport[dev->type],
                    dev->sector_size, dev->phys_sector_size,
                    pt_name, dev->model, disk_flags);
        } else {
            printf (_("Model: %s (%s)\n"),
                    dev->model, transport[dev->type]);
            printf (_("Disk %s: %s\n"), dev->path, end);
            printf (_("Sector size (logical/physical): %lldB/%lldB\n"),
                    dev->sector_size, dev->phys_sector_size);
        }

        free (start);
        free (end);

        if (ped_unit_get_default () == PED_UNIT_CHS
            || ped_unit_get_default () == PED_UNIT_CYLINDER)
                _print_disk_geometry (dev);

        if (!opt_machine_mode) {
            printf (_("Partition Table: %s\n"), pt_name);
            printf (_("Disk Flags: %s\n"), disk_flags);
        }
        free (disk_flags);
}

static int
do_print (PedDevice** dev)
{
        PedDisk*        disk = NULL;
        Table*          table;
        int             has_extended;
        int             has_name;
        int             has_devices_arg = 0;
        int             has_free_arg = 0;
        int             has_list_arg = 0;
        int             has_num_arg = 0;
        char*           peek_word;
        char*           start;
        char*           end;
        char*           size;
        const char*     name;
        char*           tmp;
        wchar_t*        table_rendered;
        int ok = 1; /* default to success */

        peek_word = command_line_peek_word ();
        if (peek_word) {
                if (strncmp (peek_word, "devices", 7) == 0) {
                        char *w = command_line_pop_word();
                        free (w);
                        has_devices_arg = 1;
                }
                else if (strncmp (peek_word, "free", 4) == 0) {
                        char *w = command_line_pop_word ();
                        free (w);
                        has_free_arg = 1;
                }
                else if (strncmp (peek_word, "list", 4) == 0 ||
                         strncmp (peek_word, "all", 3) == 0) {
                        char *w = command_line_pop_word();
                        free (w);
                        has_list_arg = 1;
                }
                else
                        has_num_arg = isdigit(peek_word[0]);

                free (peek_word);
        }

        if (!has_devices_arg && !has_list_arg) {
                disk = ped_disk_new (*dev);
                /* Returning NULL here is an indication of failure, when in
                   script mode.  Otherwise (interactive mode) it may indicate
                   a real error, but it may also indicate that the user
                   declined when asked to perform some operation.  FIXME:
                   what this really needs is an API change, but a reliable
                   exit code is less important in interactive mode.  */
                if (disk == NULL && opt_script_mode)
                        ok = 0;
        }

        if (disk &&
            ped_disk_is_flag_available(disk, PED_DISK_CYLINDER_ALIGNMENT))
                if (!ped_disk_set_flag(disk, PED_DISK_CYLINDER_ALIGNMENT,
                                       alignment == ALIGNMENT_CYLINDER))
                        goto error_destroy_disk;

        if (has_devices_arg) {
                char*           dev_name;
                PedDevice*      current_dev = NULL;

                ped_device_probe_all();

                while ((current_dev = ped_device_get_next(current_dev))) {
                        end = ped_unit_format_byte (current_dev,
                                             current_dev->length
                                             * current_dev->sector_size);
                        printf ("%s (%s)\n", current_dev->path, end);
                        free (end);
                }

                dev_name = xstrdup ((*dev)->path);
                ped_device_free_all ();

                *dev = ped_device_get (dev_name);
                if (!*dev)
		        return 0;
                if (!ped_device_open (*dev))
                        return 0;

                free (dev_name);

                return 1;
        }

        else if (has_list_arg)
                return _print_list ();

        else if (disk && has_num_arg) {
                PedPartition*   part = NULL;
                int             status = 0;
                if (command_line_get_partition ("", disk, &part))
                        status = partition_print (part);
                ped_disk_destroy (disk);
                return status;
        }

        _print_disk_info (*dev, disk);
        if (!disk)
                goto nopt;
        if (!opt_machine_mode)
                putchar ('\n');

        has_extended = ped_disk_type_check_feature (disk->type,
                                         PED_DISK_TYPE_EXTENDED);
        has_name = ped_disk_type_check_feature (disk->type,
                                         PED_DISK_TYPE_PARTITION_NAME);

        PedPartition* part;
        if (!opt_machine_mode) {
            StrList *row1;

            if (ped_unit_get_default() == PED_UNIT_CHS) {
                    row1 = str_list_create (_("Number"), _("Start"),
                                               _("End"), NULL);
            } else {
                    row1 = str_list_create (_("Number"), _("Start"),
                                               _("End"), _("Size"), NULL);
            }

            if (has_extended)
                    str_list_append (row1, _("Type"));

            str_list_append (row1, _("File system"));

            if (has_name)
                    str_list_append (row1, _("Name"));

            str_list_append (row1, _("Flags"));


            table = table_new (str_list_length(row1));

            table_add_row_from_strlist (table, row1);

            for (part = ped_disk_next_partition (disk, NULL); part;
                 part = ped_disk_next_partition (disk, part)) {

                    if ((!has_free_arg && !ped_partition_is_active(part)) ||
                        part->type & PED_PARTITION_METADATA)
                            continue;

                    tmp = ped_malloc (4);

                    if (part->num >= 0)
                            sprintf (tmp, "%2d ", part->num);
                    else
                            sprintf (tmp, "%2s ", "");

                    StrList *row = str_list_create (tmp, NULL);

                    start = ped_unit_format (*dev, part->geom.start);
                    end = ped_unit_format_byte (
                            *dev,
                            (part->geom.end + 1) * (*dev)->sector_size - 1);
                    size = ped_unit_format (*dev, part->geom.length);
                    if (ped_unit_get_default() == PED_UNIT_CHS) {
                            str_list_append (row, start);
                            str_list_append (row, end);
                    } else {
                            str_list_append (row, start);
                            str_list_append (row, end);
                            str_list_append (row, size);
                    }

                    if (!(part->type & PED_PARTITION_FREESPACE)) {
                            if (has_extended) {
                                name = ped_partition_type_get_name (part->type);
                                str_list_append (row, name);
                            }

                            str_list_append (row, part->fs_type ?
                                             part->fs_type->name : "");

                            if (has_name) {
                                    name = ped_partition_get_name (part);
                                    str_list_append (row, name);
                            }

                            char *flags = partition_print_flags (part);
                            str_list_append (row, flags);
                            free (flags);
                    } else {
                            if (has_extended)
                                    str_list_append (row, "");
                            str_list_append (row, _("Free Space"));
                            if (has_name)
                                    str_list_append (row, "");
                            str_list_append (row, "");
                    }

                    //PED_ASSERT (row.cols == caption.cols)
                    table_add_row_from_strlist (table, row);
                    str_list_destroy (row);
                    free (start);
                    free (end);
                    free (size);
            }

            table_rendered = table_render (table);
#ifdef ENABLE_NLS
            printf("%ls\n", table_rendered);
#else
            printf("%s\n", table_rendered);
#endif
            free (table_rendered);
            table_destroy (table);
            str_list_destroy (row1);

        } else {

            for (part = ped_disk_next_partition (disk, NULL); part;
                 part = ped_disk_next_partition (disk, part)) {

                if ((!has_free_arg && !ped_partition_is_active(part)) ||
                        part->type & PED_PARTITION_METADATA)
                            continue;

                if (part->num >= 0)
                    printf ("%d:", part->num);
                else
                    fputs ("1:", stdout);

                char *s = ped_unit_format (*dev, part->geom.start);
                printf ("%s:", s);
                free (s);
                s = ped_unit_format_byte (*dev,
                                          (part->geom.end + 1) *
                                          (*dev)->sector_size - 1);
                printf ("%s:", s);
                free (s);

                if (ped_unit_get_default() != PED_UNIT_CHS) {
                    s = ped_unit_format (*dev, part->geom.length);
                    printf ("%s:", s);
                    free (s);
                }

                if (!(part->type & PED_PARTITION_FREESPACE)) {

                    if (part->fs_type)
                        printf ("%s:", part->fs_type->name);
                    else
                        putchar (':');

                    if (has_name)
                        printf ("%s:", _(ped_partition_get_name (part)));
                    else
                        putchar (':');

                    char *flags = partition_print_flags (part);
                    printf ("%s;\n", flags);
                    free (flags);

                } else {
                    puts ("free;");
                }
            }
        }

        ped_disk_destroy (disk);

        return ok;

error_destroy_disk:
        ped_disk_destroy (disk);
        return 0;
nopt:
        return ok;
}

static int
_print_list ()
{
        PedDevice *current_dev = NULL;

        ped_device_probe_all();

        while ((current_dev = ped_device_get_next(current_dev))) {
                do_print (&current_dev);
                putchar ('\n');
        }

        return 1;
}

static int
do_quit (PedDevice** dev)
{
        _done (*dev);
        exit (EXIT_SUCCESS);
}

static PedPartitionType
_disk_get_part_type_for_sector (PedDisk* disk, PedSector sector)
{
        PedPartition*   extended;

        extended = ped_disk_extended_partition (disk);
        if (!extended
            || !ped_geometry_test_sector_inside (&extended->geom, sector))
                return 0;

        return PED_PARTITION_LOGICAL;
}

/* This function checks if "part" contains a file system, and returs
 *      0 if either no file system was found, or the user declined to add it.
 *      1 if a file system was found, and the user chose to add it.
 *      -1 if the user chose to cancel the entire search.
 */
static int
_rescue_add_partition (PedPartition* part)
{
        const PedFileSystemType*        fs_type;
        PedGeometry*                    probed;
        PedExceptionOption              ex_opt;
        PedConstraint*                  constraint;
        char*                           found_start;
        char*                           found_end;

        fs_type = ped_file_system_probe (&part->geom);
        if (!fs_type)
                return 0;
        probed = ped_file_system_probe_specific (fs_type, &part->geom);
        if (!probed)
                return 0;

        if (!ped_geometry_test_inside (&part->geom, probed)) {
                ped_geometry_destroy (probed);
                return 0;
        }

        constraint = ped_constraint_exact (probed);
        if (!ped_disk_set_partition_geom (part->disk, part, constraint,
                                          probed->start, probed->end)) {
                ped_constraint_destroy (constraint);
                return 0;
        }
        ped_constraint_destroy (constraint);

        found_start = ped_unit_format (probed->dev, probed->start);
        found_end = ped_unit_format (probed->dev, probed->end);
        ex_opt = ped_exception_throw (
                PED_EXCEPTION_INFORMATION,
                PED_EXCEPTION_YES_NO_CANCEL,
                _("A %s %s partition was found at %s -> %s.  "
                  "Do you want to add it to the partition table?"),
                fs_type->name, ped_partition_type_get_name (part->type),
                found_start, found_end);
        ped_geometry_destroy (probed);
        free (found_start);
        free (found_end);

        switch (ex_opt) {
                case PED_EXCEPTION_CANCEL: return -1;
                case PED_EXCEPTION_NO: return 0;
                default: break;
        }

        ped_partition_set_system (part, fs_type);
        ped_disk_commit (part->disk);
        return 1;
}

/* hack: we only iterate through the start, since most (all) fs's have their
 * superblocks at the start.  We'll need to change this if we generalize
 * for RAID, or something...
 */
static int
_rescue_pass (PedDisk* disk, PedGeometry* start_range, PedGeometry* end_range)
{
        PedSector               start;
        PedGeometry             start_geom_exact;
        PedGeometry             entire_dev;
        PedConstraint           constraint;
        PedPartition*           part;
        PedPartitionType        part_type;

        part_type = _disk_get_part_type_for_sector (
                        disk, (start_range->start + end_range->end) / 2);

        ped_geometry_init (&entire_dev, disk->dev, 0, disk->dev->length);

        ped_timer_reset (g_timer);
        ped_timer_set_state_name (g_timer, _("searching for file systems"));
        for (start = start_range->start; start <= start_range->end; start++) {
                ped_timer_update (g_timer, 1.0 * (start - start_range->start)
                                         / start_range->length);

                ped_geometry_init (&start_geom_exact, disk->dev, start, 1);
                ped_constraint_init (
                        &constraint, ped_alignment_any, ped_alignment_any,
                        &start_geom_exact, &entire_dev,
                        1, disk->dev->length);
                part = ped_partition_new (disk, part_type, NULL, start,
                                end_range->end);
                if (!part) {
                        ped_constraint_done (&constraint);
                        continue;
                }

                ped_exception_fetch_all ();
                if (ped_disk_add_partition (disk, part, &constraint)) {
                        ped_exception_leave_all ();
                        switch (_rescue_add_partition (part)) {
                        case 1:
                                ped_constraint_done (&constraint);
                                return 1;

                        case 0:
                                ped_disk_remove_partition (disk, part);
                                break;

                        case -1:
                                goto error_remove_partition;
                        }
                } else {
                        ped_exception_leave_all ();
                }
                ped_partition_destroy (part);
                ped_constraint_done (&constraint);
        }
        ped_timer_update (g_timer, 1.0);

        return 1;

error_remove_partition:
        ped_disk_remove_partition (disk, part);
        ped_partition_destroy (part);
        ped_constraint_done (&constraint);
        return 0;
}

static int
do_rescue (PedDevice** dev)
{
        PedDisk*                disk;
        PedSector               start = 0, end = 0;
        PedSector               fuzz;
        PedGeometry             probe_start_region;
        PedGeometry             probe_end_region;

        disk = ped_disk_new (*dev);
        if (!disk)
                goto error;

        if (!command_line_get_sector (_("Start?"), *dev, &start, NULL, NULL))
                goto error_destroy_disk;
        if (!command_line_get_sector (_("End?"), *dev, &end, NULL, NULL))
                goto error_destroy_disk;

        fuzz = PED_MAX (PED_MIN ((end - start) / 10, MEGABYTE_SECTORS(*dev)),
                        MEGABYTE_SECTORS(*dev) * 16);

        ped_geometry_init (&probe_start_region, *dev,
                           PED_MAX(start - fuzz, 0),
                           PED_MIN(2 * fuzz, (*dev)->length - (start - fuzz)));
        ped_geometry_init (&probe_end_region, *dev,
                           PED_MAX(end - fuzz, 0),
                           PED_MIN(2 * fuzz, (*dev)->length - (end - fuzz)));

        if (!_rescue_pass (disk, &probe_start_region, &probe_end_region))
                goto error_destroy_disk;

        ped_disk_destroy (disk);

        if ((*dev)->type != PED_DEVICE_FILE)
                disk_is_modified = 1;

        return 1;

error_destroy_disk:
        ped_disk_destroy (disk);
error:
        return 0;
}

static int
do_rm (PedDevice** dev)
{
        PedDisk*                disk;
        PedPartition*           part = NULL;

        disk = ped_disk_new (*dev);
        if (!disk)
                goto error;

        if (!command_line_get_partition (_("Partition number?"), disk, &part))
                goto error_destroy_disk;
        if (!_partition_warn_busy (part))
                goto error_destroy_disk;

        ped_disk_delete_partition (disk, part);
        ped_disk_commit (disk);
        ped_disk_destroy (disk);

        if ((*dev)->type != PED_DEVICE_FILE)
                disk_is_modified = 1;

        return 1;

error_destroy_disk:
        ped_disk_destroy (disk);
error:
        return 0;
}

static int
do_select (PedDevice** dev)
{
        PedDevice*      new_dev = *dev;

        if (!command_line_get_device (_("New device?"), &new_dev))
                return 0;
        if (!ped_device_open (new_dev))
                return 0;

        ped_device_close (*dev);
        *dev = new_dev;
        print_using_dev (*dev);
        return 1;
}

/* Return true if partition PART is consistent with DISK's selected
   offset and alignment requirements.  Also return true if there is
   insufficient kernel support to determine DISK's alignment requirements.
   Otherwise, return false.  A_TYPE selects whether to check for minimal
   or optimal alignment requirements.  */
static bool
partition_align_check (PedDisk const *disk, PedPartition const *part,
		       enum AlignmentType a_type)
{
  PED_ASSERT (part->disk == disk);
  PedDevice const *dev = disk->dev;

  PedAlignment *pa = (a_type == PA_MINIMUM
		      ? ped_device_get_minimum_alignment (dev)
		      : ped_device_get_optimum_alignment (dev));
  if (pa == NULL)
    return true;

  PED_ASSERT (pa->grain_size != 0);
  bool ok = (part->geom.start % pa->grain_size == pa->offset);
  free (pa);
  return ok;
}

static int
do_align_check (PedDevice **dev)
{
  PedDisk *disk = ped_disk_new (*dev);
  if (!disk)
    goto error;

  enum AlignmentType align_type = PA_OPTIMUM;
  PedPartition *part = NULL;

  if (!command_line_get_align_type (_("alignment type(min/opt)"), &align_type))
    goto error_destroy_disk;
  if (!command_line_get_partition (_("Partition number?"), disk, &part))
    goto error_destroy_disk;

  bool aligned = partition_align_check (disk, part, align_type);
  if (!opt_script_mode)
    printf(aligned ? _("%d aligned\n") : _("%d not aligned\n"), part->num);

  ped_disk_destroy (disk);

  if (opt_script_mode)
    return aligned ? 1 : 0;

  /* Always return 1 in interactive mode, to be consistent
     with the other modes.  */
  return 1;

error_destroy_disk:
  ped_disk_destroy (disk);
error:
  return 0;
}

static int
do_disk_set (PedDevice** dev)
{
    PedDisk*    disk;
    PedDiskFlag flag;
    int         state;

    disk = ped_disk_new (*dev);
    if (!disk)
        goto error;

    if (!command_line_get_disk_flag (_("Flag to Invert?"), disk, &flag))
        goto error_destroy_disk;
    state = (ped_disk_get_flag (disk, flag) == 0 ? 1 : 0);

    if (!is_toggle_mode) {
        if (!command_line_get_state (_("New state?"), &state))
            goto error_destroy_disk;
    }

    if (!ped_disk_set_flag (disk, flag, state))
        goto error_destroy_disk;
    if (!ped_disk_commit (disk))
        goto error_destroy_disk;
    ped_disk_destroy (disk);

    if ((*dev)->type != PED_DEVICE_FILE)
        disk_is_modified = 1;

    return 1;

error_destroy_disk:
    ped_disk_destroy (disk);
error:
    return 0;
}

static int
do_set (PedDevice** dev)
{
        PedDisk*                disk;
        PedPartition*           part = NULL;
        PedPartitionFlag        flag;
        int                     state;

        disk = ped_disk_new (*dev);
        if (!disk)
                goto error;

        if (!command_line_get_partition (_("Partition number?"), disk, &part))
                goto error_destroy_disk;
        if (!command_line_get_part_flag (_("Flag to Invert?"), part, &flag))
                goto error_destroy_disk;
        state = (ped_partition_get_flag (part, flag) == 0 ? 1 : 0);

        if (!is_toggle_mode) {
                if (!command_line_get_state (_("New state?"), &state))
		            goto error_destroy_disk;
        }

        if (!ped_partition_set_flag (part, flag, state))
	        	goto error_destroy_disk;
    	if (!ped_disk_commit (disk))
	        	goto error_destroy_disk;
    	ped_disk_destroy (disk);

        if ((*dev)->type != PED_DEVICE_FILE)
                disk_is_modified = 1;

	    return 1;

error_destroy_disk:
        ped_disk_destroy (disk);
error:
        return 0;
}

static int
do_disk_toggle (PedDevice **dev)
{
    int result;

    is_toggle_mode = 1;
    result = do_disk_set (dev);
    is_toggle_mode = 0;

    return result;
}

static int
do_toggle (PedDevice **dev)
{
        int result;

        is_toggle_mode = 1;
        result = do_set (dev);
        is_toggle_mode = 0;

        return result;
}

static int
do_unit (PedDevice** dev)
{
        PedUnit unit = ped_unit_get_default ();
        if (!command_line_get_unit (_("Unit?"), &unit))
                return 0;
        ped_unit_set_default (unit);
        return 1;
}

static int
do_version ()
{
    printf ("\n%s\n%s",
            prog_name,
            _(copyright_msg));
    return 1;
}

static void
_init_messages ()
{
        StrList*                list;
        int                     first;
        PedFileSystemType*      fs_type;
        PedFileSystemAlias*     fs_alias;
        PedDiskType*            disk_type;
        PedPartitionFlag        part_flag;
        PedUnit                 unit;

/* flags */
        first = 1;
        list = str_list_create (_(flag_msg_start), NULL);
        for (part_flag = ped_partition_flag_next (0); part_flag;
                        part_flag = ped_partition_flag_next (part_flag)) {
                if (first)
                        first = 0;
                else
                        str_list_append (list, ", ");
                str_list_append (list,
                                 _(ped_partition_flag_get_name (part_flag)));
        }
        str_list_append (list, "\n");

        flag_msg = str_list_convert (list);
        str_list_destroy (list);

/* units */
        first = 1;
        list = str_list_create (_(unit_msg_start), NULL);
        for (unit = PED_UNIT_FIRST; unit <= PED_UNIT_LAST; unit++) {
                if (first)
                        first = 0;
                else
                        str_list_append (list, ", ");
                str_list_append (list, ped_unit_get_name (unit));
        }
        str_list_append (list, "\n");

        unit_msg = str_list_convert (list);
        str_list_destroy (list);

/* disk type */
        list = str_list_create (_(label_type_msg_start), NULL);

        first = 1;
        for (disk_type = ped_disk_type_get_next (NULL);
             disk_type; disk_type = ped_disk_type_get_next (disk_type)) {
                if (disk_type->ops->write == NULL)
                        continue;

                if (first)
                        first = 0;
                else
                        str_list_append (list, ", ");
                str_list_append (list, disk_type->name);
        }
        str_list_append (list, "\n");

        label_type_msg = str_list_convert (list);
        str_list_destroy (list);

/* mkpart - file system types and aliases */
        list = str_list_create (_(fs_type_msg_start), NULL);

        first = 1;
        for (fs_type = ped_file_system_type_get_next (NULL);
             fs_type; fs_type = ped_file_system_type_get_next (fs_type)) {
                if (first)
                        first = 0;
                else
                        str_list_append (list, ", ");
                str_list_append (list, fs_type->name);
        }
        for (fs_alias = ped_file_system_alias_get_next (NULL);
             fs_alias; fs_alias = ped_file_system_alias_get_next (fs_alias)) {
                if (first)
                        first = 0;
                else
                        str_list_append (list, ", ");
                str_list_append (list, fs_alias->alias);
        }
        str_list_append (list, "\n");

        mkpart_fs_type_msg = str_list_convert (list);
        str_list_destroy (list);
}

static void
_done_messages ()
{
        free (flag_msg);
        free (unit_msg);
        free (mkpart_fs_type_msg);
        free (label_type_msg);
}

static void
_init_commands ()
{
  command_register (commands,
    command_create ( str_list_create_unique ("align-check",
					     _("align-check"), NULL),
		     do_align_check,
		     str_list_create (
				      _("align-check TYPE N"
					"                        "
					"check partition N for"
					" TYPE(min|opt) alignment"), NULL),

		     str_list_create (_(number_msg), _(min_or_opt_msg),
				      NULL), 1));
        command_register (commands, command_create (
                str_list_create_unique ("help", _("help"), NULL),
                do_help,
                str_list_create (
_("help [COMMAND]                           print general help, or help "
  "on COMMAND"),
NULL),
                NULL, 1));

        command_register (commands, command_create (
                str_list_create_unique ("mklabel", _("mklabel"), "mktable", _("mktable"), NULL),
                do_mklabel,
                str_list_create (
_("mklabel,mktable LABEL-TYPE               create a new disklabel "
  "(partition table)"),
NULL),
                str_list_create (label_type_msg, NULL), 1));

        command_register (commands, command_create (
                str_list_create_unique ("mkpart", _("mkpart"), NULL),
                do_mkpart,
                str_list_create (
_("mkpart PART-TYPE [FS-TYPE] START END     make a partition"),
NULL),
                str_list_create (_(part_type_msg),
                                 _(mkpart_fs_type_msg),
                                 _(start_end_msg),
                                 "\n",
_("'mkpart' makes a partition without creating a new file system on the "
  "partition.  FS-TYPE may be specified to set an appropriate partition ID.\n"),
NULL), 1));

command_register (commands, command_create (
        str_list_create_unique ("name", _("name"), NULL),
        do_name,
        str_list_create (
_("name NUMBER NAME                         name partition NUMBER as NAME"),
NULL),
        str_list_create (_(number_msg), _(name_msg), NULL), 1));

command_register (commands, command_create (
        str_list_create_unique ("print", _("print"), NULL),
        do_print,
        str_list_create (
_("print [devices|free|list,all|NUMBER]     display the partition table, "
  "available devices, free space, all found partitions, or a particular "
  "partition"),
NULL),
        str_list_create (
_("Without arguments, 'print' displays the entire partition table. However "
  "with the following arguments it performs various other actions.\n"),
_("  devices   : display all active block devices\n"),
_("  free      : display information about free unpartitioned space on the "
  "current block device\n"),
_("  list, all : display the partition tables of all active block devices\n"),
_("  NUMBER    : display more detailed information about this particular "
  "partition\n"),
NULL), 1));

command_register (commands, command_create (
        str_list_create_unique ("quit", _("quit"), NULL),
        do_quit,
        str_list_create (
_("quit                                     exit program"),
NULL),
        NULL, 1));

command_register (commands, command_create (
        str_list_create_unique ("rescue", _("rescue"), NULL),
        do_rescue,
        str_list_create (
_("rescue START END                         rescue a lost partition near "
"START and END"),
NULL),
        str_list_create (_(start_end_msg), NULL), 1));

command_register (commands, command_create (
        str_list_create_unique ("rm", _("rm"), NULL),
        do_rm,
        str_list_create (
_("rm NUMBER                                delete partition NUMBER"),
NULL),
        str_list_create (_(number_msg), NULL), 1));

command_register (commands, command_create (
        str_list_create_unique ("select", _("select"), NULL),
        do_select,
        str_list_create (
_("select DEVICE                            choose the device to edit"),
NULL),
        str_list_create (_(device_msg), NULL), 1));

command_register (commands, command_create (
        str_list_create_unique ("disk_set", _("disk_set"), NULL),
        do_disk_set,
        str_list_create (
_("disk_set FLAG STATE                      change the FLAG on selected device"),
NULL),
        str_list_create (flag_msg, _(state_msg), NULL), 1));

command_register (commands, command_create (
        str_list_create_unique ("disk_toggle", _("disk_toggle"), NULL),
        do_disk_toggle,
        str_list_create (
_("disk_toggle [FLAG]                       toggle the state of FLAG on "
"selected device"),
NULL),
        str_list_create (flag_msg, NULL), 1));

command_register (commands, command_create (
		str_list_create_unique ("set", _("set"), NULL),
		do_set,
		str_list_create (
_("set NUMBER FLAG STATE                    change the FLAG on partition "
  "NUMBER"),
NULL),
        str_list_create (_(number_msg), flag_msg, _(state_msg), NULL), 1));

command_register (commands, command_create (
        str_list_create_unique ("toggle", _("toggle"), NULL),
        do_toggle,
        str_list_create (
_("toggle [NUMBER [FLAG]]                   toggle the state of FLAG on "
  "partition NUMBER"),
NULL),
        str_list_create (_(number_msg), flag_msg, NULL), 1));

command_register (commands, command_create (
        str_list_create_unique ("unit", _("unit"), NULL),
        do_unit,
        str_list_create (
_("unit UNIT                                set the default unit to UNIT"),
NULL),
        str_list_create (unit_msg, NULL), 1));

command_register (commands, command_create (
        str_list_create_unique ("version", _("version"), NULL),
        do_version,
        str_list_create (
_("version                                  display the version number "
"and copyright information of GNU Parted"),
NULL),
        str_list_create (
_("'version' displays copyright and version information corresponding to this "
"copy of GNU Parted\n"),
NULL), 1));

}

static void
_done_commands ()
{
Command**       walk;

for (walk = commands; *walk; walk++) {
        command_destroy (*walk);
        *walk = NULL;
}
}

static void
_init_i18n ()
{
/* intialize i18n */
#ifdef ENABLE_NLS
setlocale(LC_ALL, "");
bindtextdomain(PACKAGE, LOCALEDIR);
textdomain(PACKAGE);
#endif /* ENABLE_NLS */
}

void
_version ()
{
  version_etc (stdout, PROGRAM_NAME, PACKAGE_NAME, Version, AUTHORS,
               (char *) NULL);
}

static int
_parse_options (int* argc_ptr, char*** argv_ptr)
{
int     opt, help = 0, list = 0, version = 0, wrong = 0;

while (1)
{
        opt = getopt_long (*argc_ptr, *argv_ptr, "hlmsva:",
                           options, NULL);
        if (opt == -1)
                break;

        switch (opt) {
                case 'h': help = 1; break;
                case 'l': list = 1; break;
                case 'm': opt_machine_mode = 1; break;
                case 's': opt_script_mode = 1; break;
                case 'v': version = 1; break;
                case 'a':
                  alignment = XARGMATCH ("--align", optarg,
                                         align_args, align_types);
                  break;
                case PRETEND_INPUT_TTY:
                  pretend_input_tty = 1;
                  break;
                default:
                  wrong = 1;
                  break;
        }
}

if (wrong == 1) {
        fprintf (stderr,
                 _("Usage: %s [-hlmsv] [-a<align>] [DEVICE [COMMAND [PARAMETERS]]...]\n"),
                 program_name);
        return 0;
}

if (version == 1) {
        _version ();
        exit (EXIT_SUCCESS);
}

if (help == 1) {
        help_msg ();
        exit (EXIT_SUCCESS);
}

if (list == 1) {
        _print_list ();
        exit (EXIT_SUCCESS);
}

*argc_ptr -= optind;
*argv_ptr += optind;
return 1;
}

static PedDevice*
_choose_device (int* argc_ptr, char*** argv_ptr)
{
PedDevice*      dev;

/* specified on comand line? */
if (*argc_ptr) {
        dev = ped_device_get ((*argv_ptr) [0]);
        if (!dev)
                return NULL;
        (*argc_ptr)--;
        (*argv_ptr)++;
} else {
retry:
        ped_device_probe_all ();
        dev = ped_device_get_next (NULL);
        if (!dev) {
                if (ped_exception_throw (PED_EXCEPTION_ERROR,
                        PED_EXCEPTION_RETRY_CANCEL,
                        _("No device found"))
                                == PED_EXCEPTION_RETRY)
                        goto retry;
                else
                        return NULL;
        }
}

if (!ped_device_open (dev))
        return NULL;
return dev;
}

static PedDevice*
_init (int* argc_ptr, char*** argv_ptr)
{
PedDevice*      dev;

#ifdef ENABLE_MTRACE
mtrace();
#endif

_init_i18n ();
if (!init_ui ())
        goto error;
_init_messages ();
_init_commands ();

if (!_parse_options (argc_ptr, argv_ptr))
        goto error_done_commands;

if (!opt_script_mode)
        if (init_readline ())
                goto error_done_commands;

#ifdef HAVE_GETUID
        if (getuid() != 0 && !opt_script_mode) {
            fputs (_("WARNING: You are not superuser.  Watch out for "
                     "permissions.\n"), stderr);
        }
#endif

dev = _choose_device (argc_ptr, argv_ptr);
if (!dev)
        goto error_done_commands;

g_timer = ped_timer_new (_timer_handler, &timer_context);
if (!g_timer)
        goto error_done_commands;
timer_context.last_update = 0;

return dev;

error_done_commands:
_done_commands ();
_done_messages ();
done_ui ();
error:
return NULL;
}

static void
_done (PedDevice* dev)
{
if (dev->boot_dirty && dev->type != PED_DEVICE_FILE) {
        ped_exception_throw (
                PED_EXCEPTION_WARNING,
                PED_EXCEPTION_OK,
        _("You should reinstall your boot loader before "
          "rebooting.  Read section 4 of the Parted User "
          "documentation for more information."));
}
if (!opt_script_mode && !opt_machine_mode && disk_is_modified) {
        ped_exception_throw (
                PED_EXCEPTION_INFORMATION, PED_EXCEPTION_OK,
                _("You may need to update /etc/fstab.\n"));
}

ped_device_close (dev);

ped_timer_destroy (g_timer);
_done_commands ();
_done_messages ();
done_ui();
}

int
main (int argc, char** argv)
{
        PedDevice*      dev;
        int             status;

        set_program_name (argv[0]);
        atexit (close_stdout);

        dev = _init (&argc, &argv);
        if (!dev)
                return 1;

        if (argc || opt_script_mode)
                status = non_interactive_mode (&dev, commands, argc, argv);
        else
                status = interactive_mode (&dev, commands);

        _done (dev);

        return !status;
}
