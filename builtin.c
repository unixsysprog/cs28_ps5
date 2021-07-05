/* builtin.c
 * contains the switch and the functions for builtin commands
 */

#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>
#include	<stdlib.h>
#include	<inttypes.h>
#include	<errno.h>
#include	<unistd.h>
#include	<sys/types.h>
#include	<sys/uio.h>

#include	"smsh.h"
#include	"varlib.h"
#include	"builtin.h"
#include	"splitline.h"

int is_builtin(char **args, int *resultp)
/*
 * purpose: run a builtin command 
 * returns: 1 if args[0] is builtin, 0 if not
 * details: test args[0] against all known builtins.  Call functions
 */
{
	if ( is_assign_var(args[0], resultp) )
		return 1;
	if ( is_list_vars(args[0], resultp) )
		return 1;
	if ( is_export(args, resultp) )
		return 1;
	if ( is_cd(args, resultp) )
		return 1;
	if ( is_exit(args, resultp) )
		return 1;
	if ( is_read(args, resultp) )
		return 1;
	if ( is_exec(args, resultp) )
		return 1;
	return 0;
}
/* checks if a legal assignment cmd
 * if so, does it and retns 1
 * else return 0
 */
int is_assign_var(char *cmd, int *resultp)
{
	char *eq_sign;
	if ( (eq_sign = strchr(cmd, '=')) != NULL ){
		if ( isdigit(cmd) ) /* bash variables cannot start with a digit */
			return 0;

		for (char * substr = cmd; substr != eq_sign; substr++ ) {
			if ( !(isalnum(*substr) || *substr != '_') )
				return 0;  /* bash vars are only alphanumeric with underscores */
		}

		*resultp = assign(cmd);
		if ( *resultp != -1 )
			return 1;
	}
	return 0;
}
/* checks if command is "set" : if so list vars */
int is_list_vars(char *cmd, int *resultp)
{
	if ( strcmp(cmd,"set") == 0 ){	     /* 'set' command? */
		VLlist();
		*resultp = 0;
		return 1;
	}
	return 0;
}
/*
 * if an export command, then export it and ret 1
 * else ret 0
 * note: the opengroup says
 *  "When no arguments are given, the results are unspecified."
 */
int is_export(char **args, int *resultp)
{
	if ( strcmp(args[0], "export") == 0 ){
		if ( args[1] != NULL && okname(args[1]) )
			*resultp = VLexport(args[1]);
		else
			*resultp = 1;
		return 1;
	}
	return 0;
}

int assign(char *str)
/*
 * purpose: execute name=val AND ensure that name is legal
 * returns: -1 for illegal lval, or result of VLstore 
 * warning: modifies the string, but retores it to normal
 */
{
	char	*cp;
	int	rv ;

	cp = strchr(str,'=');
	*cp = '\0';
	rv = ( okname(str) ? VLstore(str,cp+1) : -1 );
	*cp = '=';
	return rv;
}
int okname(char *str)
/*
 * purpose: determines if a string is a legal variable name
 * returns: 0 for no, 1 for yes
 */
{
	char	*cp;

	for(cp = str; *cp; cp++ ){
		if ( (isdigit(*cp) && cp==str) || !(isalnum(*cp) || *cp=='_' ))
			return 0;
	}
	return ( cp != str );	/* no empty strings, either */
}

int is_cd(char **args, int *resultp)
/*
 * checks to see if the first argument is cd
 * if the arg is cd then call exec_cd with the preceding arguments 
 */
{
	if ( strcmp(args[0], "cd") != 0 )
		return 0;

	*resultp = exec_cd(&args[1]);
	return 1; 
}

int is_exit(char **args, int *resultp)
/*
 * checks to see if the first argument is exit
 * if the arg is exit then call exec_exit to exit the shell
 */
{ 
	if ( strcmp(args[0], "exit") != 0 )
		return 0;

	*resultp = exec_exit(args + 1);
	return 1; 
}

int is_read(char **args, int *resultp)
/*
 * checks to see if the first argument is read
 * if the arg is read then call exec_read to read 
 */
{ 
	if ( strcmp(args[0], "read") != 0 )
		return 0;

	*resultp = exec_read(args + 1);
	return 1; 
}

int is_exec(char **args, int *resultp)
/*
 * checks to see if the first argument is the exec command
 */
{
	if ( strcmp(args[0], "exec") != 0)
		return 0;
	*resultp = exec_exec(args);
	return 1; 
}

int exec_exit(char ** args)
{
	int exit_status = 0;
	char *endptr;
	if ( args[0] != NULL && args[1] != NULL ) { /* ex: exit 5 foo */
		fprintf(stderr, "%s  ", args[1]);
		fprintf(stderr, "exit: too many arguments\n");
		return -1;
	}

	if ( args[0] != NULL ) { 					/* ex: exit 3 */
		exit_status = strtol(args[0], &endptr, 10);
		if ( strlen(endptr) > 0 ) {  /* the exit status was not just a number */
			fprintf(stderr, "exit: %s: numeric argument required\n", args[0]);
			return -1; 
		} 
	}

	exit(exit_status);
	return -1;
}

int exec_cd(char ** args)
{ 
	int rv = 0;
	char *home;

	if ( args[0] == NULL ) {
		home = VLlookup("HOME");
		rv = chdir(home); 
	} else {
		rv = chdir(args[0]); 
	}

	if ( rv == -1 ) {
		perror("cd");
		exit(1);
	}
	return rv;
}

int exec_read(char ** args)
/*
 * reads from user input and stores value in the first argument to read
 * if no argument in supplied, user input is stored on REPLY variable
 */
{
	char * key;

	if ( args[0] == NULL )
		key = "REPLY";
	else
		key = args[0];

	char *prompt = "";
	char *input = next_cmd(prompt, stdin);

	VLstore(key, input);
	return 1; 
}

int exec_exec(char **args)
{
	execvp(args[1], args + 1);
	perror(args[0]);
	exit(1);
}
