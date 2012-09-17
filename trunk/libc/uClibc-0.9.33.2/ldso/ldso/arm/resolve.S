/*
 *
 *    add   ip, pc, #0xNN00000
 *    add   ip, ip, #0xNN000
 *    ldr   pc, [ip, #0xNNN]!
 *
 * So that, effectively, causes the following to happen:
 *
 *    ip :   = pc+0x0NNNNNNN
 *    pc :   = *ip
 *
 * For thumb the above fragment is preceded by "bx pc, nop" to switch to ARM
 * mode and the thumb 'bl' must go to PLT-4 - the PLT entry is expanded by
 * four bytes to accomodate the trampoline code.
 *
 * 0x0NNNNNNN is the offset of the GOT entry for this function relative to
 * the PLT entry for this function (where the code is).  So the code in the
 * PLT causes a branch to whatever is in the GOT, leaving the actual address
 * of the GOT entry in ip.  (Note that the GOT must follow the PLT - the
 * added value is 28 bit unsigned).
 *
 * ip is a pointer to the GOT entry for this function, the first time round
 * *ip points to this code:
 *
 *    str   lr, [sp, #-4]!    @ save lr
 *    ldr   lr, [pc, #4]      @ lr : = *dat (&GOT_TABLE[0]-.)
 *    add   lr, pc, lr        @ lr += &dat (so lr == &GOT_TABLE[0])
 *    ldr   pc, [lr, #8]!     @ pc : = GOT_TABLE[2]
 *dat: *.long &GOT_TABLE[0] - .
 *
 * (this code is actually held in the first entry of the PLT).  The code
 * preserves lr then uses it as a scratch register (this preserves the ip
 * value calculated above).  GOT_TABLE[2] is initialized by INIT_GOT in
 * dl-sysdep.h to point to _dl_linux_resolve - this function.  The first
 * three entries in the GOT are reserved, then they are followed by the
 * entries for the PLT entries, in order.
 *
 * The linker initialises the following (non-reserved) GOT entries to
 * the offset of the PLT with an associated relocation so that on load
 * the entry is relocated to point to the PLT - the above code.
 *
 * The net effect of all this is that on the first call to an external (as
 * yet unresolved) function all seven of the above instructions are
 * executed in sequence and the program ends up executing _dl_linux_resolve
 * with the following important values in registers:
 *
 *    ip - a pointer to the GOT entry for the as yet unresolved function
 *    lr - &GOT_TABLE[2]
 *
 * GOT_TABLE[2] has already been initialised to _dl_linux_resolve, and
 * GOT_TABLE[1] is a pointer to the (elf_resolve*) from INIT_GOT.
 * _dl_linux_resolve unfrobnicates the ip and lr values to obtain arguments
 * for a call to _dl_linux_resolver (not the additional 'r' on the end) -
 * this is in elfinterp.c in this directory.  The call takes arguments:
 *
 *    _dl_linux_resolver(struct elf_resolve *tpnt, int reloc_entry)
 *
 * And returns the address of the function, it also overwrites the GOT
 * table entry so that the next time round only the first code fragment will
 * be executed - it will call the function directly.
 *
 * [[Of course, this simply doesn't work on ARM 4T with a thumb target - because
 * 4T did not do the thumb/arm change on ldr pc!  It can be made to work by
 * changing _dl_linux_resolver to return __%s_from_arm for an STT_TFUNC, but
 * this hasn't been done, and there is no guarantee that the linker generated
 * that glue anyway.]]
 *
 * _dl_linux_resolve gets the arguments to call the resolver as follows:
 *
 *    tpnt *GOT_TABLE[1], [lr-4]
 *    reloc-entry     &GOT-&GOT_TABLE[3], (ip - lr - 4)/4
 *
 * (I.e. 'GOT' means the table entry for this function, the thing for which
 * ip holds the address.)  The reloc-entry is passed as an index, since
 * since the GOT table has 4 byte entries the code needs to divide this by 4
 * to get the actual index.
 *
 * John Bowler, August 13, 2005 - determined by experiment and examination
 * of generated ARM code (there was no documentation...)
 *
 * This code is all ARM code - not thumb - _dl_linux_resolver may, itself,
 * be thumb, in which case the linker will insert the appropriate glue.  A
 * call from thumb to the PLT hits the trampoline code described above.
 * This code (now) builds a proper stack frame.
 *
 * The code does *not* set sb (r9,v6) - to do that the basic PLT instructions
 * would need to save sb and load the new value and that would require
 * support in the linker since it generates those instructions.  (Also note
 * that linux/uclibc seems to be using r10 - sl - as a PIC base register - see
 * dl-startup.c).
 */

#include <sys/syscall.h>
#include <bits/arm_asm.h>

#include <features.h>

#define sl r10
#define fp r11
#define ip r12

 .text
 .align 4      @ 16 byte boundary and there are 32 bytes below (arm case)
 #if !defined(__thumb__) || defined(__thumb2__)
 .arm
 .globl _dl_linux_resolve
 .type _dl_linux_resolve,%function
 .align 4;

_dl_linux_resolve:
         @ _dl_linux_resolver is a standard subroutine call, therefore it
         @ preserves everything except r0-r3 (a1-a4), ip and lr.  This
         @ function must branch to the real function, and that expects
         @ r0-r3 and lr to be as they were before the whole PLT stuff -
         @ ip can be trashed.
	 @ This routine is called after pushing lr, so we must push an odd
	 @ number of words to keep the stack correctly aligned.

         stmdb sp!, {r0, r1, r2, r3, r4}
         ldr r0, [lr, #-4]       @ r0 :        = [lr-4] (GOT_TABLE[1])
         sub r1, lr, ip          @ r1 :        = (lr-ip) (a multple of 4)
         mvn r1, r1, ASR #2      @ r1 :        = ~((lr-ip)>>2), since -x = (1+~x)
                                 @ ~x = -x-1, therefore ~(r1>>2) = (-((lr-ip)>>2)-1)
                                 @ = - ((lr-ip)/4) - 1 = (ip - lr - 4)/4, as required

	bl _dl_linux_resolver

	mov ip, r0
        ldmia sp!, {r0, r1, r2, r3, r4, lr}

#if defined(__USE_BX__)
	bx ip
#else
	mov pc,ip
#endif
#else
       @ In the thumb case _dl_linux_resolver is thumb.  If a bl is used
       @ from arm code the linker will insert a stub call which, with
       @ binutils 2.16, is not PIC.  Since this code is accessed by an
       @ ldr pc the reasonable fix is to make _dl_linux_resolve thumb too.
 .thumb
 .globl _dl_linux_resolve
 .thumb_func
 .type _dl_linux_resolve,%function
 _dl_linux_resolve:
       @ _dl_linux_resolver is a standard subroutine call, therefore it
       @ preserves everything except r0-r3 (a1-a4), ip and lr.  This
       @ function must branch to the real function, and that expects
       @ r0-r3 and lr to be as they were before the whole PLT stuff -
       @ ip can be trashed.
       @ This routine is called after pushing lr, so we must push an odd
       @ number of words to keep the stack correctly aligned.
       push    {r0-r4}
       mov     r1, lr          @ &GOT_TABLE[2]
       sub     r0, r1, #4
       mov     r2, ip          @ &GOT[n]
       ldr     r0, [r0]        @ r0 := GOT_TABLE[1]
       @ for the function call r1 := n-3
       sub     r1, r2
       asr     r1, r1, #2
       mvn     r1, r1          @ exactly as in the arm code above
       bl      _dl_linux_resolver
       @ r0 contains the branch address, the return address is above
       @ the saved r0..r3
       mov     ip, r0
       ldr     r1, [sp, #20]
       mov     lr, r1
       pop     {r0-r4}
       add     sp, #4
       bx      ip

#endif
.size _dl_linux_resolve, .-_dl_linux_resolve
