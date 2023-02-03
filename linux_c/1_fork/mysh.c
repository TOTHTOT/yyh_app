#include <unistd.h>
#include "stdlib.h"
#include "stdio.h"
#include <glob.h>
#include <string.h>

// #define DELIMS " \t\n"

struct cmd_st
{
	glob_t globres;
};

void prompt(void)
{
	printf("mysh-0.1 $ ");
}

static void parse(char *line, struct cmd_st *res)
{
	char *tok;
	int i = 0;
	char * const DELIMS = " \t\n";
	struct cmd_st cmd;

	
	while(1)
	{
		tok = strsep(&line, DELIMS);
		if (tok == NULL)
		{
			break;
		}
		if(tok[0] == '\0')
		{
			continue;
		}
		glob(tok, GLOB_NOCHECK|GLOB_APPEND*i, NULL, &res->globres);
		i = 1;
	}
}


int main(void)
{
	pid_t pid;
	char *linebuf = NULL;
	size_t linebuf_size = 0;
	struct cmd_st cmd;
	
	while(1)
	{
		prompt();
		// 获取行
		if(getline(&linebuf, &linebuf_size, stdin) < 0)
		{
		
		}
		//解析行内容
		parse(linebuf, &cmd);
		
		if(0) // 内部命令
		{
		}
		else // 外部命令
		{
			pid = fork();
			if (pid < 0)
			{
				perror("fork err");
				return -1;
			}
			if(pid == 0)
			{
				execvp(cmd.globres.gl_pathv[0], cmd.globres.gl_pathv);
				perror("execvp err");
				exit (-1);
			}
			else
			{
				wait(NULL);
			}
		}
	}
	
	return 0;
}
