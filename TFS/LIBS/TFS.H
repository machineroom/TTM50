#ifndef _tfs_h
#define _tfs_h

#ifndef  __time_h
#  include  <time.h>
#endif
#ifndef  _channel_h
#  include  <channel.h>
#endif

#define  TFS_CACHE_SIZE		128	/* Number of blocks in the cache. */
#define  TFS_DEFAULT_UMASK	0000022   /* Default for new processes. */
#define  TFS_WRITE_THROUGH	8
#define  TFS_WRITE_BACK		9
#define  TFS_ERROR_MODE_ON	10
#define  TFS_ERROR_MODE_OFF	11
#define  TFS_DO_FSCK		(-1)
#define  TFS_DONT_FSCK		0
#define  TFS_NAME_MAX		27	/* Max path component length. */
#define  TFS_PATH_MAX		255	/* Max total path name length. */
#define  TFS_LINK_MAX		8	/* Std POSIX value. */
#define  TFS_OPEN_MAX		16	/* Entries in each process' fd-table. */
#define  TFS_ROOT_UID		0
#define  TFS_ROOT_GID		1

#define  TFS_SEEK_SET		0
#define  TFS_SEEK_CUR		1
#define  TFS_SEEK_END		2

#define  TFS_O_RDONLY		1
#define  TFS_O_WRONLY		2
#define  TFS_O_RDWR		3
#define  TFS_O_CREAT		0x0200
#define  TFS_O_TRUNC		0x0400
#define  TFS_O_EXCL		0x0800
#define  TFS_O_APPEND		8

#define  TFS_F_OK		0	/* ACCESS: File exists? */
#define  TFS_X_OK		1
#define  TFS_W_OK		2
#define  TFS_R_OK	        4

#define  TFS_I_TYPE	0170000		/* The inode type field. */
#define  TFS_I_REG	0100000		/* Regular file, not dir. */
#define  TFS_I_DIR	0040000		/* File id a directory. */
#define  TFS_S_PERMS	0006777		/* The PERM field. */
#define  TFS_S_IRWXU	0000700		/* Owner (User) perms mask. */
#define  TFS_S_IRUSR	0000400
#define  TFS_S_IWUSR	0000200
#define  TFS_S_IXUSR	0000100
#define  TFS_S_IRWXG	0000070		/* Group perms mask. */
#define  TFS_S_IRGRP	0000040
#define  TFS_S_IWGRP	0000020
#define  TFS_S_IXGRP	0000010
#define  TFS_S_IRWXO	0000007		/* Other perms mask. */
#define  TFS_S_IROTH	0000004
#define  TFS_S_IWOTH	0000002
#define  TFS_S_IXOTH	0000001
#define  TFS_S_ISUID	0004000		/* The SUID and SGID bits are not */
#define  TFS_S_ISGID	0002000 	/* used by TFS, but for a regular file
					   they WILL be cleared on any write
					   in accordance with POSIX. */
					   
#define  TFS_MAX_PROCS		256	/* Max nof. processes registered. */

typedef  unsigned short		u_short;

struct tfs_dir_t
{  unsigned	inode_no;
   char		name[TFS_NAME_MAX+1];    /* Make room for \0. */
};

typedef  enum
{  TFS_OK,
   TFS_EIO,			/* 1  Physical error, or inconsistency. */
   TFS_ENOSPC,			/* 2  No more data blocks or inodes. */
   TFS_ENOTDIR,			/* 3  Part of path is not directory. */
   TFS_EAGAIN,			/* 4  Resource currently unavailable. */
   TFS_EEXIST,			/* 5  File name already exists. */
   TFS_EFBIG,			/* 6  File (or dir) will become too big. */
   TFS_ENOENT,			/* 7  Part of path does not exist. */
   TFS_EISDIR,			/* 8  Operation not allowed on directory. */
   TFS_EACCES,			/* 9  Permission denied (rwx-bits). */
   TFS_EPERM,			/* 10 Permission denied (not rwx related). */
   TFS_EBADF,			/* 11 Bad file descriptor. */
   TFS_EINVAL,			/* 12 Invalid parameter. */
   TFS_ENFILE,			/* 13 Total limit on open files reached. */
   TFS_ENAMETOOLONG,		/* 14 Total path or component too long. */
   TFS_EMFILE,			/* 15 No more fd's for this process. */
   TFS_EMLINK,			/* 16 No more links for this inode. */
   TFS_ENOTEMPTY,		/* 17 Dir not empty. */
   TFS_EBUSY			/* 18 Device already in use. */
}  fs_error_type;

struct tfs_stat_t
{  u_short		st_mode;
   unsigned		st_ino;
   unsigned		st_dev;
   u_short		st_nlink;
   u_short		st_uid;
   u_short		st_gid;
   unsigned		st_size;
   time_t		st_mtime;
};


struct tfs_statfs_t
{
	long    f_type;     /* type of info, zero for now */
	long    f_bsize;    /* fundamental file system block size */
	long    f_blocks;   /* total blocks in file system */
	long    f_bfree;    /* free blocks */
	long    f_bavail;   /* free blocks available to non-super-user */
	long    f_files;    /* total file nodes in file system */
	long    f_ffree;    /* free file nodes in fs */
	long    f_fsid;     /* file system id */
};

extern  int  tfs_errno;
extern  int  tfs_init(		unsigned	fs_id,
				int		scsi_target_id,
				int		link_no,
				int		cache_mode,
				time_t		sys_time,
				Channel		*err_c,
				int		err_m );

extern  int  tfs_terminate( 	int		do_fsck );

extern  int  tfs_offline( 	void );

extern  int  tfs_mkfs( 		unsigned 	inodes );

extern  int  tfs_mount( 	int		do_fsck );

extern  int  tfs_format( 	void );

extern  int  tfs_sync( 		void );

extern  int  tfs_part_sync( 	int 	blocks );

extern  int  tfs_logon( 	u_short 	uid, 
				u_short 	gid );

extern  int  tfs_logoff( 	int 	pid );

extern  int  tfs_chdir( 	int 		pid, 
				char 		*path );

extern  u_short  tfs_umask( 	int 		pid, 
				u_short 	new_mask );

extern  int  tfs_open( 		int 		pid, 
				char 		*file_name, 
				int 		mode,
		       		u_short 	c_perm );

extern  int  tfs_dup( 		int 		pid, 
				int 		old_fd );

extern  int  tfs_close( 	int 		pid, 
				int 		fd );

extern  long  tfs_lseek( 	int 		pid, 
				int 		fd, 
				long 		offset, 
			 	int 		whence );

extern  int  tfs_read( 		int 		pid, 
				int 		fd, 
				char 		*buffer, 
		       		int 		nof_bytes );

extern  int  tfs_write( 	int 		pid, 
				int 		fd, 
				char 		*buffer, 
				int 		nof_bytes );

extern  int  tfs_access( 	int 		pid, 
				char 		*path, 
				int 		aperm );

extern  int  tfs_chmod( 	int 		pid, 
				char 		*path, 
				int 		perm );

extern  int  tfs_chown( 	int 		pid, 
				char 		*path, 
				u_short 	owner,
				u_short 	group );

extern  int  tfs_utime( 	int 		pid, 
				char 		*path, 
				time_t 		*tim );

extern  int  tfs_stat( 		int 		pid, 
				char 		*path, 
		       		struct tfs_stat_t *sbuf );

extern  int  tfs_fstat( 	int 		pid, 
				int 		fd, 
				struct tfs_stat_t *sbuf );

extern  int  tfs_statfs( 	int 		pid, 
				struct tfs_statfs_t *buf );

extern  int  tfs_mkdir( 	int 		pid, 
				char 		*path, 
				u_short 	perms );

extern  int  tfs_rmdir( 	int 		pid, 
				char 		*path );

extern  int  tfs_link( 		int 		pid, 
				char 		*path1, 
				char 		*path2 );

extern  int  tfs_unlink( 	int 		pid, 
				char 		*path );

extern int tfs_geterr(		int pid );			

extern void tfs_perror ( int pid, char *prefix );

extern int tfs_nerr;

extern char *tfs_errlist[];


/* client operation only */

extern int tfs_connect (Channel *to_tfs, Channel *from_tfs,	
			int uid, int gid, int authorty);

extern int tfs_disconnect (int pid);

#endif

