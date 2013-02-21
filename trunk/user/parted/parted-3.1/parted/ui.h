/*
    parted - a frontend to libparted
    Copyright (C) 1999-2001, 2007-2012 Free Software Foundation, Inc.

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

#ifndef UI_H_INCLUDED
#define UI_H_INCLUDED

#include "strlist.h"

enum AlignmentType
  {
    PA_MINIMUM = 1,
    PA_OPTIMUM
  };

extern const char *prog_name;

extern int init_ui ();
extern int init_readline ();
extern int non_interactive_mode (PedDevice** dev, Command* cmd_list[],
				 int argc, char* argv[]);
extern int interactive_mode (PedDevice** dev, Command* cmd_list[]);
extern void done_ui ();

extern int screen_width ();
extern void wipe_line ();

extern void command_line_push_word (const char* word);
extern char* command_line_pop_word ();
extern char* command_line_peek_word ();
extern void command_line_flush ();
extern int command_line_get_word_count () _GL_ATTRIBUTE_PURE;
extern void command_line_prompt_words (const char* prompt, const char* def,
				       const StrList* possibilities,
				       int multi_word);
extern char* command_line_get_word (const char* prompt, const char* def,
				    const StrList* possibilities,
				    int multi_word);
extern int command_line_get_integer (const char* prompt, int* value);
extern int command_line_get_sector (const char* prompt, PedDevice* dev,
				    PedSector* value, PedGeometry** range, char** raw_input);
extern int command_line_get_state (const char* prompt, int* value);
extern int command_line_get_device (const char* prompt, PedDevice** value);
extern int command_line_get_disk (const char* prompt, PedDisk** value)
  __attribute__((__nonnull__(2)));
extern int command_line_get_partition (const char* prompt, PedDisk* disk,
				       PedPartition** value);
extern int command_line_get_fs_type (const char* prompt,
				     const PedFileSystemType*(* value));
extern int command_line_get_disk_type (const char* prompt,
				       const PedDiskType*(* value));
extern int command_line_get_disk_flag (const char* prompt,
				       const PedDisk* disk,
				       PedDiskFlag* flag);
extern int command_line_get_part_flag (const char* prompt,
				       const PedPartition* part,
				       PedPartitionFlag* flag);
extern int command_line_get_part_type (const char* prompt, const PedDisk* disk,
				       PedPartitionType* type);
extern PedExceptionOption command_line_get_ex_opt (const char* prompt,
						   PedExceptionOption options);
extern int command_line_get_unit (const char* prompt, PedUnit* unit);
extern int command_line_get_align_type (const char *prompt,
					enum AlignmentType *align_type);

extern int command_line_is_integer ();
extern int command_line_is_sector ();

extern void help_msg () __attribute__((__noreturn__));

extern void print_using_dev (PedDevice* dev);

/* in parted.c */
extern int	opt_script_mode;
extern int	pretend_input_tty;

extern void print_options_help ();
extern void print_commands_help ();

#endif /* UI_H_INCLUDED */
