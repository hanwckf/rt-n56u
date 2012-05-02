/* Unlike the asm/unistd.h kernel header file (which this is partly based on),
 * this file must be able to cope with PIC and non-PIC code.  For some arches
 * there is no difference.  For x86 (which has far too few registers) there is
 * a difference.   Regardless, including asm/unistd.h is hereby officially
 * forbidden.  Don't do it.  It is bad for you. 
 */ 


#error You have not provided architecture specific _syscall[0-5] macros

