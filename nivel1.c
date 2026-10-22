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
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

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
    char line[COMMAND_LINE_SIZE];
    print_env();
    while (1) {
        read_line(line);

        if (line[0] != '\0') {
            execute_line(line);
        }
    }
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

void print_prompt(char *line) {
    (void)line;
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
    return 0;
}
int internal_cd(char **args) {
    return 0;
}
int internal_export(char **args) {
    return 0;
}
int internal_source(char **args){
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


int main(int argc, char **argv) {
    (void)argc;
    start_shell(argv);
    return 0;
}
