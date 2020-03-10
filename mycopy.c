#include<stdio.h>
#include<sys/syscall.h>
#include<linux/kernel.h>
#include<unistd.h>

int main(int argc, char* argv[])
{
	long key;
	key = syscall(335,argv[1],argv[2]);
	return 0;
}
