
#define TRUE 1
#define FALSE 0


struct command
{
	char *name;
	char *usage;
	void (*function)(struct command *com, int argc, char **argv);
};

#define BLOCK_SIZE (16*1024)

extern char buffer[BLOCK_SIZE];
extern int pid;
extern char cwd[];

int isdir(char*path);
int getcwd(char* cwd);
void call_command (struct command *com, char *line);
void usage( struct command *command );

void ls(struct command *com, int argc, char **argv);
void download( struct command *command, int argc, char **argv);
void upload( struct command *command, int argc, char **argv);
void source( struct command *command, int argc, char **argv);
void ln (struct command *command, int argc, char **argv);
void mv (struct command *command, int argc, char **argv);
void rm (struct command *command, int argc, char **argv);
void cp (struct command *command, int argc, char **argv);
void du (struct command *command, int argc, char **argv);
void df (struct command *command, int argc, char **argv);
void chmod (struct command *command, int argc, char **argv);
void find (struct command *command, int argc, char **argv);
void cd (struct command *command, int argc, char **argv);
void pwd (struct command *command, int argc, char **argv);
void mkdir (struct command *command, int argc, char **argv);
void rmdir (struct command *command, int argc, char **argv);
void format (struct command *command, int argc, char **argv);
void mkfs (struct command *command, int argc, char **argv);
void mount (struct command *command, int argc, char **argv);
void sync (struct command *command, int argc, char **argv);
void logon (struct command *command, int argc, char **argv);
void logoff (struct command *command, int argc, char **argv);
void tar (struct command *command, int argc, char **argv);
void benchmark (struct command *command, int argc, char **argv);
void offline (struct command *command, int argc, char **argv);

extern struct command Command[];

int tfsh_exec_line(char*line);
int tfsh_exec_command_line(int argc, char *argv[]);
void tfsh_interactive(void);
int tfsh(int argc, char *argv[]);

#ifdef DEBUG
#define DB(x) printf x
#else
#define DB(x)
#endif

#ifndef NDEBUG
#define DBG(x) printf x
#else
#define DBG(x)
#endif
