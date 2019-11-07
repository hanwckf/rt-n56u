#define L1_SCRATCH_START	0xFFB00000

/* Data that is "mapped" into the process VM at the start of the L1 scratch
   memory, so that each process can access it at a fixed address.  Used for
   stack checking.  */
struct l1_scratch_task_info
{
	/* Points to the start of the stack.  */
	void *stack_start;
	/* Not updated by the kernel; a user process can modify this to
	   keep track of the lowest address of the stack pointer during its
	   runtime.  */
	void *lowest_sp;
};

/* A pointer to the structure in memory.  */
#define L1_SCRATCH_TASK_INFO ((struct l1_scratch_task_info *)L1_SCRATCH_START)
