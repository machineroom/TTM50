#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <misc.h>
#include <channel.h>
#include <process.h>
#include <iocntrl.h>
#include <time.h>
#include "tfs.h"
#include "tfs_util.h"


int pid = -1;
char cwd[TFS_PATH_MAX+1] = "not mounted";

void usage( struct command *command )
{
	if (command->usage)
		printf( "Usage: %s %s\n", command->name, command->usage );
	else
		printf( "Usage: %s\n", command->name );
}


static void listen( Process *p, Channel *in )
{
	int length;
	char data[256];

	p = p;

	while (TRUE)
	{
		length = ChanInInt( in );
		if ((length < 0) || (length > sizeof( data )))
		{
			printf( "Invalid debug message of length %d.\n",
			 length );
			exit_terminate( EXIT_FAILURE );
		}

		ChanIn( in, data, length );
		printf( "%s", data );
	}
}


struct command Command[] =
{
	"format", NULL, format,
	"mkfs", "<inodes>", mkfs,
	"mount", "[ fsck ]", mount,
	"sync", "[ <blocks> ]", sync,
	"logon", "<uid> <gid>", logon,
	"logoff", NULL, logoff,
	"mkdir", "<name> <perms>", mkdir,
	"cd", "<path>", cd,
	"pwd", NULL, pwd,
	"ls", "[ <path> ... ]", ls,
	"download", "<hostfile> <tfs_file>", download,
	"upload", "<tfs_file> <hostfile>", upload,
	"rm", "[ -r ] [ <path> ... ]", rm,
	"rmdir", "[ <path> ... ]", rmdir,
	"source", "<command_file>", source,
	"ln", "<filename> <linkname>", ln,
	"mv", "<oldname> <newname> OR [ <files> ... ] <dir>", mv,
	"cp", "[ -r ] <name> <newname> OR [ <files> ... ] <dir>", cp,
	"du", "[ -s ] [ <dirs> ... ]", du,
	"chmod", " <mode> [ <files> ... ]", chmod,
	"find", "[ <files> ... ] [ -type f|d ] -print", find,
	"tar", "c|x[v][f] [ <files> ... ]", tar,
	"df", NULL, df,
	"benchmark", "<blocksize> <blocks>", benchmark,
	"offline", NULL, offline,
	NULL, NULL, NULL
};



int tfsh(int argc, char *argv[])
{
	int i;
	int uid, gid;
	char *p;
	int rv;

	p = getenv("TFS_UID");
	uid = (p) ? atoi(p) : -2;

	p = getenv("TFS_GID");
	gid = (p) ? atoi(p) : -2;

	DB(("uid=%d, gid=%d\n", uid, gid));

	if (argc == 1 || strcmp(argv[1],"-n")!=0)
	{
		/* mount and logon */
		rv = tfs_mount(TFS_DONT_FSCK);

		if (rv != 0)
		{
			tfs_perror(-1, "tfsh: mount failed");
			return -1;
		}

		strcpy(cwd, "not logged on");

		pid = tfs_logon(uid, gid);

		if (pid == -1)
		{
			tfs_perror(-1, "tfsh: logon failed");
		}
		else
		{
			p = getenv("TFS_HOME");
			if (p==NULL)
			{
				p="/";
			}

			tfs_chdir(pid, p);

			getcwd(cwd);
		}

		if (argc > 1)
		{
			return tfsh_exec_command_line(argc-1, argv+1);
		}
		else
		{
			tfsh_interactive();
		}
	}
	else
	{
		/* argv[1] == "-n" */

		if (argc > 2)
		{
			return tfsh_exec_command_line(argc-2, argv+2);
		}
		else
		{
			tfsh_interactive();
		}
	}

	return 0;
}


void logon( struct command *command, int argc, char **argv)
{
	int uid;
	int gid;

	if (argc != 3)
	{
		usage( command );
		return;
	}

	uid = atoi(argv[1]);
	gid = atoi(argv[2]);

	pid = tfs_logon( (u_short) uid, (u_short) gid );
	if (pid < 0)
		printf( "logon: TFS error %d.\n", tfs_errno );
	else
	{
		getcwd(cwd);
	}
}

void logoff( struct command *command, int argc, char **argv)
{
	command = command;

	if (tfs_logoff( pid ))
		printf( "logoff: TFS error %d.\n", tfs_errno );

	pid = -1;
	strcpy(cwd, "not logged on");
}


void offline( struct command *command, int argc, char **argv)
{
	tfs_offline();
	exit_terminate(0);
}


void mount( struct command *command, int argc, char **argv)
{
	int do_fsck;

	if (argc == 1)
	{
		do_fsck = TFS_DONT_FSCK;
	}
	else if (strcmp( "fsck", argv[1] ) == 0)
	{
		do_fsck = TFS_DO_FSCK;
	}
	else
	{
		usage( command );
		return;
	}

	if (tfs_mount( do_fsck ))
	{
		tfs_perror(-1,"mount");
		return;
	}

	strcpy(cwd, "not logged on");
}


void format( struct command *command, int argc, char **argv)
{
	command = command;

	printf( "Formatting ...\n" );
	if (tfs_format())
		tfs_perror(-1,"format");

}

void mkfs( struct command *command, int argc, char **argv)
{
	int inodes;

	if (argc != 2)
	{
		usage( command );
		return;
	}

	inodes = atoi(argv[1]);

	if (inodes <= 0)
	{
		printf("Illegal number of inodes\n");
		usage( command );
		return;
	}


	if (tfs_mkfs( inodes ))
		tfs_perror(-1,"mkfs");
}

int main(int argc, char *argv[])
{
	Channel *chan;
	Process *proc;

	chan = ChanAlloc();
	if (chan == NULL)
	{
		printf( "Failed to allocate debug channel.\n" );
		exit_terminate( EXIT_FAILURE );
	}

	proc = ProcAlloc( listen, 0, 1, chan );
	if (proc == NULL)
	{
		printf( "Failed to allocate debug process.\n" );
		exit_terminate( EXIT_FAILURE );
	}

	ProcRun( proc );

	if (tfs_init( 0, 0, 2, TFS_WRITE_BACK,
			time(NULL), chan, TFS_ERROR_MODE_ON ))
	{
		printf( "Failed to initialise TFS. Error = %d.\n", tfs_errno );
		ProcWait( 1562 );
		exit_terminate( EXIT_FAILURE );
	}

	tfsh(argc, argv);

	tfs_terminate( TFS_DONT_FSCK );
	exit_terminate(0);
}


