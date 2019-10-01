
#include <stdio.h>
#include <stdlib.h>
#include <iocntrl.h>
#include <string.h>
#include <assert.h>
#include <process.h>
#include "tfs.h"
#include "tfs_util.h"


void benchmark( struct command *command, int argc, char **argv)
{
	char *buffer;
	int times;
	int size;
	int fd;
	int second;
	int start;
	int end;
	int interval;
	int kb;
	int i;

	if (argc != 3)
	{
		usage( command );
		return;
	}

	size = atoi(argv[1]);
	times = atoi(argv[2]);

	if ((size <= 0) || (times <= 0))
	{
		printf( "Invalid arguments.\n" );
		usage( command );
		return;
	}

	buffer = (char*)calloc(size,1);

	if (buffer == NULL)
	{
		printf( "Can't malloc %d bytes\n", size);
		return;
	}

	second = ProcGetPriority() ? 15625 : 1000000;


	fd = tfs_open( pid, "temp.dat", TFS_O_WRONLY | TFS_O_CREAT, 0666 );
	if (fd < 0)
	{
		printf( "Failed to open \"temp.dat\". TFS error = %d.\n",
		 tfs_errno );

		goto err_exit;
	}

	start = ProcTime();

	for (i = 0; i < times; i++)
	{
		if (tfs_write( pid, fd, buffer, size ) != size)
		{
			printf( "Failed to write data, TFS error = %d.\n",
				tfs_errno );

			goto err_close;
		}
	}

	end = ProcTime();

	interval = ProcTimeMinus( end, start );
	kb = (size*times)/1024;

	printf( "Write: %.1f seconds for %d Kbytes, using %d byte blocks, ",
		(float)interval/(float)second, kb, size);
	printf( "which is %.2f Kb/s.\n", (float)(kb)*(float)(second)/(float)interval );

	if (tfs_close( pid, fd ))
	{
		printf( "Failed to close \"temp.dat\". TFS error = %d.\n",
		 tfs_errno );

		 goto err_exit;
	}

	tfs_sync();

	fd = tfs_open( pid, "temp.dat", TFS_O_RDONLY, 0666 );
	if (fd < 0)
	{
		printf( "Failed to open \"temp.dat\". TFS error = %d.\n",
			 tfs_errno );

		goto err_exit;
	}

	start = ProcTime();

	for (i = 0; i < times; i++)
	{
		if (tfs_read( pid, fd, buffer, size ) != size)
		{
			printf( "Failed to read data, TFS error = %d.\n",
				tfs_errno );

			goto err_close;
		}
	}

	end = ProcTime();

	interval = ProcTimeMinus( end, start );
	kb = (size*times)/1024;

	printf( "Read: %.1f seconds for %d Kbytes, using %d byte blocks, ",
		(float)interval/(float)second, kb, size);
	printf( "which is %.2f Kb/s.\n", (float)(kb)*(float)(second)/(float)interval );


err_close:
	if (tfs_close( pid, fd ))
		printf( "Failed to close \"temp.dat\". TFS error = %d.\n",
		 tfs_errno );

err_exit:
	free(buffer);
}

