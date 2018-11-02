/************************************************************************\
 * file name 	: appl.c						*
 * Description 	: Application test basic character driver		*
 * AUthor      	: Sateesh KG						*
\************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main()
{
	int fd;

	char buf[100];
	int n =0;

	printf("%d\n", getpid());

	if((	fd = open("/dev/skdevice", O_RDWR) ) < 0)
		perror("open:");

	getchar();	
	printf("\n Now calling read first time \n");
	while((n=read(fd, buf, 15)) > 0 )
	{	
		write(STDOUT_FILENO,buf, n);
		printf("\n");
	}

	printf("\n");

	printf("Enter data to write to driver: ");
	scanf("%[^\n]", buf);

	n = strlen(buf);

	write(fd, buf, n);

	printf("\n Now calling read second time \n");
	while((n=read(fd, buf, 15)) > 0 )
	{	
		write(STDOUT_FILENO,buf, n);
		printf("\n");
	}


	close(fd);

	return 0;
}




