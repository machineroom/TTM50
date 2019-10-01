/* @(#)fulltest.c	1.2 */

#include  <stdlib.h>
#include  <channel.h>
#include  <process.h>
#include  <stdio.h>
#include  <time.h>
#include <misc.h>
#include <string.h>
#include <assert.h>
#include  "tfs.h"

int pid = -1;
int serious_errs;
int minor_errs;
char *home = "/test_dir";

#define  FS_ID		117	/* Anything, realy... */
#define  UID		4
#define  GID		10
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


void fatal_error(char *msg)
{
	fprintf(stderr, "FATAL ERROR: %s: ", msg);
	fflush(stderr);
	tfs_perror(pid, NULL);

	tfs_terminate(TFS_DO_FSCK);
	exit_terminate(-1);
}


void serious_error(char *msg)
{
	fprintf(stderr, "SERIOUS ERROR: %s: ", msg);
	fflush(stderr);
	tfs_perror(pid, NULL);

	serious_errs++;
}

void minor_error(char *msg)
{
	fprintf(stderr, "Minor error: %s: ", msg);
	fflush(stderr);
	tfs_perror(pid, NULL);
	minor_errs++;
}

void test_umask()
{
	int rv;
	int fd;
	char *name = "Umask_test_file";
	struct tfs_stat_t st;
	u_short old_mask;

	printf("test_umask\n");

	tfs_unlink(pid, name);

	fd = tfs_open(pid, name, TFS_O_RDWR | TFS_O_CREAT | TFS_O_TRUNC,
			0777);
	if (fd == -1)
	{
		serious_error("open failed");
		return;
	}

	rv = tfs_close(pid, fd);
	if (rv != 0)
	{
		serious_error("close");
		return;
	}

	old_mask = tfs_umask(pid, 7); /* switch off "other" permission */
	if (tfs_errno != 0)
	{
		serious_error("umask");
		return;
	}

	rv = tfs_stat(pid, name, &st);
	if (rv == -1)
	{
		serious_error("stat failed");
		return;
	}

	if ((st.st_mode & 0777) != ((~old_mask) & 0777))
	{
		printf("st_mode=%o, ~old_mask=%o\n", (st.st_mode & 0777),
			((~old_mask) & 0777) );
		serious_error("file has wrong mode (old_mask)");
	}

	rv = tfs_chmod(pid, name, 0700);
	if (rv != 0)
	{
		serious_error("chmod");
	}

	rv = tfs_stat(pid, name, &st);
	if (rv != 0)
	{
		serious_error("stat 2");
	}

	if ((st.st_mode & 0777) != 0700)
	{
		serious_error("file has wrong mode (0700)");
	}

	rv = tfs_unlink(pid, name);
	if (rv != 0)
	{
		serious_error("unlink");
	}

	rv = tfs_access(pid, name, TFS_F_OK);
	if (rv != -1)
	{
		serious_error("access failed");
	}

	fd = tfs_open(pid, name, TFS_O_RDWR | TFS_O_CREAT | TFS_O_TRUNC,
				0777);
	if (fd == -1)
	{
		serious_error("open 2 failed");
	}
	
	rv = tfs_close(pid, fd);
	if (rv != 0)
	{
		serious_error("close 2");
	}

	rv = tfs_stat(pid, name, &st); 
	if (rv != 0)
	{
		serious_error("stat 3");
	}

	if ((st.st_mode & 0777) != 0770)
	{
		serious_error("file has wrong mode (0770)\n");
	}
}


void test_create()
{
	int rv;
	int fd;
	char *name = "test_dir/test_file";
	struct tfs_stat_t st;

	printf("mkdir/open/close...\n");

	rv = tfs_mkdir(pid, "test_dir", 0777);
	if (rv == -1 && tfs_errno != TFS_EEXIST)
	{
		serious_error("mkdir failed");
		return;
	}

	fd = tfs_open(pid, name, TFS_O_RDWR | TFS_O_CREAT | TFS_O_TRUNC,
			0777);
	if (fd == -1)
	{
		serious_error("open failed");
		return;
	}


	if (strcmp(name, "test_dir/test_file") != 0)
	{
		minor_error("path name was corrupted\n");
	}

	rv = tfs_write(pid, fd, "hello\n", 6);
	if (rv != 6)
	{
		serious_error("write failed");
		return;
	}

	rv = tfs_close(pid, fd);
	if (rv == -1)
	{
		minor_error("close failed"); 
	}

	rv = tfs_stat(pid, "test_dir/test_file", &st);
	if (rv == -1)
	{
		serious_error("stat failed");
		return;
	}


	if (st.st_size != 6)
	{
		serious_error("file has wrong size\n");
	}
}



	
#define SIZE (100*1024)
#define BLOCK  1000

char buf_orig [SIZE];
char buf_cmp [SIZE];
char *file = "test.file";

int random(int limit)
{
	unsigned int r = rand();

	r = (r & 0x000fffff) % limit;

	return (int)r;
}


void tst_seq_write_rand_read()
{
	int fd;
	int i;
	int count;
	int pos;
	int rv;
	int *ip;

	printf ("tst_seq_write_rand_read...\n");

	fd = tfs_open (pid, file, TFS_O_RDWR | TFS_O_TRUNC | TFS_O_CREAT, 0777);

	if (fd == -1)
	{
		serious_error("tst_seq_write_rand_read: open failed");
		return;
	}


	ip = (int*)buf_orig;

	for (i=0; i<SIZE / 4; i++)
	{
		*(ip++) = i;
	}

	rv = tfs_lseek(pid, fd, 0, TFS_SEEK_SET);
	if(rv != 0)
	{
		serious_error("lseek failed");
		return;
	}
		

	rv = tfs_write (pid, fd, buf_orig, SIZE);
	if (rv != SIZE)
	{
		serious_error("write failed");
		return;
	}


	for (count=0; count<100; count++)
	{
		pos = random(SIZE-BLOCK);

		rv = tfs_lseek (pid, fd, pos, TFS_SEEK_SET);
		if (rv != pos)
		{
			serious_error("lseek failed");
			return;
		}

		rv = tfs_read (pid, fd, buf_cmp, BLOCK);
		if (rv != BLOCK)
		{
			serious_error("read failed");
			return;
		}

		for (i=0; i<BLOCK; i++)
		{
			if (buf_cmp[i] != buf_orig[pos+i])
			{
				fprintf (stderr, "error at %d+%d: found 0x%x, expected 0x%x\n",
				pos,i, buf_cmp[i], buf_orig[pos+i]);
				serious_error("");
			}
		}
	}

	rv = tfs_close(pid, fd);
	if (rv != 0)
	{
		serious_error("close");
	}

	fd = tfs_open (pid, file, TFS_O_RDONLY, 0);

	for (count=0; count<1000; count++)
	{
		pos = random(SIZE-BLOCK);

		rv = tfs_lseek (pid, fd, pos, TFS_SEEK_SET);
		assert(rv == pos);

		rv = tfs_read (pid, fd, buf_cmp, BLOCK);
		if (rv != BLOCK)
		{
			printf("read(%d, addr, %d) returned %d, pos=%d\n",
				fd, BLOCK, rv, pos);
			assert(rv == BLOCK);
		}

		for (i=0; i<BLOCK; i++)
		{
			if (buf_cmp[i] != buf_orig[pos+i])
			{
				printf ("error at %d+%d: found 0x%x, expected 0x%x\n",
				pos,i, buf_cmp[i], buf_orig[pos+i]);
				assert(0);
			}
		}
	}

	rv = tfs_close(pid, fd);
	assert(rv == 0);
}

void tst_rand_write_seq_read()
{
	int fd;
	int i;
	int count;
	int pos;
	int rv;

	printf("tst_rand_write_seq_read...\n");

	fd = tfs_open (pid, file, TFS_O_RDWR | TFS_O_TRUNC |
					TFS_O_CREAT, 0777);

	if (fd == -1)
	{
		serious_error("open failed");
		return;
	}


	for (i=0; i<SIZE; i++)
	{
		buf_orig[i] = rand();
		buf_cmp[i] = 0;
	}

	for (count=0; count<1000; count++)
	{
		pos = random(SIZE-BLOCK);

		tfs_lseek (pid, fd, pos, TFS_SEEK_SET);

		tfs_write (pid, fd, buf_orig+pos, BLOCK);

		for (i=0; i<BLOCK; i++)
		{
			buf_cmp[pos+i] = 1;
		}
	}

	for (i=0; i<SIZE; i++)
	{
		for (count=0; i+count < SIZE; count++)
		{
			if (buf_cmp[i+count] != 0)
				break;
		}

		if (count > 0)
		{
			tfs_lseek (pid, fd, i, TFS_SEEK_SET);
			tfs_write(pid, fd, buf_orig+i, count);
		}
	}

	tfs_close(pid, fd);

	fd = tfs_open (pid, file, TFS_O_RDONLY, 0);


	rv = tfs_read(pid, fd, buf_cmp, SIZE);

	if (rv != SIZE)
	{
		printf("read(fd, buf_cmp, %d) = %d\n",
			SIZE, rv);

		serious_error("read");
	}

	for (i=0; i<SIZE; i++)
	{
		if (buf_cmp[i] != buf_orig[i])
		{
			printf ("error at %d, found %d, expected %d\n",
				i, buf_cmp[i], buf_orig[i]);
			assert(0);
		}
	}

	tfs_close(pid, fd);
}


void overwrite_tst()
{
	int fd;
	int i;
	int count;
	int pos;
	int opos;
	int rv;
	int j;

	printf("overwrite_tst...\n");

	fd = tfs_open (pid, file, TFS_O_RDWR | TFS_O_TRUNC |
				TFS_O_CREAT, 0777);
	assert(fd != -1);


	for (i=0; i<SIZE; i++)
	{
		buf_orig[i] = 0;
	}

	rv = tfs_lseek(pid, fd, 0, TFS_SEEK_SET);
	assert(rv == 0);

	rv = tfs_write (pid, fd, buf_orig, SIZE);
	assert(rv == SIZE);

	printf ("original file written...\n");

	for (count=0; count<1000; count++)
	{
		
		pos = random(SIZE-BLOCK);

		for (j=0; j<BLOCK; j++)
		{
			buf_cmp[j] = count;
			buf_orig[pos+j] = count;
		}


		rv = tfs_lseek (pid, fd, pos, TFS_SEEK_SET);
		if(rv != pos)
		{
			serious_error("lseek");
			return;
		}

		rv = tfs_write (pid, fd, buf_cmp, BLOCK);
		assert(rv == BLOCK);

		opos = pos;
		pos -= 100;
		if (pos < 0) pos = 0;

		rv = tfs_lseek (pid, fd, pos, TFS_SEEK_SET);
		assert(rv == pos);

		rv = tfs_read (pid, fd, buf_cmp, BLOCK);
		if (rv != BLOCK)
		{
			printf("lseek(df, %d, L_SET) = %d\n",
				pos, pos);
			printf("read (fd, buf_cmp, %d) = %d\n",
				BLOCK, rv);
		}
		assert (rv == BLOCK);


		for (j=0; j<BLOCK; j++)
		{
			if (buf_cmp[j] != buf_orig[pos+j])
			{
				printf ("error at %d: found %d, expected %d\n",
					pos+j, buf_cmp[j], buf_orig[pos+j]);
				printf("writing %d at %d\n", count & 0xff, opos);
				printf("checking at %d\n", pos);
				assert(0);
			}
		}

	}

	printf ("block by block test complete...\n");

	rv = tfs_lseek(pid, fd, 0, TFS_SEEK_SET);
	assert(rv == 0);

	rv = tfs_read (pid, fd, buf_cmp, SIZE);
	assert(rv == SIZE);

	for (j=0; j<SIZE; j++)
	{
		assert(buf_cmp[j] == buf_orig[j]);
	}

	tfs_close(pid, fd);
}

			






void test_dir()
{
	int rv;
	char name[TFS_NAME_MAX];
	int fd;
	int i;
	struct tfs_stat_t st;

	printf("chdir/big directory/read directory...\n");

	rv = tfs_chdir(pid, "test_dir");
	if (rv != 0)
	{
		serious_error("chdir failed");
		return;
	}

	for (i=0; i<300; i++)
	{
		sprintf(name, "file.%d", i);

		fd = tfs_open(pid, name, 
			TFS_O_RDWR | TFS_O_CREAT, 0777);
		if (fd == -1)
		{
			serious_error("open failed"); 
			return;
		}

		rv = tfs_close(pid, fd);
		if (rv != 0)
		{
			serious_error("close failed");
			return;
		}
	}

	rv = tfs_stat(pid, ".", &st);
	if (rv == -1)
	{
		serious_error("stat failed");
		return;
	}


	if (!(st.st_mode & TFS_I_DIR))
	{
		serious_error("directory mode bit");
		return;
	}

	if (st.st_size < 303 * sizeof(struct tfs_dir_t))
	{
		minor_error("directory has wrong size");
	}
}



void test_link()
{
	int rv;
	int fd;
	struct tfs_stat_t st;

	printf("test link...\n");

	fd = tfs_open (pid, "link.test", TFS_O_RDWR | TFS_O_TRUNC |
					TFS_O_CREAT, 0777);
	
	if (fd == -1)
	{
		serious_error("open");
		return;
	}

	tfs_close(pid, fd);

	rv = tfs_link(pid, "link.test", "link2.test");
	if (rv != 0)
	{
		serious_error("link");
		return;
	}

	rv = tfs_stat(pid, "link.test", &st);
	if (rv != 0)
	{
		serious_error("stat");
		return;
	}

	if (st.st_nlink != 2)
	{
		serious_error("wrong number of links");
	}

	rv = tfs_unlink(pid, "link2.test");
	if (rv != 0)
	{
		serious_error("unlink");
		return;
	}
}


void test_rmdir()
{
	int rv;
	static char name[256];

	printf ("test mkdir/rmdir\n");

	strcpy(name, "tmpdir");
	while (tfs_access(pid, name, TFS_F_OK)==0)
	{
		strcat(name,"A");
	}

	rv = tfs_mkdir(pid, name, 0777);
	if (rv != 0)
	{
		serious_error("mkdir");
		return;
	}

	rv = tfs_rmdir(pid, name);
	if (rv != 0)
	{
		serious_error("mkdir");
		return;
	}

	rv = tfs_access(pid, name, TFS_F_OK);
	if (rv != -1)
	{
		serious_error("file still exits");
	}
}


void printstat(struct tfs_stat_t *st)
{
	printf("\tst_mode = 0x%x\n", st->st_mode);
	printf("\tst_ino = 0x%x\n", st->st_ino);
	printf("\tst_dev = 0x%x\n", st->st_dev);
	printf("\tst_nlink = 0x%x\n", st->st_nlink);
	printf("\tst_uid = 0x%x\n", st->st_uid);
	printf("\tst_gid = 0x%x\n", st->st_gid);
	printf("\tst_size = 0x%x\n", st->st_size);
	printf("\tst_mtime = 0x%x\n", st->st_mtime);
}

int statcmp(struct tfs_stat_t *s1, struct tfs_stat_t *s2)
{
	if (s1->st_mode != s1->st_mode) return -1;
	if (s1->st_ino != s1->st_ino) return -1;
	if (s1->st_dev != s1->st_dev) return -1;
	if (s1->st_nlink != s1->st_nlink) return -1;
	if (s1->st_uid != s1->st_uid) return -1;
	if (s1->st_gid != s1->st_gid) return -1;
	if (s1->st_size != s1->st_size) return -1;
	if (s1->st_mtime != s1->st_mtime) return -1;
	return 0;
}

void test_fstat()
{
	int rv;
	struct tfs_stat_t st;
	struct tfs_stat_t fst;
	int fd;

	printf ("fstat...\n");

	fd = tfs_open (pid, "fstat.test", TFS_O_RDWR | TFS_O_TRUNC |
					TFS_O_CREAT, 0777);
	
	if (fd == -1)
	{
		serious_error("open");
		return;
	}

	rv = tfs_stat(pid, "fstat.test", &st);
	if (rv != 0)
	{
		serious_error("stat");
		return;
	}

	rv = tfs_fstat(pid, fd, &fst);
	if (rv != 0)
	{
		serious_error("fstat");
		return;
	}

	if (statcmp(&st, &fst) != 0)
	{
		printf("stat returned:\n");
		printstat(&st);
		printf("fstat returned:\n");
		printstat(&fst);
		serious_error("difference between stat and fstat");
	}
	
	tfs_close(pid, fd);
}



void test_bandwidth()
{
#define BLK (16*1024)
#define CNT 512

	int fd;
	int rv;
	int i;
	unsigned int t1, t2;
	float tick = ProcGetPriority() ? 64.0e-6 : 1.0e-6;

	printf ("test bandwidth\n");

	tfs_sync();

	fd = tfs_open (pid, "bandwidth.test", TFS_O_RDWR | TFS_O_TRUNC |
					TFS_O_CREAT, 0777);
	
	if (fd == -1)
	{
		serious_error("open trunc");
		return;
	}

	t1 = ProcTime();

	for (i=0; i<512; i++)
	{
		rv = tfs_write(pid, fd, buf_orig, BLK);
		if (rv != BLK)
		{
			serious_error("write");
			return;
		}
	}

	t2 = ProcTime();

	printf ("Write truncated file: %.2f K/s\n",
		((float)(BLK*CNT)/1024.0) / ((float)(t2-t1) * tick));
	
	tfs_close(pid, fd);

	tfs_sync();

	fd = tfs_open (pid, "bandwidth.test", TFS_O_RDWR, 0);
	if (fd == -1)
	{
		serious_error("open rdwr");
		return;
	}

	t1 = ProcTime();

	for (i=0; i<CNT; i++)
	{
		rv = tfs_write(pid, fd, buf_orig, BLK);
		if (rv != BLK)
		{
			serious_error("write");
			return;
		}
	}

	t2 = ProcTime();

	printf ("Write non-truncated file: %.2f K/s\n",
		((float)(CNT*BLK)/1024.0) / ((float)(t2-t1) * tick));


	tfs_lseek(pid, fd, 0, TFS_SEEK_SET);
	tfs_sync();


	t1 = ProcTime();

	for (i=0; i<CNT; i++)
	{
		rv = tfs_read(pid, fd, buf_orig, BLK);
		if (rv != BLK)
		{
			serious_error("read");
			return;
		}
	}

	t2 = ProcTime();

	printf ("read: %.2f K/s\n",
		((float)(BLK*CNT)/1024.0) / ((float)(t2-t1) * tick));

	tfs_close(pid, fd);
}




int main( )
{
	int             res;

	Process        *p;
	Channel *err_ch;

	printf("fulltest running\n");

	err_ch = ChanAlloc();
	p = ProcAlloc(err_mon, 0, 1, err_ch);
	ProcRun(p);

	/* Init disk etc. */
	printf("tfs_init...\n");
	res = tfs_init(FS_ID, SCSI_ID, LINK_NO, TFS_WRITE_BACK,
		       time(NULL), err_ch, TFS_ERROR_MODE_ON);
	if (res != 0)
	{
		tfs_perror(-1, "tfs_init() failed");
		exit_terminate(-1);
	}

	printf("tfs_mount...\n");
	res = tfs_mount(TFS_DO_FSCK);
	if (res != 0)
	{
		tfs_perror(-1, "tfs_mount() failed");
		tfs_terminate(TFS_DO_FSCK);
		exit_terminate(-1);
	}

	printf("tfs_logon...\n");
	pid = tfs_logon(UID, GID);
	if (pid == -1)
	{
		tfs_perror(-1, "tfs_logon() failed");
		tfs_terminate(TFS_DONT_FSCK);
		exit_terminate(-1);
	}



   overwrite_tst();
   tst_seq_write_rand_read();
   tst_rand_write_seq_read();

   test_create();
   test_dir();
   test_umask();
   test_link();
   test_rmdir();
   test_fstat();
   test_bandwidth();


   printf ("tests complete: %d serious errors, %d minor errors\n",
		serious_errs, minor_errs);

   tfs_logoff(pid);
   tfs_terminate(TFS_DONT_FSCK);
   exit_terminate(0);
}

