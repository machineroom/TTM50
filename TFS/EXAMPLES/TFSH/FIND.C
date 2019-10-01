
#include <stdio.h>
#include <iocntrl.h>
#include <string.h>
#include <assert.h>
#include "tfs.h"
#include "tfs_util.h"

static int type_mask;
static int print;

int _find(char *path);

int find_dir(char *dir_path)
{
	int fd = tfs_open(pid, dir_path, TFS_O_RDONLY, 0);
	char entry_path[TFS_PATH_MAX+1];
	struct tfs_dir_t  dir;

	if (fd == -1)
	{
		fprintf(stderr, "find: \"%s\": open failed: %s\n",
			dir_path,
			tfs_errlist[tfs_geterr(pid)]);
		return 0;
	}

	while ( tfs_read(pid, fd, (char*)&dir, sizeof(dir)) == sizeof(dir))
	{
		if (dir.inode_no != 0 &&
			strcmp(dir.name, ".")!=0 &&
			strcmp(dir.name, "..")!=0 )
		{
			strcpy(entry_path, dir_path);
			strcat(entry_path, "/");
			strcat(entry_path, dir.name);

			_find(entry_path);
		}
	}

	tfs_close(pid,fd);

	return 0;
}


int _find(char *path)
{
	struct tfs_stat_t st;

	if (tfs_stat(pid, path, &st) == -1)
	{
		fprintf(stderr, "find: \"%s\": stat failed: %s\n",
			path, tfs_errlist[tfs_geterr(pid)]);
		return 0;
	}

	if (st.st_mode & type_mask)
	{
		if (print)
			printf("%s\n", path);
	}

	if (st.st_mode & TFS_I_DIR)
	{
		find_dir(path);
	}

	return 0;
}

	

void find (struct command *command, int argc, char **argv)
{
	int i,j;

	type_mask = TFS_I_DIR | TFS_I_REG;
	print = 0;

	for (i=2; i<argc; i++)
	{
		if (strcmp(argv[i], "-type")==0)
		{
			if (i==argc-1)
				goto use_err;

			if (*argv[i+1] == 'f')
				type_mask = TFS_I_REG;
			else if (*argv[i+1] == 'd')
				type_mask = TFS_I_DIR;
			else
				goto use_err;
		}

		if (strcmp(argv[i], "-print")==0)
		{
			print = 1;
		}
	}

	/* find the first option argument */

	for (j=1; j<argc; j++)
	{
		if (*argv[j] == '-')
			break;
	}

	for (i=1; i<j; i++)
	{
		_find(argv[i]);
	}

	return;

use_err:
	usage (command);
}

