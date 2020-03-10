#include<sys/types.h>
#include<sys/stat.h>
#include<stdio.h>
#include<fcntl.h>
#define BUF_SIZE 1024

char buffer[BUF_SIZE];

int main()
{
	int fd;
	int cnt = 0;
	fd = open("/dev/easy_drive",O_RDWR);
	if(fd != -1)
	{
		printf("Input: \n");
		scanf("%s",buffer);
		while(buffer[cnt])
		{
			cnt++;
		}
		if(write(fd,buffer,cnt))
		{
			if(read(fd,buffer,cnt))
				printf("Output:\n%s\n",buffer);
			else
				printf("Read Device Error !\n");
		}
		else 
			printf("Write Device Error !\n");
	}
	else
	{
		printf("Open Error !\n");
		return 0;
	}
	close(fd);
	return 0;
}