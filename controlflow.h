/*
 * controlflow.h
 */
int is_control_command(char *);
int do_control_command(char **);
int ok_to_execute();
void check_if_state(char*, int);
void syn_err(char *);
