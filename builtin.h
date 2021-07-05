#ifndef	BUILTIN_H
#define	BUILTIN_H

int is_builtin(char **args, int *resultp);
int is_assign_var(char *cmd, int *resultp);
int is_list_vars(char *cmd, int *resultp);
int assign(char *);
int okname(char *);
int is_export(char **, int *);
int is_cd(char **, int *);
int is_exit(char **, int *);
int is_read(char **, int *);
int is_exec(char **, int *);

int exec_cd(char **);
int exec_exit(char **);
int exec_read(char **);
int exec_exec(char **);

#endif
