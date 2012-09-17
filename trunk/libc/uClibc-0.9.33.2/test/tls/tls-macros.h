/* Macros to support TLS testing in times of missing compiler support.  */

#define COMMON_INT_DEF(x) \
  __asm__ (".tls_common " #x ",4,4")
/* XXX Until we get compiler support we don't need declarations.  */
#define COMMON_INT_DECL(x)

/* XXX This definition will probably be machine specific, too.  */
#define VAR_INT_DEF(x) \
  __asm__ (".section .tdata\n\t"						      \
       ".globl " #x "\n"						      \
       ".balign 4\n"							      \
       #x ":\t.long 0\n\t"						      \
       ".size " #x ",4\n\t"						      \
       ".previous")
/* XXX Until we get compiler support we don't need declarations.  */
#define VAR_INT_DECL(x)

#ifdef __mips__
#include <tls-macros-mips.h>
#endif

#ifdef __arm__
#ifdef __thumb__
#include <tls-macros-thumb.h>
#else
#include <tls-macros-arm.h>
#endif
#endif

  /* XXX Each architecture must have its own asm for now.  */
#ifdef __i386__
# define TLS_LE(x) \
  ({ int *__l;								      \
     __asm__ ("movl %%gs:0,%0\n\t"						      \
	  "subl $" #x "@tpoff,%0"					      \
	  : "=r" (__l));						      \
     __l; })

# ifdef __PIC__
#  define TLS_IE(x) \
  ({ int *__l;								      \
     __asm__ ("movl %%gs:0,%0\n\t"						      \
	  "subl " #x "@gottpoff(%%ebx),%0"				      \
	  : "=r" (__l));						      \
     __l; })
# else
#  define TLS_IE(x) \
  ({ int *__l;                                                               \
     __asm__ ("call 1f\n\t"							      \
	  ".subsection 1\n"						      \
	  "1:\tmovl (%%esp), %%ebx\n\t"					      \
	  "ret\n\t"							      \
	  ".previous\n\t"						      \
	  "addl $_GLOBAL_OFFSET_TABLE_, %%ebx\n\t"			      \
	  "movl %%gs:0,%0\n\t"						      \
	  "subl " #x "@gottpoff(%%ebx),%0"				      \
	  : "=r" (__l));					              \
     __l; })
# endif

# ifdef __PIC__
#  define TLS_LD(x) \
  ({ int *__l, __c, __d;						      \
     __asm__ ("leal " #x "@tlsldm(%%ebx),%%eax\n\t"				      \
	  "call ___tls_get_addr@plt\n\t"				      \
	  "leal " #x "@dtpoff(%%eax), %%eax"				      \
	  : "=a" (__l), "=&c" (__c), "=&d" (__d));			      \
     __l; })
# else
#  define TLS_LD(x) \
  ({ int *__l, __b, __c, __d;						      \
     __asm__ ("call 1f\n\t"							      \
	  ".subsection 1\n"						      \
	  "1:\tmovl (%%esp), %%ebx\n\t"					      \
	  "ret\n\t"							      \
	  ".previous\n\t"						      \
	  "addl $_GLOBAL_OFFSET_TABLE_, %%ebx\n\t"			      \
	  "leal " #x "@tlsldm(%%ebx),%%eax\n\t"				      \
	  "call ___tls_get_addr@plt\n\t"				      \
	  "leal " #x "@dtpoff(%%eax), %%eax"				      \
	  : "=a" (__l), "=&b" (__b), "=&c" (__c), "=&d" (__d));		      \
     __l; })
# endif

# ifdef __PIC__
#  define TLS_GD(x) \
  ({ int *__l, __c, __d;						      \
     __asm__ ("leal " #x "@tlsgd(%%ebx),%%eax\n\t"				      \
	  "call ___tls_get_addr@plt\n\t"				      \
	  "nop"								      \
	  : "=a" (__l), "=&c" (__c), "=&d" (__d));			      \
     __l; })
# else
#  define TLS_GD(x) \
  ({ int *__l, __c, __d;						      \
     __asm__ ("call 1f\n\t"							      \
	  ".subsection 1\n"						      \
	  "1:\tmovl (%%esp), %%ebx\n\t"					      \
	  "ret\n\t"							      \
	  ".previous\n\t"						      \
	  "addl $_GLOBAL_OFFSET_TABLE_, %%ebx\n\t"			      \
	  "leal " #x "@tlsgd(%%ebx),%%eax\n\t"				      \
	  "call ___tls_get_addr@plt\n\t"				      \
	  "nop"								      \
	  : "=a" (__l), "=&c" (__c), "=&d" (__d));		              \
     __l; })
# endif

#elif defined __x86_64__

# define TLS_LE(x) \
  ({ int *__l;								      \
     __asm__ ("movq %%fs:0,%0\n\t"						      \
	  "leaq " #x "@tpoff(%0), %0"					      \
	  : "=r" (__l));						      \
     __l; })

# define TLS_IE(x) \
  ({ int *__l;								      \
     __asm__ ("movq %%fs:0,%0\n\t"						      \
	  "addq " #x "@gottpoff(%%rip),%0"				      \
	  : "=r" (__l));						      \
     __l; })

# define TLS_LD(x) \
  ({ int *__l, __c, __d;						      \
     __asm__ ("leaq " #x "@tlsld(%%rip),%%rdi\n\t"				      \
	  "call __tls_get_addr@plt\n\t"					      \
	  "leaq " #x "@dtpoff(%%rax), %%rax"				      \
	  : "=a" (__l), "=&c" (__c), "=&d" (__d)			      \
	  : : "rdi", "rsi", "r8", "r9", "r10", "r11"); 			      \
     __l; })

# define TLS_GD(x) \
  ({ int *__l, __c, __d;						      \
     __asm__ (".byte 0x66\n\t"						      \
	  "leaq " #x "@tlsgd(%%rip),%%rdi\n\t"				      \
	  ".word 0x6666\n\t"						      \
	  "rex64\n\t"							      \
	  "call __tls_get_addr@plt"					      \
	  : "=a" (__l), "=&c" (__c), "=&d" (__d)			      \
	  : : "rdi", "rsi", "r8", "r9", "r10", "r11"); 			      \
     __l; })

#elif defined __sh__

# define TLS_LE(x) \
  ({ int *__l; void *__tp;						      \
     __asm__ ("stc gbr,%1\n\t"						      \
	  "mov.l 1f,%0\n\t"						      \
	  "bra 2f\n\t"							      \
	  " add %1,%0\n\t"						      \
	  ".align 2\n\t"						      \
	  "1: .long " #x "@tpoff\n\t"					      \
	  "2:"								      \
	  : "=r" (__l), "=r" (__tp));					      \
     __l; })

# ifdef __PIC__
#  define TLS_IE(x) \
  ({ int *__l; void *__tp;						      \
     register void *__gp __asm__("r12");				      \
     __asm__ ("mov.l 1f,r0\n\t"						      \
	  "stc gbr,%1\n\t"						      \
	  "mov.l @(r0,r12),%0\n\t"					      \
	  "bra 2f\n\t"							      \
	  " add %1,%0\n\t"						      \
	  ".align 2\n\t"						      \
	  "1: .long " #x "@gottpoff\n\t"				      \
	  "2:"								      \
	  : "=r" (__l), "=r" (__tp) : "r" (__gp) : "r0");		      \
     __l; })
# else
#  define TLS_IE(x) \
  ({ int *__l; void *__tp;						      \
     __asm__ ("mov.l r12,@-r15\n\t"						      \
	  "mova 0f,r0\n\t"						      \
	  "mov.l 0f,r12\n\t"						      \
	  "add r0,r12\n\t"						      \
	  "mov.l 1f,r0\n\t"						      \
	  "stc gbr,%1\n\t"						      \
	  "mov.l @(r0,r12),%0\n\t"					      \
	  "bra 2f\n\t"							      \
	  " add %1,%0\n\t"						      \
	  ".align 2\n\t"						      \
	  "1: .long " #x "@gottpoff\n\t"				      \
	  "0: .long _GLOBAL_OFFSET_TABLE_\n\t"				      \
	  "2: mov.l @r15+,r12"						      \
	  : "=r" (__l), "=r" (__tp) : : "r0");				      \
     __l; })
#endif

# ifdef __PIC__
#  define TLS_LD(x) \
  ({ int *__l;								      \
     register void *__gp __asm__("r12");				      \
     __asm__ ("mov.l 1f,r4\n\t"						      \
	  "mova 2f,r0\n\t"						      \
	  "mov.l 2f,r1\n\t"						      \
	  "add r0,r1\n\t"						      \
	  "jsr @r1\n\t"							      \
	  " add r12,r4\n\t"						      \
	  "bra 4f\n\t"							      \
	  " nop\n\t"							      \
	  ".align 2\n\t"						      \
	  "1: .long " #x "@tlsldm\n\t"					      \
	  "2: .long __tls_get_addr@plt\n\t"				      \
	  "4: mov.l 3f,%0\n\t"						      \
	  "bra 5f\n\t"							      \
	  " add r0,%0\n\t"						      \
	  ".align 2\n\t"						      \
	  "3: .long " #x "@dtpoff\n\t"					      \
	  "5:"								      \
	  : "=r" (__l) : "r" (__gp) : "r0", "r1", "r2", "r3", "r4", "r5",     \
				      "r6", "r7", "pr", "t");		      \
     __l; })
# else
#  define TLS_LD(x) \
  ({ int *__l;								      \
     __asm__ ("mov.l r12,@-r15\n\t"						      \
	  "mova 0f,r0\n\t"						      \
	  "mov.l 0f,r12\n\t"						      \
	  "add r0,r12\n\t"						      \
	  "mov.l 1f,r4\n\t"						      \
	  "mova 2f,r0\n\t"						      \
	  "mov.l 2f,r1\n\t"						      \
	  "add r0,r1\n\t"						      \
	  "jsr @r1\n\t"							      \
	  " add r12,r4\n\t"						      \
	  "bra 4f\n\t"							      \
	  " nop\n\t"							      \
	  ".align 2\n\t"						      \
	  "1: .long " #x "@tlsldm\n\t"					      \
	  "2: .long __tls_get_addr@plt\n\t"				      \
	  "0: .long _GLOBAL_OFFSET_TABLE_\n\t"				      \
	  "4: mov.l 3f,%0\n\t"						      \
	  "bra 5f\n\t"							      \
	  " add r0,%0\n\t"						      \
	  ".align 2\n\t"						      \
	  "3: .long " #x "@dtpoff\n\t"					      \
	  "5: mov.l @r15+,r12"						      \
	  : "=r" (__l) : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",    \
			   "pr", "t");					      \
     __l; })
#endif

# ifdef __PIC__
#  define TLS_GD(x) \
  ({ int *__l;								      \
     register void *__gp __asm__("r12");				      \
     __asm__ ("mov.l 1f,r4\n\t"						      \
	  "mova 2f,r0\n\t"						      \
	  "mov.l 2f,r1\n\t"						      \
	  "add r0,r1\n\t"						      \
	  "jsr @r1\n\t"							      \
	  " add r12,r4\n\t"						      \
	  "bra 3f\n\t"							      \
	  " mov r0,%0\n\t"						      \
	  ".align 2\n\t"						      \
	  "1: .long " #x "@tlsgd\n\t"					      \
	  "2: .long __tls_get_addr@plt\n\t"				      \
	  "3:"								      \
	  : "=r" (__l) : "r" (__gp) : "r0", "r1", "r2", "r3", "r4", "r5",     \
				      "r6", "r7", "pr", "t");		      \
     __l; })
# else
#  define TLS_GD(x) \
  ({ int *__l;								      \
     __asm__ ("mov.l r12,@-r15\n\t"						      \
	  "mova 0f,r0\n\t"						      \
	  "mov.l 0f,r12\n\t"						      \
	  "add r0,r12\n\t"						      \
	  "mov.l 1f,r4\n\t"						      \
	  "mova 2f,r0\n\t"						      \
	  "mov.l 2f,r1\n\t"						      \
	  "add r0,r1\n\t"						      \
	  "jsr @r1\n\t"							      \
	  " add r12,r4\n\t"						      \
	  "bra 3f\n\t"							      \
	  " mov r0,%0\n\t"						      \
	  ".align 2\n\t"						      \
	  "1: .long " #x "@tlsgd\n\t"					      \
	  "2: .long __tls_get_addr@plt\n\t"				      \
	  "0: .long _GLOBAL_OFFSET_TABLE_\n\t"				      \
	  "3: mov.l @r15+,r12"						      \
	  : "=r" (__l) : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",    \
			   "pr", "t");					      \
     __l; })
#endif

#elif defined __alpha__

register void *__gp __asm__("$29");

# define TLS_LE(x) \
  ({ int *__l;								      \
     __asm__ ("call_pal 158\n\tlda $0," #x "($0)\t\t!tprel" : "=v"(__l));	      \
     __l; })

# define TLS_IE(x) \
  ({ char *__tp; unsigned long __o;					      \
     __asm__ ("call_pal 158\n\tldq %1," #x "($gp)\t\t!gottprel"		      \
	  : "=v"(__tp), "=r"(__o) : "r"(__gp));				      \
     (int *)(__tp + __o); })

# define TLS_LD(x) \
  ({ extern void *__tls_get_addr(void *); int *__l; void *__i;		      \
     __asm__ ("lda %0," #x "($gp)\t\t!tlsldm" : "=r" (__i) : "r"(__gp));	      \
     __i = __tls_get_addr(__i);						      \
     __asm__ ("lda %0, " #x "(%1)\t\t!dtprel" : "=r"(__l) : "r"(__i));	      \
     __l; })

# define TLS_GD(x) \
  ({ extern void *__tls_get_addr(void *); void *__i;			      \
     __asm__ ("lda %0," #x "($gp)\t\t!tlsgd" : "=r" (__i) : "r"(__gp));	      \
     (int *) __tls_get_addr(__i); })


#elif defined __ia64__

# define TLS_LE(x) \
  ({ void *__l;								      \
     __asm__ ("mov r2=r13\n\t"						      \
         ";;\n\t"							      \
         "addl %0=@tprel(" #x "),r2\n\t"				      \
         : "=r" (__l) : : "r2"  ); __l; })

# define TLS_IE(x) \
  ({ void *__l;								      \
     register long __gp __asm__ ("gp");					      \
     __asm__ (";;\n\t"							      \
	 "addl r16=@ltoff(@tprel(" #x ")),gp\n\t"			      \
         ";;\n\t"							      \
         "ld8 r17=[r16]\n\t"						      \
         ";;\n\t"							      \
         "add %0=r13,r17\n\t"						      \
         ";;\n\t"							      \
         : "=r" (__l) : "r" (__gp) : "r16", "r17" ); __l; })

# define __TLS_CALL_CLOBBERS \
  "r2", "r3", "r8", "r9", "r10", "r11", "r14", "r15", "r16", "r17",	      \
  "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26",	      \
  "r27", "r28", "r29", "r30", "r31",					      \
  "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15",	      \
  "f6", "f7", "f8", "f9", "f10", "f11", "f12", "f13", "f14", "f15",	      \
  "b6", "b7",								      \
  "out0", "out1", "out2", "out3", "out4", "out5", "out6", "out7"

# define TLS_LD(x) \
  ({ void *__l;								      \
     register long __gp __asm__ ("gp");					      \
     __asm__ (";;\n\t"							      \
	 "mov loc0=gp\n\t"						      \
         "addl r16=@ltoff(@dtpmod(" #x ")),gp\n\t"			      \
         "addl out1=@dtprel(" #x "),r0\n\t"				      \
         ";;\n\t"							      \
         "ld8 out0=[r16]\n\t"						      \
         "br.call.sptk.many b0=__tls_get_addr"				      \
         ";;\n\t"							      \
         "mov gp=loc0\n\t"						      \
         "mov %0=r8\n\t"						      \
         ";;\n\t"							      \
         : "=r" (__l) : "r" (__gp) : "loc0", __TLS_CALL_CLOBBERS);	      \
     __l; })

# define TLS_GD(x) \
  ({ void *__l;								      \
     register long __gp __asm__ ("gp");					      \
     __asm__ (";;\n\t"							      \
	 "mov loc0=gp\n\t"						      \
         "addl r16=@ltoff(@dtpmod(" #x ")),gp\n\t"			      \
         "addl r17=@ltoff(@dtprel(" #x ")),gp\n\t"			      \
         ";;\n\t"							      \
         "ld8 out0=[r16]\n\t"						      \
         "ld8 out1=[r17]\n\t"						      \
         "br.call.sptk.many b0=__tls_get_addr"				      \
         ";;\n\t"							      \
         "mov gp=loc0\n\t"						      \
         "mov %0=r8\n\t"						      \
         ";;\n\t"							      \
          : "=r" (__l) : "r" (__gp) : "loc0", __TLS_CALL_CLOBBERS);	      \
     __l; })

#elif defined __sparc__ && !defined __arch64__

# define TLS_LE(x) \
  ({ int *__l;								      \
     __asm__ ("sethi %%tle_hix22(" #x "), %0" : "=r" (__l));		      \
     __asm__ ("xor %1, %%tle_lox10(" #x "), %0" : "=r" (__l) : "r" (__l));	      \
     __asm__ ("add %%g7, %1, %0" : "=r" (__l) : "r" (__l));			      \
     __l; })

# ifdef __PIC__
#  define TLS_LOAD_PIC \
  ({ register long pc __asm__ ("%o7");					      \
     long got;								      \
     __asm__ ("sethi %%hi(_GLOBAL_OFFSET_TABLE_-4), %1\n\t"			      \
	  "call .+8\n\t"						      \
	  "add %1, %%lo(_GLOBAL_OFFSET_TABLE_+4), %1\n\t"		      \
	  "add %1, %0, %1\n\t"						      \
	  : "=r" (pc), "=r" (got));					      \
     got; })
# else
#  define TLS_LOAD_PIC \
   ({ long got;								      \
      __asm__ (".hidden _GLOBAL_OFFSET_TABLE_\n\t"				      \
	   "sethi %%hi(_GLOBAL_OFFSET_TABLE_), %0\n\t"			      \
	   "or %0, %%lo(_GLOBAL_OFFSET_TABLE_), %0"			      \
	   : "=r" (got));						      \
      got; })
# endif

# define TLS_IE(x) \
  ({ int *__l;								      \
     __asm__ ("sethi %%tie_hi22(" #x "), %0" : "=r" (__l));			      \
     __asm__ ("add %1, %%tie_lo10(" #x "), %0" : "=r" (__l) : "r" (__l));	      \
     __asm__ ("ld [%1 + %2], %0, %%tie_ld(" #x ")"				      \
	  : "=r" (__l) : "r" (TLS_LOAD_PIC), "r" (__l));		      \
     __asm__ ("add %%g7, %1, %0, %%tie_add(" #x ")" : "=r" (__l) : "r" (__l));    \
     __l; })

# define TLS_LD(x) \
  ({ int *__l; register void *__o0 __asm__ ("%o0");				      \
     long __o;								      \
     __asm__ ("sethi %%tldm_hi22(" #x "), %0" : "=r" (__l));		      \
     __asm__ ("add %1, %%tldm_lo10(" #x "), %0" : "=r" (__l) : "r" (__l));	      \
     __asm__ ("add %1, %2, %0, %%tldm_add(" #x ")"				      \
	  : "=r" (__o0) : "r" (TLS_LOAD_PIC), "r" (__l));		      \
     __asm__ ("call __tls_get_addr, %%tgd_call(" #x ")\n\t"			      \
	  " nop"							      \
	  : "=r" (__o0) : "0" (__o0)					      \
	  : "g1", "g2", "g3", "g4", "g5", "g6", "o1", "o2", "o3", "o4",	      \
	    "o5", "o7", "cc");						      \
     __asm__ ("sethi %%tldo_hix22(" #x "), %0" : "=r" (__o));		      \
     __asm__ ("xor %1, %%tldo_lox10(" #x "), %0" : "=r" (__o) : "r" (__o));	      \
     __asm__ ("add %1, %2, %0, %%tldo_add(" #x ")" : "=r" (__l)		      \
	  : "r" (__o0), "r" (__o));					      \
     __l; })

# define TLS_GD(x) \
  ({ int *__l; register void *__o0 __asm__ ("%o0");				      \
     __asm__ ("sethi %%tgd_hi22(" #x "), %0" : "=r" (__l));			      \
     __asm__ ("add %1, %%tgd_lo10(" #x "), %0" : "=r" (__l) : "r" (__l));	      \
     __asm__ ("add %1, %2, %0, %%tgd_add(" #x ")"				      \
	  : "=r" (__o0) : "r" (TLS_LOAD_PIC), "r" (__l));		      \
     __asm__ ("call __tls_get_addr, %%tgd_call(" #x ")\n\t"			      \
	  " nop"							      \
	  : "=r" (__o0) : "0" (__o0)					      \
	  : "g1", "g2", "g3", "g4", "g5", "g6", "o1", "o2", "o3", "o4",	      \
	    "o5", "o7", "cc");						      \
     __o0; })

#elif defined __sparc__ && defined __arch64__

# define TLS_LE(x) \
  ({ int *__l;								      \
     __asm__ ("sethi %%tle_hix22(" #x "), %0" : "=r" (__l));		      \
     __asm__ ("xor %1, %%tle_lox10(" #x "), %0" : "=r" (__l) : "r" (__l));	      \
     __asm__ ("add %%g7, %1, %0" : "=r" (__l) : "r" (__l));			      \
     __l; })

# ifdef __PIC__
#  define TLS_LOAD_PIC \
  ({ long pc, got;							      \
     __asm__ ("sethi %%hi(_GLOBAL_OFFSET_TABLE_-4), %1\n\t"			      \
	  "rd %%pc, %0\n\t"						      \
	  "add %1, %%lo(_GLOBAL_OFFSET_TABLE_+4), %1\n\t"		      \
	  "add %1, %0, %1\n\t"						      \
	  : "=r" (pc), "=r" (got));					      \
     got; })
# else
#  define TLS_LOAD_PIC \
   ({ long got;								      \
      __asm__ (".hidden _GLOBAL_OFFSET_TABLE_\n\t"				      \
	   "sethi %%hi(_GLOBAL_OFFSET_TABLE_), %0\n\t"			      \
	   "or %0, %%lo(_GLOBAL_OFFSET_TABLE_), %0"			      \
	   : "=r" (got));						      \
      got; })
# endif

# define TLS_IE(x) \
  ({ int *__l;								      \
     __asm__ ("sethi %%tie_hi22(" #x "), %0" : "=r" (__l));			      \
     __asm__ ("add %1, %%tie_lo10(" #x "), %0" : "=r" (__l) : "r" (__l));	      \
     __asm__ ("ldx [%1 + %2], %0, %%tie_ldx(" #x ")"			      \
	  : "=r" (__l) : "r" (TLS_LOAD_PIC), "r" (__l));		      \
     __asm__ ("add %%g7, %1, %0, %%tie_add(" #x ")" : "=r" (__l) : "r" (__l));    \
     __l; })

# define TLS_LD(x) \
  ({ int *__l; register void *__o0 __asm__ ("%o0");				      \
     long __o;								      \
     __asm__ ("sethi %%tldm_hi22(" #x "), %0" : "=r" (__l));		      \
     __asm__ ("add %1, %%tldm_lo10(" #x "), %0" : "=r" (__l) : "r" (__l));	      \
     __asm__ ("add %1, %2, %0, %%tldm_add(" #x ")"				      \
	  : "=r" (__o0) : "r" (TLS_LOAD_PIC), "r" (__l));		      \
     __asm__ ("call __tls_get_addr, %%tgd_call(" #x ")\n\t"			      \
	  " nop"							      \
	  : "=r" (__o0) : "0" (__o0)					      \
	  : "g1", "g2", "g3", "g4", "g5", "g6", "o1", "o2", "o3", "o4",	      \
	    "o5", "o7", "cc");						      \
     __asm__ ("sethi %%tldo_hix22(" #x "), %0" : "=r" (__o));		      \
     __asm__ ("xor %1, %%tldo_lox10(" #x "), %0" : "=r" (__o) : "r" (__o));	      \
     __asm__ ("add %1, %2, %0, %%tldo_add(" #x ")" : "=r" (__l)		      \
	  : "r" (__o0), "r" (__o));					      \
     __l; })

# define TLS_GD(x) \
  ({ int *__l; register void *__o0 __asm__ ("%o0");				      \
     __asm__ ("sethi %%tgd_hi22(" #x "), %0" : "=r" (__l));			      \
     __asm__ ("add %1, %%tgd_lo10(" #x "), %0" : "=r" (__l) : "r" (__l));	      \
     __asm__ ("add %1, %2, %0, %%tgd_add(" #x ")"				      \
	  : "=r" (__o0) : "r" (TLS_LOAD_PIC), "r" (__l));		      \
     __asm__ ("call __tls_get_addr, %%tgd_call(" #x ")\n\t"			      \
	  " nop"							      \
	  : "=r" (__o0) : "0" (__o0)					      \
	  : "g1", "g2", "g3", "g4", "g5", "g6", "o1", "o2", "o3", "o4",	      \
	    "o5", "o7", "cc");						      \
     __o0; })

#elif defined __s390x__

# define TLS_LE(x) \
  ({ unsigned long __offset;						      \
     __asm__ ("bras %0,1f\n"						      \
	  "0:\t.quad " #x "@ntpoff\n"					      \
	  "1:\tlg %0,0(%0)"						      \
	  : "=a" (__offset) : : "cc" );					      \
     (int *) (__builtin_thread_pointer() + __offset); })

# ifdef __PIC__
#  define TLS_IE(x) \
  ({ unsigned long __offset;						      \
     __asm__ ("bras %0,1f\n"						      \
	  "0:\t.quad " #x "@gotntpoff\n"				      \
	  "1:\tlg %0,0(%0)\n\t"						      \
	  "lg %0,0(%0,%%r12):tls_load:" #x				      \
	  : "=&a" (__offset) : : "cc" );				      \
     (int *) (__builtin_thread_pointer() + __offset); })
# else
#  define TLS_IE(x) \
  ({ unsigned long  __offset;						      \
     __asm__ ("bras %0,1f\n"						      \
	  "0:\t.quad " #x "@indntpoff\n"				      \
	  "1:\t lg %0,0(%0)\n\t"					      \
	  "lg %0,0(%0):tls_load:" #x					      \
	  : "=&a" (__offset) : : "cc" );				      \
     (int *) (__builtin_thread_pointer() + __offset); })
# endif

# ifdef __PIC__
#  define TLS_LD(x) \
  ({ unsigned long __offset, __save12;					      \
     __asm__ ("bras %0,1f\n"						      \
	  "0:\t.quad " #x "@tlsldm\n\t"					      \
	  ".quad " #x "@dtpoff\n"					      \
	  "1:\tlgr %1,%%r12\n\t"					      \
          "larl %%r12,_GLOBAL_OFFSET_TABLE_\n\t"			      \
          "lg %%r2,0(%0)\n\t"						      \
	  "brasl %%r14,__tls_get_offset@plt:tls_ldcall:" #x "\n\t"	      \
	  "lg %0,8(%0)\n\t"						      \
	  "algr %0,%%r2\n\t"						      \
          "lgr %%r12,%1"						      \
	  : "=&a" (__offset), "=&a" (__save12)				      \
          : : "cc", "0", "1", "2", "3", "4", "5", "14" );		      \
     (int *) (__builtin_thread_pointer() + __offset); })
# else
#  define TLS_LD(x) \
  ({ unsigned long __offset;						      \
     __asm__ ("bras %0,1f\n"						      \
	  "0:\t.quad " #x "@tlsldm\n\t"					      \
	  ".quad " #x "@dtpoff\n"					      \
	  "1:\tlarl %%r12,_GLOBAL_OFFSET_TABLE_\n\t"			      \
          "lg %%r2,0(%0)\n\t"						      \
	  "brasl %%r14,__tls_get_offset@plt:tls_ldcall:" #x "\n\t"	      \
	  "lg %0,8(%0)\n\t"						      \
	  "algr %0,%%r2"						      \
	  : "=&a" (__offset)						      \
	  : : "cc", "0", "1", "2", "3", "4", "5", "12", "14" );		      \
     (int *) (__builtin_thread_pointer() + __offset); })
# endif

# ifdef __PIC__
#  define TLS_GD(x) \
  ({ unsigned long __offset, __save12;					      \
     __asm__ ("bras %0,1f\n"						      \
	  "0:\t.quad " #x "@tlsgd\n"					      \
	  "1:\tlgr %1,%%r12\n\t"					      \
	  "larl %%r12,_GLOBAL_OFFSET_TABLE_\n\t"			      \
          "lg %%r2,0(%0)\n\t"						      \
	  "brasl %%r14,__tls_get_offset@plt:tls_gdcall:" #x "\n\t"	      \
          "lgr %0,%%r2\n\t"						      \
          "lgr %%r12,%1"						      \
	  : "=&a" (__offset), "=&a" (__save12)				      \
          : : "cc", "0", "1", "2", "3", "4", "5", "14" );		      \
     (int *) (__builtin_thread_pointer() + __offset); })
# else
#  define TLS_GD(x) \
  ({ unsigned long __offset;						      \
     __asm__ ("bras %0,1f\n"						      \
	  "0:\t.quad " #x "@tlsgd\n"					      \
	  "1:\tlarl %%r12,_GLOBAL_OFFSET_TABLE_\n\t"			      \
	  "lg %%r2,0(%0)\n\t"						      \
	  "brasl %%r14,__tls_get_offset@plt:tls_gdcall:" #x "\n\t"	      \
          "lgr %0,%%r2"							      \
	  : "=&a" (__offset)						      \
	  : : "cc", "0", "1", "2", "3", "4", "5", "12", "14" );		      \
     (int *) (__builtin_thread_pointer() + __offset); })
# endif

#elif defined __s390__

# define TLS_LE(x) \
  ({ unsigned long __offset;						      \
     __asm__ ("bras %0,1f\n"						      \
	  "0:\t.long " #x "@ntpoff\n"					      \
	  "1:\tl %0,0(%0)"						      \
	  : "=a" (__offset) : : "cc" );					      \
     (int *) (__builtin_thread_pointer() + __offset); })

# ifdef __PIC__
#  define TLS_IE(x) \
  ({ unsigned long __offset;						      \
     __asm__ ("bras %0,1f\n"						      \
	  "0:\t.long " #x "@gotntpoff\n"				      \
	  "1:\tl %0,0(%0)\n\t"						      \
	  "l %0,0(%0,%%r12):tls_load:" #x				      \
	  : "=&a" (__offset) : : "cc" );				      \
     (int *) (__builtin_thread_pointer() + __offset); })
# else
#  define TLS_IE(x) \
  ({ unsigned long  __offset;						      \
     __asm__ ("bras %0,1f\n"						      \
	  "0:\t.long " #x "@indntpoff\n"				      \
	  "1:\t l %0,0(%0)\n\t"						      \
	  "l %0,0(%0):tls_load:" #x					      \
	  : "=&a" (__offset) : : "cc" );				      \
     (int *) (__builtin_thread_pointer() + __offset); })
# endif

# ifdef __PIC__
#  define TLS_LD(x) \
  ({ unsigned long __offset, __save12;					      \
     __asm__ ("bras %0,1f\n"						      \
	  "0:\t.long _GLOBAL_OFFSET_TABLE_-0b\n\t"			      \
	  ".long __tls_get_offset@plt-0b\n\t"				      \
	  ".long " #x "@tlsldm\n\t"					      \
	  ".long " #x "@dtpoff\n"					      \
	  "1:\tlr %1,%%r12\n\t"						      \
          "l %%r12,0(%0)\n\t"						      \
          "la %%r12,0(%%r12,%0)\n\t"					      \
	  "l %%r1,4(%0)\n\t"						      \
	  "l %%r2,8(%0)\n\t"						      \
	  "bas %%r14,0(%%r1,%0):tls_ldcall:" #x "\n\t"			      \
	  "l %0,12(%0)\n\t"						      \
	  "alr %0,%%r2\n\t"						      \
          "lr %%r12,%1"							      \
	  : "=&a" (__offset), "=&a" (__save12)				      \
          : : "cc", "0", "1", "2", "3", "4", "5" );			      \
     (int *) (__builtin_thread_pointer() + __offset); })
# else
#  define TLS_LD(x) \
  ({ unsigned long __offset;						      \
     __asm__ ("bras %0,1f\n"						      \
	  "0:\t.long _GLOBAL_OFFSET_TABLE_\n\t"				      \
	  ".long __tls_get_offset@plt\n\t"				      \
	  ".long " #x "@tlsldm\n\t"					      \
	  ".long " #x "@dtpoff\n"					      \
	  "1:\tl %%r12,0(%0)\n\t"					      \
	  "l %%r1,4(%0)\n\t"						      \
	  "l %%r2,8(%0)\n\t"						      \
	  "bas %%r14,0(%%r1):tls_ldcall:" #x "\n\t"			      \
	  "l %0,12(%0)\n\t"						      \
	  "alr %0,%%r2"							      \
	  : "=&a" (__offset) : : "cc", "0", "1", "2", "3", "4", "5", "12" );  \
     (int *) (__builtin_thread_pointer() + __offset); })
# endif

# ifdef __PIC__
#  define TLS_GD(x) \
  ({ unsigned long __offset, __save12;					      \
     __asm__ ("bras %0,1f\n"						      \
	  "0:\t.long _GLOBAL_OFFSET_TABLE_-0b\n\t"			      \
	  ".long __tls_get_offset@plt-0b\n\t"				      \
	  ".long " #x "@tlsgd\n"					      \
	  "1:\tlr %1,%%r12\n\t"						      \
          "l %%r12,0(%0)\n\t"						      \
          "la %%r12,0(%%r12,%0)\n\t"					      \
	  "l %%r1,4(%0)\n\t"						      \
	  "l %%r2,8(%0)\n\t"						      \
	  "bas %%r14,0(%%r1,%0):tls_gdcall:" #x "\n\t"			      \
          "lr %0,%%r2\n\t"						      \
          "lr %%r12,%1"							      \
	  : "=&a" (__offset), "=&a" (__save12)				      \
          : : "cc", "0", "1", "2", "3", "4", "5" );			      \
     (int *) (__builtin_thread_pointer() + __offset); })
# else
#  define TLS_GD(x) \
  ({ unsigned long __offset;						      \
     __asm__ ("bras %0,1f\n"						      \
	  "0:\t.long _GLOBAL_OFFSET_TABLE_\n\t"				      \
	  ".long __tls_get_offset@plt\n\t"				      \
	  ".long " #x "@tlsgd\n"					      \
	  "1:\tl %%r12,0(%0)\n\t"					      \
	  "l %%r1,4(%0)\n\t"						      \
	  "l %%r2,8(%0)\n\t"						      \
	  "bas %%r14,0(%%r1):tls_gdcall:" #x "\n\t"			      \
          "lr %0,%%r2"							      \
	  : "=&a" (__offset) : : "cc", "0", "1", "2", "3", "4", "5", "12" );  \
     (int *) (__builtin_thread_pointer() + __offset); })
# endif

#elif defined __powerpc__ && !defined __powerpc64__

/*#include "config.h"*/

# define __TLS_CALL_CLOBBERS						\
	"0", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12",	\
	"lr", "ctr", "cr0", "cr1", "cr5", "cr6", "cr7"

/* PowerPC32 Local Exec TLS access.  */
# define TLS_LE(x)				\
  ({ int *__result;				\
     __asm__ ("addi %0,2," #x "@tprel"		\
	  : "=r" (__result));			\
     __result; })

/* PowerPC32 Initial Exec TLS access.  */
# ifdef HAVE_ASM_PPC_REL16
#  define TLS_IE(x)					\
  ({ int *__result;					\
     __asm__ ("bcl 20,31,1f\n1:\t"				\
	  "mflr %0\n\t"					\
	  "addis %0,%0,_GLOBAL_OFFSET_TABLE_-1b@ha\n\t"	\
	  "addi %0,%0,_GLOBAL_OFFSET_TABLE_-1b@l\n\t"	\
	  "lwz %0," #x "@got@tprel(%0)\n\t"		\
	  "add %0,%0," #x "@tls"			\
	  : "=b" (__result) :				\
	  : "lr");					\
     __result; })
# else
#  define TLS_IE(x)					\
  ({ int *__result;					\
     __asm__ ("bl _GLOBAL_OFFSET_TABLE_@local-4\n\t"	\
	  "mflr %0\n\t"					\
	  "lwz %0," #x "@got@tprel(%0)\n\t"		\
	  "add %0,%0," #x "@tls"			\
	  : "=b" (__result) :				\
	  : "lr");					\
     __result; })
# endif

/* PowerPC32 Local Dynamic TLS access.  */
# ifdef HAVE_ASM_PPC_REL16
#  define TLS_LD(x)					\
  ({ int *__result;					\
     __asm__ ("bcl 20,31,1f\n1:\t"				\
	  "mflr 3\n\t"					\
	  "addis 3,3,_GLOBAL_OFFSET_TABLE_-1b@ha\n\t"	\
	  "addi 3,3,_GLOBAL_OFFSET_TABLE_-1b@l\n\t"	\
	  "addi 3,3," #x "@got@tlsld\n\t"		\
	  "bl __tls_get_addr@plt\n\t"			\
	  "addi %0,3," #x "@dtprel"			\
	  : "=r" (__result) :				\
	  : __TLS_CALL_CLOBBERS);			\
     __result; })
# else
#  define TLS_LD(x)					\
  ({ int *__result;					\
     __asm__ ("bl _GLOBAL_OFFSET_TABLE_@local-4\n\t"	\
	  "mflr 3\n\t"					\
	  "addi 3,3," #x "@got@tlsld\n\t"		\
	  "bl __tls_get_addr@plt\n\t"			\
	  "addi %0,3," #x "@dtprel"			\
	  : "=r" (__result) :				\
	  : __TLS_CALL_CLOBBERS);			\
     __result; })
# endif

/* PowerPC32 General Dynamic TLS access.  */
# ifdef HAVE_ASM_PPC_REL16
#  define TLS_GD(x)					\
  ({ register int *__result __asm__ ("r3");		\
     __asm__ ("bcl 20,31,1f\n1:\t"				\
	  "mflr 3\n\t"					\
	  "addis 3,3,_GLOBAL_OFFSET_TABLE_-1b@ha\n\t"	\
	  "addi 3,3,_GLOBAL_OFFSET_TABLE_-1b@l\n\t"	\
	  "addi 3,3," #x "@got@tlsgd\n\t"		\
	  "bl __tls_get_addr@plt"			\
	  : :						\
	  : __TLS_CALL_CLOBBERS);			\
     __result; })
# else
#  define TLS_GD(x)					\
  ({ register int *__result __asm__ ("r3");		\
     __asm__ ("bl _GLOBAL_OFFSET_TABLE_@local-4\n\t"	\
	  "mflr 3\n\t"					\
	  "addi 3,3," #x "@got@tlsgd\n\t"		\
	  "bl __tls_get_addr@plt"			\
	  : :						\
	  : __TLS_CALL_CLOBBERS);			\
     __result; })
# endif

#elif defined __powerpc__ && defined __powerpc64__

/* PowerPC64 Local Exec TLS access.  */
# define TLS_LE(x) \
  ({  int * __result;  \
      __asm__ ( \
        "  addis %0,13," #x "@tprel@ha\n"  \
        "  addi  %0,%0," #x "@tprel@l\n"   \
        : "=b" (__result) );  \
      __result;  \
  })
/* PowerPC64 Initial Exec TLS access.  */
#  define TLS_IE(x) \
  ({  int * __result;  \
      __asm__ (  \
        "  ld  %0," #x "@got@tprel(2)\n"  \
        "  add %0,%0," #x "@tls\n"   \
        : "=b" (__result) );  \
      __result;  \
  })
/* PowerPC64 Local Dynamic TLS access.  */
#  define TLS_LD(x) \
  ({  int * __result;  \
      __asm__ (  \
        "  addi  3,2," #x "@got@tlsld\n"  \
        "  bl    .__tls_get_addr\n"  \
        "  nop   \n"  \
        "  addis %0,3," #x "@dtprel@ha\n"  \
        "  addi  %0,%0," #x "@dtprel@l\n"  \
        : "=b" (__result) :  \
        : "0", "3", "4", "5", "6", "7",    \
          "8", "9", "10", "11", "12",      \
          "lr", "ctr",  \
          "cr0", "cr1", "cr5", "cr6", "cr7");  \
      __result;  \
  })
/* PowerPC64 General Dynamic TLS access.  */
#  define TLS_GD(x) \
  ({  int * __result;  \
      __asm__ (  \
        "  addi  3,2," #x "@got@tlsgd\n"  \
        "  bl    .__tls_get_addr\n"  \
        "  nop   \n"  \
        "  mr    %0,3\n"  \
        : "=b" (__result) :  \
        : "0", "3", "4", "5", "6", "7",    \
          "8", "9", "10", "11", "12",      \
          "lr", "ctr",  \
          "cr0", "cr1", "cr5", "cr6", "cr7");  \
      __result;  \
  })

#elif !defined TLS_LE || !defined TLS_IE \
      || !defined TLS_LD || !defined TLS_GD
# error "No support for this architecture so far."
#endif
