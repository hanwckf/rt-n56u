/* make sure we can vfork/exec across setjmp/longjmp's
 * and make sure signal block masks don't get corrupted
 * in the process.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int verbose = 0;

static int execute_child(const char *prog)
{
	int status;
	pid_t child;
	child = vfork();
	if (child == 0) {
		execlp(prog, prog, NULL);
		perror("Could not execute specified prog");
		_exit(1);
	} else if (child == 1)
		return 1;
	wait(&status);
	return WEXITSTATUS(status);
}

sigset_t orig_mask;

static int check_sig_mask(void)
{
	int status;
	pid_t child;

	child = vfork();
	if (child == 0) {
		int ret;
		sigset_t child_mask;
		memset(&child_mask, 0x00, sizeof(child_mask));
		ret = sigprocmask(SIG_BLOCK, NULL, &child_mask);
		if (ret != 0) {
			perror("could not get child sig block mask");
			_exit(1);
		}
		ret = memcmp(&orig_mask, &child_mask, sizeof(orig_mask));
		if (verbose) {
			printf("sigmsk: %08lx%08lx ", child_mask.__val[1], child_mask.__val[0]);
			printf("sigmsk: %08lx%08lx ", orig_mask.__val[1], orig_mask.__val[0]);
			printf("%i\n", ret);
		}
		_exit(ret);
	} else if (child == 1)
		return 1;
	wait(&status);
	return WEXITSTATUS(status);
}

int main(int argc, char *argv[])
{
	const char *prog;
	jmp_buf env;
	sigjmp_buf sigenv;
	int max;
	/* values modified between setjmp/longjmp cannot be local to this func */
	static int cnt, ret;

	memset(&orig_mask, 0x00, sizeof(orig_mask));
	ret = sigprocmask(SIG_BLOCK, NULL, &orig_mask);
	if (ret != 0) {
		perror("could not get orig sig block mask");
		return 1;
	}

	prog = (argc > 1 ? argv[1] : "true");
	ret = 0;
	verbose = 0;
	max = 10;

	/* test vfork()/exec() inside of sigsetjmp/siglongjmp */
	cnt = 0;
	sigsetjmp(sigenv, 1);
	++cnt;
	if (verbose)
		printf("sigsetjmp loop %i\n", cnt);
	ret |= check_sig_mask();
	ret |= execute_child(prog);
	if (cnt < max)
		siglongjmp(sigenv, 0);

	/* test vfork()/sigprocmask() inside of setjmp/longjmp */
	cnt = 0;
	setjmp(env);
	++cnt;
	if (verbose)
		printf("setjmp loop %i\n", cnt);
	ret |= check_sig_mask();
	ret |= execute_child(prog);
	if (cnt < max)
		longjmp(env, 0);

	return ret;
}
