#define 	_GNU_SOURCE
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<signal.h>
#include	<sys/wait.h>
#include	<string.h>

#include	"smsh.h"
#include	"splitline.h"
#include	"varlib.h"
#include	"process.h"
#include 	"controlflow.h"

/**
 **	small-shell version 5
 **		first really useful version after prompting shell
 **		this one parses the command line into strings
 **		uses fork, exec, wait, and ignores signals
 **
 **     hist: 2017-04-12: changed comment to say "version 5" 
 **/

#define	DFL_PROMPT	"> "
static char* curr_filename;
static char* temp_filename;

void	setup();

void save_last_result(int result) { 
	char *res = VLlookup("?");
	asprintf(&res, "%d", result);
	VLstore("?", res);
}

int execute_file(FILE *input, char *prompt) 
/*
 * Reads data from the input file stream and presents a prompt
 * if a source (.) is encountered, the function is called 
 * recursively with an empty prompt.
 */
{ 
	char	*cmdline, **arglist;
	int		result;
	FILE *	temp; /* hold a file stream if we source */
	int curr_line = 1;

	while ( (cmdline = next_cmd(prompt, input)) != NULL ){
		cmdline = substitute_variables(&cmdline); 

		if ( (arglist = splitline(cmdline)) != NULL  ){
			/* check for source as first argument */
			if ( arglist[0] && strcmp(arglist[0], ".") == 0 ) {
				temp = input;
				temp_filename = curr_filename;
				curr_filename = arglist[1];
				if ( (input = fopen(arglist[1], "r")) == NULL ) {
					perror("smsh");
					exit(1);
				} 
				result = execute_file(input, ""); /* execute subshell with current env */ 
				fclose(input);
				input = temp;
				curr_filename = temp_filename;
			} else {
				result = process(arglist);
				freelist(arglist); 
			} 
		}
		curr_line++;
		free(cmdline);
		save_last_result(result);
	}
	check_if_state(curr_filename, curr_line);
	return result;
}

int main(int argc, char ** argv)
{
	FILE *input = stdin;
	setup();
	char *prompt = DFL_PROMPT ;

	if (argc > 1) {
		prompt = "";
		curr_filename = argv[1];
		if ( (input = fopen(curr_filename, "r")) == NULL ) {
			perror("smsh");
			exit(1);
		}
	}

	if (argc > 2) {
		char key[10];
		for (int i = 2; i < argc; i++) {
			sprintf(key, "%d", i-1);
			VLstore(key, argv[i]);
		}
	}

	return execute_file(input, prompt);
}

void setup()
/*
 * purpose: initialize shell
 * returns: nothing. calls fatal() if trouble
 */
{
	extern char **environ;

	VLenviron2table(environ);
	char *pid;
	asprintf(&pid, "%d", getpid());
	VLstore("$", pid); 			/* store process id */
	signal(SIGINT,  SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
}

void fatal(char *s1, char *s2, int n)
{
	fprintf(stderr,"Error: %s,%s\n", s1, s2);
	exit(n);
}
