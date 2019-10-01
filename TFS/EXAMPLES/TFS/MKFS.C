/*
 * @(#)mkfs.c	1.1
 * 
 * Make a TFS file system.
 * 
 */

#include  "tfs.h"
#include  <stdio.h>
#include  <process.h>
#include  <time.h>
#include  <stdlib.h>
#include  <misc.h>

#define  FS_ID		117	/* Anything, realy... */
#define  LINK_NO	2
#define  SCSI_ID	0




static void 
err_mon(Process *p, Channel *ch)
{
	int             len;
	char            str[1000];

	while (1)
	{
		len = ChanInInt(ch);
		ChanIn(ch, str, len);
		fputs(str, stderr);
	}
}


int 
main(int argc, char *argv[])
{
	int             res;

	Channel *err_chan = ChanAlloc();
	Process        *p = ProcAlloc(err_mon, 0, 1, err_chan);

	ProcRunHigh(p);

	/* Init disk etc. */
	printf("tfs_init...\n");
	res = tfs_init(FS_ID, SCSI_ID, LINK_NO, TFS_WRITE_BACK,
		       time(NULL), err_chan, TFS_ERROR_MODE_ON);
	if (res != 0)
		tfs_perror(-1, "tfs_init() failed");

	printf("tfs_mkfs: making file system of 5000 inodes...\n");
	res = tfs_mkfs(5000);
	if (res != 0)
		tfs_perror(-1, "tfs_mkfs() failed");

	res = tfs_terminate(TFS_DO_FSCK);
	if (res != 0)
		tfs_perror(-1, "tfs_terminate() failed");

	printf("done\n");
	exit_terminate(EXIT_SUCCESS);
}
