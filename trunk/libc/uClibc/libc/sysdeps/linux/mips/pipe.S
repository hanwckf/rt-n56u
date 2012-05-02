/* pipe system call for Linux/MIPS */

/*see uClibc's sh/pipe.c and glibc-2.2.4's mips/pipe.S */

#include <features.h>
#include <asm/asm.h>
#include <asm/unistd.h>
#include <asm/regdef.h>

        .globl  pipe
        .ent    pipe, 0
pipe:
        .frame  sp, 24, sp
#ifdef __PIC__
        .set    noreorder
        .cpload $25
        .set    reorder
        addiu   sp,sp,-24
        .cprestore     16
#else
        addiu   sp,sp,-24
#endif
        sw      a0,16(sp)
        li      v0,__NR_pipe
        syscall
        beqz    a3, 1f
        addiu   sp,sp,24
        move    a0,v0
#ifdef __PIC__
        la      t9, __syscall_error
        jr      t9
#else
        j       __syscall_error
#endif
1:
        lw      a0, 16(sp)
        sw      v0, 0(a0)
        sw      v1, 4(a0)
        li      v0, 0
2:
        addiu   sp,sp,24
        j       ra
        .end    pipe
        .size   pipe,.-pipe
