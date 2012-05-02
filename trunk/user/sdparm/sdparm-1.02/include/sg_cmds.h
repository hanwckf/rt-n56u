#ifndef SG_CMDS_H
#define SG_CMDS_H

/********************************************************************
 * This header did contain wrapper declarations for many SCSI commands
 * up until sg3_utils version 1.22 . In that version, the command
 * wrappers were broken into two groups, the 'basic' ones found in the
 * "sg_cmds_basic.h" header and the 'extra' ones found in the
 * "sg_cmds_extra.h" header. This header now simply includes those two
 * headers.
 * The wrappers function definitions are found in the "sg_cmds_basic.c"
 * and "sg_cmds_extra.c" files.
 ********************************************************************/

#include "sg_cmds_basic.h"
#include "sg_cmds_extra.h"

#endif
