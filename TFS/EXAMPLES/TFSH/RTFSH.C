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



struct command Command[] =
{
	"sync", "[ <blocks> ]", sync,
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
	NULL, NULL, NULL
};



int main(int argc, char *argv[])
{
	int uid, gid, auth;
	char *p;
	Channel *ttfs, *ftfs;
	
	ftfs = (Channel*) get_param(3);
	ttfs = (Channel*) get_param(4);

	if (ftfs==NULL || ttfs==NULL)
	{
		fprintf(stderr, "rtfsh: bad configuration interface\n");
	}

	p = getenv("TFS_UID");
	uid = (p) ? atoi(p) : -2;

	p = getenv("TFS_GID");
	gid = (p) ? atoi(p) : -2;

	p = getenv("TFS_AUTH");
	auth = (p) ? atoi(p) : -1;

	DB(("uid=%d, gid=%d\n", uid, gid));

	/* connect */

	pid = tfs_connect(ttfs, ftfs, uid, gid, auth);

	if (pid == -1)
	{
		tfs_perror(-1, "tfsh: connect failed");
		exit_terminate(-1);
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
		tfsh_exec_command_line(argc-1, argv+1);
	}
	else
	{
		tfsh_interactive();
	}

	tfs_disconnect(pid);

	exit_terminate(0);
}






