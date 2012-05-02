#include <linux/oprofile.h>
#include <linux/sched.h>

#include <asm/ptrace.h>
#include <asm/stacktrace.h>

static inline void do_user_backtrace(unsigned long low_addr,
				     struct user_stackframe *frame,
				     unsigned int depth)
{
	const unsigned int max_instr_check = 512;
	const unsigned long high_addr = low_addr + THREAD_SIZE;

	while (depth-- && !unwind_user_frame(frame, max_instr_check)) {
		oprofile_add_trace(frame->ra);
		if (frame->sp < low_addr || frame->sp > high_addr)
			break;
	}
}

#ifndef CONFIG_KALLSYMS
static inline void do_kernel_backtrace(unsigned long low_addr,
				       struct user_stackframe *frame,
				       unsigned int depth) { }
#else
static inline void do_kernel_backtrace(unsigned long low_addr,
				       struct user_stackframe *frame,
				       unsigned int depth)
{
	while (depth-- && frame->pc) {
		frame->pc = unwind_stack_by_address(low_addr,
						    &(frame->sp),
						    frame->pc,
						    &(frame->ra));
		oprofile_add_trace(frame->ra);
	}
}
#endif

void op_mips_backtrace(struct pt_regs *const regs, unsigned int depth)
{
	struct user_stackframe frame = { .sp = regs->regs[29],
				    .pc = regs->cp0_epc,
				    .ra = regs->regs[31] };
	const int userspace = user_mode(regs);
	const unsigned long low_addr = ALIGN(frame.sp, THREAD_SIZE);

	if (userspace)
		do_user_backtrace(low_addr, &frame, depth);
	else
		do_kernel_backtrace(low_addr, &frame, depth);
}
