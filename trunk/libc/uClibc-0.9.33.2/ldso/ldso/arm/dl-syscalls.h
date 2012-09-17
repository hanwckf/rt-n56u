/* _call_via_rX calls are used in thumb ldso because of calls via
 * function pointers, but ldso is not linked with anything which
 * provides them, so define them here (only required for thumb).
 */
#if defined(__thumb__)
__asm__(
	".macro call_via register\n"
	"	.global	_call_via_\\register\n"
	"	.hidden	_call_via_\\register\n"
	"	.type	_call_via_\\register, %function\n"
	"	.thumb_func\n"
	"_call_via_\\register:\n"
	"	bx	\\register\n"
	"	.size	_call_via_\\register, . - _call_via_\\register\n"
	".endm\n"

	".text\n"
	".thumb\n"
	".align 1\n"
	"	call_via r0\n"
	"	call_via r1\n"
	"	call_via r2\n"
	"	call_via r3\n"
	"	call_via r4\n"
	"	call_via r5\n"
	"	call_via r6\n"
	"	call_via r7\n"
	"	call_via r8\n"
	"	call_via r9\n"
	"	call_via r10\n"
	"	call_via r11\n"
	"	call_via r12\n"
	"	call_via r13\n"
	"	call_via r14\n"
);
#endif
