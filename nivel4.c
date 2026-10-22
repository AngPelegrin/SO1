//
// Created by GaWii on 24/10/2025.
//

#define _POSIX_C_SOURCE 200112L

#include "my_shell.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

static char mi_shell[COMMAND_LINE_SIZE];
static struct info_job jobs_list[N_JOBS];
int jobs;
int const DEBUGNx = 1;

void print_env() {
    if (DEBUGNx) {
        printf("PWD : %s\n", getenv("PWD"));
        printf("HOME : %s\n", getenv("HOME"));
        printf("LANGUAGE : %s\n", getenv("LANG"));
        printf("USER : %s\n", getenv("USER"));
        const int size = 100;
        char buf[size];
        printf("CURRENT DIR: %s\n", getcwd(buf,size));
    }
}

void start_shell(char **argv) {
    signal(SIGCHLD, reaper);
    signal(SIGINT,  ctrlc);
    char line[COMMAND_LINE_SIZE];
    strcpy(mi_shell,argv[0]);
    jobs = 0;
    memset(jobs_list, 0, sizeof(jobs_list));
    jobs_list[0].pid = 0;
    jobs_list[0].estado = 'F';
    jobs_list[0].cmd[0] = '\0';
    print_env();
    while (1) {
        read_line(line);

        if (line[0] != '\0') {
            execute_line(line);
        }
    }
}

int count_args(char **args) {
    int count = 0;
    while (args[count] != NULL) {
        count++;
    }
    return count;
}

char *read_line(char *line) {
    usleep(500000);
    fflush(stdout);
    print_prompt();
    char* ctrd = fgets(line, COMMAND_LINE_SIZE, stdin);
    if (ctrd == NULL) {
        if (feof(stdin)) {
            fflush(stdout);
            printf("Saliendo del minishell... ¡Hasta pronto!\n");
            exit(0);
        }
        fprintf(stderr, "Error en fgets(): ");
        perror("read_line");
        return NULL;
    }
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') {
        line[len - 1] = '\0';
    }
    line[strcspn(line, "\r")] = '\0';
    return line;
}

int execute_line(char *line) {
    char **args = malloc(sizeof(char) * COMMAND_LINE_SIZE);
    if (!args) {
        fprintf(stderr, ROJO "Error creando la memoria" RESET);
        return 1;
    }
    int tokens = parse_args(args,line);
    if (tokens > 0) {
        check_internal(args);
    }
    free(args);
    return 0;
}

void print_prompt() {
    printf(AZUL"%s:%s%c "RESET,getenv("USER"),getenv("PWD"),PROMPT);
    fflush(stdout);
}

int parse_args(char **args, char *line) {
    int i = 0;
    int corr = 0;
    char *token = strtok(line, " \t\r\n");

    while (token != NULL) {

        if (DEBUGNx) {
            printf(GRIS"[parse_args()→token %d: %s]\n"RESET,i,token);
            if (token[0] == '#') {
                corr = 1;
                printf(GRIS"[parse_args()→token %d corregido: (null)]\n"RESET,i);
                break;
            }
        }

        args[i++] = token;
        token = strtok(NULL, " \t\r\n");
    }
    if (DEBUGNx) {
        if (corr == 0) {
            printf(GRIS"[parse_args()→token %d: %s]\n"RESET,i,token);
        }
    }
    args[i] = NULL;
    return i;
}

int check_internal(char **args) {
    if (args[0] == NULL) {
        fprintf(stderr, ROJO "Argumentos vacios\n" RESET);
        return 0;
    }

    if (strcmp(args[0], "cd") == 0) {
        if (DEBUGNx) {
            printf(GRIS"[internal_cd()→ Esta función cambiará de directorio]\n"RESET);
        }
        return internal_cd(args);
    }
    if (strcmp(args[0], "export") == 0) {
        if (DEBUGNx) {
            printf(GRIS"[internal_export()→Esta función asignará valores a variablescd de entorno]\n"RESET);
        }
        return internal_export(args);
    }
    if (strcmp(args[0], "source") == 0) {
        if (DEBUGNx) {
            printf(GRIS"[internal_source()→Esta función ejecutará un fichero de líneas de comandos]\n"RESET);
        }
        return internal_source(args);
    }
    if (strcmp(args[0], "jobs") == 0) {
        if (DEBUGNx) {
            printf(GRIS"[internal_jobs()→Esta función mostrará el PID de los procesos que no estén en foreground]\n"RESET);
        }
        return internal_jobs(args);
    }
    if (strcmp(args[0], "fg") == 0) {
        return internal_fg(args);
    }
    if (strcmp(args[0], "bg") == 0) {
        return internal_bg(args);
    }
    if (strcmp(args[0], "exit") == 0) {
        printf("Saliendo del minishell... ¡Hasta pronto!\n");
        exit(0);
    }
    return external(args);
}

int internal_cd(char **args) {
    int args_size = count_args(args);
    const char *target;
    if (args_size == 1) {
        target = getenv("HOME");
    }
    else if (args_size == 2) {
        target = args[1];
    }
    else {
        target = getenv("PWD");
    }
    if (DEBUGNx) {
        char oldpwd[COMMAND_LINE_SIZE];
        getcwd(oldpwd, COMMAND_LINE_SIZE);
        printf(GRIS "[internal_cd()→PWD inicial: %s]\n" RESET, oldpwd);
    }
    if (chdir(target) != 0) {
        fprintf(stderr, ROJO);
        perror("cd");
        fprintf(stderr, RESET);
        return 1;
    }
    char newpwd[COMMAND_LINE_SIZE];
    getcwd(newpwd, COMMAND_LINE_SIZE);

    setenv("PWD", newpwd, 1);

    if (DEBUGNx) {
        printf(GRIS "[internal_cd()→PWD final: %s]\n" RESET, newpwd);
    }
    return 0;
}

int export_valido(char *export) {
    char *valores[] = {"PWD", "HOME", "LANG", "USER"};
    for (int i = 0; i < 4; ++i) {
        if (strcmp(export, valores[i]) == 0) {
            return 1;
        }
    }
    fprintf(stderr, ROJO "Uso: export Nombre=Valor, Nombre invalido\n" RESET);
    return 0;
}

int internal_export(char **args){
    if (args[1] == NULL) {
        fprintf(stderr, ROJO "Error de sintaxis. Uso: export Nombre=Valor\n" RESET);
        return 1;
    }

    if (strchr(args[1], '=') == NULL) {
        fprintf(stderr, ROJO "Error de sintaxis. Uso: export Nombre=Valor\n" RESET);
        return 1;
    }
    char buffer[COMMAND_LINE_SIZE];
    strcpy(buffer, args[1]);
    char *nombre = strtok(buffer, "=");
    char *valor  = strtok(NULL, "");
    if (!export_valido(nombre)) {
        return 1;
    }
    if (DEBUGNx) {
        printf(GRIS "[internal_export()→ antiguo valor para %s: %s]\n" RESET,
           nombre,
           getenv(nombre) ? getenv(nombre) : "(null)");
    }
    if (setenv(nombre, valor, 1) != 0) {
        fprintf(stderr, ROJO);
        perror("setenv");
        fprintf(stderr, RESET);
        return 1;
    }
    if (DEBUGNx) {
        printf(GRIS "[internal_export()→ nuevo valor para %s: %s]\n" RESET,
           nombre,
           getenv(nombre));
    }
    return 0;
}

int internal_source(char **args){
    if (args[1] == NULL) {
        fprintf(stderr, ROJO "Error de sintaxis. Uso: source <nombre_fichero>\n" RESET);
        return 1;
    }

    FILE *f = fopen(args[1], "r");
    if (!f) {
        fprintf(stderr, ROJO);
        perror("fopen");
        fprintf(stderr, RESET);
        return 1;
    }

    char line[COMMAND_LINE_SIZE];

    while (fgets(line, COMMAND_LINE_SIZE, f)) {
        line[strcspn(line, "\n")] = '\0';
        if (DEBUGNx) {
            printf(NARANJA "[internal_source()→ LINE: %s]\n" RESET, line);
        }
        fflush(f);
        execute_line(line);
    }

    fclose(f);
    return 0;
}

int internal_jobs(){
    return 0;
}
int internal_fg(char **args){
    return 0;
}
int internal_bg(char **args){
    return 0;
}

char *remake(char **args) {
    if (args == NULL || args[0] == NULL)
        return NULL;

    char *line = malloc(COMMAND_LINE_SIZE);
    if (!line) {
        fprintf(stderr, ROJO);
        perror("malloc");
        fprintf(stderr, RESET);
        return NULL;
    }

    line[0] = '\0';

    for (int i = 0; args[i] != NULL; i++) {
        strcat(line, args[i]);
        if (args[i+1] != NULL) {
            strcat(line, " ");
        }
    }

    return line;
}

int external(char **args) {
    pid_t pid;
    char *cmdline = remake(args);

    if (!cmdline) {
        return 1;
    }

    pid = fork();

    if (pid == -1) {
        fprintf(stderr, ROJO);
        perror("fork");
        fprintf(stderr, RESET);
        free(cmdline);
        return 1;
    }

    if (pid == 0) {
        /* HIJO */
        signal(SIGINT, SIG_IGN);   // Ignorar Ctrl+C en hijo

        execvp(args[0], args);
        fprintf(stderr, ROJO);
        perror("execvp");
        fprintf(stderr, RESET);
        free(cmdline);
        exit(-1);
    }

    /* PADRE */
    jobs_list[0].pid = pid;
    jobs_list[0].estado = 'E';
    memset(jobs_list[0].cmd, 0, COMMAND_LINE_SIZE);
    strncpy(jobs_list[0].cmd, cmdline, COMMAND_LINE_SIZE - 1);

    if (DEBUGNx) {
        char buffer[1024];
        sprintf(buffer,
                NARANJA "[execute_line()→ PID padre: %d (%s)]\n"
                        "[execute_line()→ PID hijo: %d (%s)]\n" RESET,
                getpid(), mi_shell,
                pid, jobs_list[0].cmd);
        write(2, buffer, strlen(buffer));
    }

    while (jobs_list[0].pid > 0)
        pause();   // Espera señales SIGCHLD o SIGINT

    free(cmdline);
    return 0;
}

void reaper(int signum) {
    signal(SIGCHLD, reaper);

    pid_t ended;
    int status;

    while ((ended = waitpid(-1, &status, WNOHANG)) > 0) {

        if (DEBUGNx) {
            char buffer[1024];
            sprintf(buffer,
                NARANJA "[reaper()→ recibida señal %d (SIGCHLD)]\n" RESET,
                signum);
            write(2, buffer, strlen(buffer));
        }

        if (ended == jobs_list[0].pid) {
            if (DEBUGNx) {
                char buffer[1024];

                if (WIFEXITED(status)) {
                    sprintf(buffer,
                        NARANJA "[reaper()→ Proceso hijo %d (%s) finalizado con exit code %d]\n" RESET,
                        ended, jobs_list[0].cmd, WEXITSTATUS(status));
                } else if (WIFSIGNALED(status)) {
                    sprintf(buffer,
                        NARANJA "[reaper()→ Proceso hijo %d (%s) finalizado por señal %d]\n" RESET,
                        ended, jobs_list[0].cmd, WTERMSIG(status));
                }
                write(2, buffer, strlen(buffer));
            }

            jobs_list[0].pid = 0;
            jobs_list[0].estado = 'F';
            jobs_list[0].cmd[0] = '\0';
        }
    }
}

void ctrlc(int signum) {
    signal(SIGINT, ctrlc);

    if (DEBUGNx) {
        char buffer[1024];
        sprintf(buffer,
            NARANJA "[ctrlc()→ Soy el proceso con PID %d (%s), el proceso en foreground es %d (%s)]\n" RESET,
            getpid(), mi_shell,
            jobs_list[0].pid,
            jobs_list[0].cmd);
        write(2, buffer, strlen(buffer));

        sprintf(buffer,
            NARANJA "[ctrlc()→ recibida señal %d (SIGINT)]\n" RESET,
            signum);
        write(2, buffer, strlen(buffer));
    }

    if (jobs_list[0].pid > 0) {

        if (strcmp(jobs_list[0].cmd, mi_shell) == 0) {
            if (DEBUGNx) {
                char buffer[1024];
                sprintf(buffer,
                    NARANJA "[ctrlc()→ Señal 15 (SIGTERM) no enviada por %d (%s) debido a que su proceso en foreground es el shell]\n" RESET,
                    getpid(), mi_shell);
                write(2, buffer, strlen(buffer));
            }
            return;
        }

        kill(jobs_list[0].pid, SIGTERM);

        if (DEBUGNx) {
            char buffer[1024];
            sprintf(buffer,
                NARANJA "[ctrlc()→ Señal 15 (SIGTERM) enviada a %d (%s) por %d (%s)]\n" RESET,
                jobs_list[0].pid,
                jobs_list[0].cmd,
                getpid(),
                mi_shell);
            write(2, buffer, strlen(buffer));
        }

    } else if (DEBUGNx) {
        char buffer[1024];
        sprintf(buffer,
            NARANJA "[ctrlc()→ Señal 15 (SIGTERM) no enviada por %d (%s) debido a que no hay proceso en foreground]\n" RESET,
            getpid(), mi_shell);
        write(2, buffer, strlen(buffer));
    }

    fflush(stdout);
}

int main(int argc, char **argv) {
    (void)argc;
    start_shell(argv);
    return 0;
}
