/* controlflow.c
 *
 * "if" processing is done with two state variables
 *    if_state and if_result
 */
#include	<stdio.h>
#include 	<string.h>
#include	<stdlib.h>
#include	"smsh.h"
#include	"process.h"

enum states   { NEUTRAL, WANT_THEN, THEN_BLOCK, WANT_ELSE, ELSE_BLOCK };
enum results  { SUCCESS, FAIL };

static int if_state  = NEUTRAL;
static int if_result = SUCCESS;
static int last_stat = 0;

int	syn_err(char *);

int ok_to_execute()
/*
 * purpose: determine the shell should execute a command
 * returns: 1 for yes, 0 for no
 * details: if in THEN_BLOCK and if_result was SUCCESS then yes
 *          if in THEN_BLOCK and if_result was FAIL    then no
 *          if in WANT_THEN  then syntax error (sh is different)
 */
{
	int	rv = 1;		/* default is positive */
	
	if ( if_state == WANT_THEN ){
		syn_err("then expected");
		rv = 0;
	}
	else if ( if_state == THEN_BLOCK && if_result == SUCCESS ) {
		rv = 1; 
	}
	else if ( if_state == THEN_BLOCK && if_result == FAIL ) {
		rv = 0; 
	}
	else if ( if_state == ELSE_BLOCK && if_result == SUCCESS ) {
		rv = 0; 
	}
	else if ( if_state == ELSE_BLOCK && if_result == FAIL ) {
		rv = 1; 
	}

	return rv;
}

int is_control_command(char *s)
/*
 * purpose: boolean to report if the command is a shell control command
 * returns: 0 or 1
 */
{
    return (
		strcmp(s, "if")   == 0 ||
		strcmp(s, "then") == 0 ||
		strcmp(s, "else") == 0 ||
		strcmp(s, "fi")   == 0
	);
}


int do_control_command(char **args)
/*
 * purpose: Process "if", "then", "fi" - change state or detect error
 * returns: 0 if ok, -1 for syntax error
 *   notes: I would have put returns all over the place, Barry says "no"
 */
{
	char	*cmd = args[0];
	int	rv = -1;

	if( strcmp(cmd,"if")==0 ){
		if ( if_state != NEUTRAL )
			rv = syn_err("if unexpected");
		else {
			last_stat = process(args+1);
			if_result = (last_stat == 0 ? SUCCESS : FAIL );
			if_state = WANT_THEN;
			rv = 0;
		}
	}
	else if ( strcmp(cmd,"then")==0 ){
		if ( if_state != WANT_THEN )
			rv = syn_err("then unexpected");
		else {
			if_state = THEN_BLOCK;
			rv = 0;
		}
	}
	else if ( strcmp(cmd,"else")==0 ){
		if ( if_state == NEUTRAL || 
			 if_state == WANT_THEN || 
			 if_state == ELSE_BLOCK 
		) { 
			rv = syn_err("else unexpected");
		}
		if_state = ELSE_BLOCK;
		rv = 0;
	}
	else if ( strcmp(cmd,"fi")==0 ){ 
		// These are incorrect states for the fi token
		if ( if_state == NEUTRAL || if_state == WANT_THEN) { 
			rv = syn_err("fi unexpected");
		}
		else {
			if_state = NEUTRAL;
			rv = 0;
		}
	}
	else 
		fatal("internal error processing:", cmd, 2);
	
	return rv;
}

int syn_err(char *msg)
/* purpose: handles syntax errors in control structures
 * details: resets state to NEUTRAL
 * returns: -1 in interactive mode. Should call fatal in scripts
 */
{
	if_state = NEUTRAL;
	fprintf(stderr,"syntax error: %s\n", msg);
	return -1;
}

void check_if_state(char *filename, int line_number)
/*
 * checks to make sure the if_state is in neutral when reaching the
 * EOF function. Otherwise there's a syntax error.
 */
{
	if ( if_state == NEUTRAL ) return ; /* NEUTRAL is the expected state at EOF*/ 

	fprintf(stderr, "%s: line %d: ", filename, line_number);
	syn_err("unexpected end of file");
	exit(2);
}