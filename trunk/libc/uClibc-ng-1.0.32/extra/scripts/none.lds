/* GNU ld script
 * Used by the build system to probe flags.
 * Discards all output to avoid error
 * "failed to merge target specific data of file %B". */
TARGET(binary)
INPUT(/dev/null)
/* We could as well omit the whole SECTIONS block.
 * This redundancy is just for clarity.  */
SECTIONS
{
  /DISCARD/ : { *(*) }
}

