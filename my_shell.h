//
// Created by GaWii on 24/10/2025.
//

#ifndef PR2_MY_SHELL_H
#define PR2_MY_SHELL_H

#define RESET "\033[0m"
#define NEGRO "\x1b[30m"
#define GRIS "\x1b[38;5;245m"
#define ROJO "\x1b[31m"
#define VERDE "\x1b[32m"
#define NARANJA "\x1b[38;5;214m"
#define AMARILLO "\x1b[33m"
#define AZUL "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define BLANCO "\x1b[97m"
#define NEGRITA "\x1b[1m"

#define COMMAND_LINE_SIZE 1024
#define N_JOBS 64
#define PROMPT '$'
#include <signal.h>

void start_shell();
void print_prompt();

char *read_line(char *line);
int execute_line(char *line);
int parse_args(char **args, char *line);
int check_internal(char **args);
int internal_cd(char **args);
int internal_export(char **args);
int internal_source(char **args);
int internal_jobs();
int internal_fg(char **args);
int internal_bg(char **args);
int external(char **args);
int is_background(char **args);
int is_output_redirection(char **args);
int jobs_list_add(pid_t pid, char estado, char *cmd);
int jobs_list_find(pid_t pid);
int jobs_list_remove(int pos);
void reaper(int signum);
void ctrlc(int signum);
void ctrlz(int signum);

struct info_job {
    pid_t pid;
    char estado; // 'N', 'E', 'D', 'F'
    char cmd[COMMAND_LINE_SIZE];
};

#endif //PR2_MY_SHELL_H