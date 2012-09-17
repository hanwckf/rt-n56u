/* make sure we handle argv[0] == NULL */

#include <unistd.h>

int main(int argc, char *argv[])
{
	if (argc == 0)
		return 0;

	char *exec_argv[1], *exec_envp[1];
	exec_argv[0] = exec_envp[0] = NULL;
	return execve("./exec-null", exec_argv, exec_envp);
}
