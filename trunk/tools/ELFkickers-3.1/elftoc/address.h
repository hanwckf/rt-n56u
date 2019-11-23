/* address.h: Functions for identifying memory addresses.
 * Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#ifndef _address_h_
#define _address_h_

/* Records a memory address found in the file, with its probable
 * offset within the segment, the size of the memory chunk, and the
 * associated name. This information is used to build the collection
 * of memory segments.
 */
extern void recordaddress(long address, long offset, long size,
			  char const *name);

/* Assigns each identified memory segment a unique name. Called after
 * all addresses have been recorded.
 */
extern void setaddressnames(void);

/* Return the name of the segment that contains the given address, If
 * base is not NULL, on return it is assigned the base address of the
 * named segment. If no segment can be matched with the address, NULL
 * is returned and base's value is set to zero.
 */
extern char const *getbaseaddress(long address, long *base);

/* Translate a memory address back into a file offset. The original
 * value is returned if it does not fall within any of the recorded
 * memory segments.
 */
extern long getaddressoffset(long address);

/* Output the list of recorded addresses as a series of macro
 * definitions.
 */
extern void outputaddresses(void);

#endif
