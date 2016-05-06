#include <unistd.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
	system("ps l");

	printf("\n");

	printf("my pid -> %d\n",getpid());
	printf("my ppid -> %d\n",getppid());
	printf("my pgid -> %d\n",getpgid(0));
	printf("\n\n\n");

	return 0;
}