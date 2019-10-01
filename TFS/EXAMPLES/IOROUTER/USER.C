/* @(#)user.c	1.1 */

#include <stdio.h>
#include <stdlib.h>
#include <channel.h>
#include <process.h>
#include <misc.h>
#include <iocntrl.h>


#define BLOCK (1024*8)
#define COUNT ((1024*1024)/BLOCK)

char buf[BLOCK];
char first_write[BLOCK];
char last_write [BLOCK];
char first_read[BLOCK];
char last_read[BLOCK];


int main(int argc, char *argv[])
{
	int i;
	unsigned int t1, t2;
	double tick = ProcGetPriority() ? 64.0e-6 : 1.0e-6;
	int fh;
	int err_count;
	char name[20];
	int user;

	user = *( (int*)get_param(3) );

	printf ("User %d running\n", user);

	first_write[0] = 35;
	last_write[0] = 143;

	for (i=1; i<BLOCK; i++)
	{
		first_write[i] = (int)(first_write[i-1])*329857 + 985793;
		last_write[i] = (int)(last_write[i-1])*386157 + 298735;
	}

	sprintf(name, "TFS:test%d", user );

	printf ("openning \"%s\"\n", name);
	fh = open(name, O_WRONLY | O_CREAT );
	if (fh == -1)
	{
		fprintf(stderr, "open-write failed\n");
		exit_terminate(-1);
	}

	write (fh, first_write, BLOCK);
	t1 = ProcTime();
	for (i=0; i<COUNT; i++)
		write (fh, buf, BLOCK);
	t2 = ProcTime();
	write (fh, last_write, BLOCK);

	printf ("write %d bytes in %d byte blocks : %d ticks, %f Mb/s\n",
		COUNT*BLOCK, BLOCK, t2-t1, (COUNT*BLOCK)/((double)(t2-t1) * tick * 1024 *1024) );

	close(fh);

	fh = open(name, O_RDONLY);
	if (fh == -1)
	{
		fprintf(stderr, "open-read failed\n");
		exit_terminate(-1);
	}


	read (fh, first_read, BLOCK);
	t1 = ProcTime();
	for (i=0; i<COUNT; i++)
		read (fh, buf, BLOCK);
	t2 = ProcTime();
	read (fh, last_read, BLOCK);


	printf ("read %d bytes in %d byte blocks : %d ticks, %f Mb/s\n",
		COUNT*BLOCK, BLOCK, t2-t1, (COUNT*BLOCK)/((double)(t2-t1) * tick * 1024 *1024) );

	err_count = 0;
	for (i=0; i<BLOCK; i++)
	{
		if (first_read[i] != first_write[i])
			err_count++;
		if (last_read[i] != last_write[i])
			err_count++;
	}

	printf ("%d errors in first and last blocks\n", err_count);
	close(fh);

	exit_terminate(0);
}

