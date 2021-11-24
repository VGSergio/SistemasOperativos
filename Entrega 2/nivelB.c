/**
 * Authors:
 * Vega García, Sergio
 * Seguí Vives, Mateo
 * Planells Torres, David
 **/

#define _POSIX_C_SOURCE 200112L

#define DEBUGN1 0 //parse_args()
#define DEBUGN3 1 //execute_line()

#define PROMPT_PERSONAL 1 // si no vale 1 el prompt será solo el carácter de PROMPT

#define RESET_FORMATO "\033[0m"
#define NEGRO_T "\x1b[30m"
#define NEGRO_F "\x1b[40m"
#define GRIS "\x1b[94m"
#define ROJO_T "\x1b[31m"
#define VERDE_T "\x1b[32m"
#define AMARILLO_T "\x1b[33m"
#define AZUL_T "\x1b[34m"
#define MAGENTA_T "\x1b[35m"
#define CYAN_T "\x1b[36m"
#define BLANCO_T "\x1b[97m"
#define NEGRITA "\x1b[1m"

#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64
#define N_JOBS 24 // cantidad de trabajos permitidos

char const PROMPT = '$';

#include <errno.h>  //errno
#include <stdio.h>  //printf(), fflush(), fgets(), stdout, stdin, stderr, fprintf()
#include <stdlib.h> //setenv(), getenv()
#include <string.h> //strcmp(), strtok(), strerror()
#include <unistd.h> //NULL, getcwd(), chdir()
#include <sys/types.h> //pid_t
#include <sys/wait.h>  //wait()
#include <signal.h> //signal(), SIG_DFL, SIG_IGN, SIGINT, SIGCHLD
#include <fcntl.h> //O_WRONLY, O_CREAT, O_TRUNC
#include <sys/stat.h> //S_IRUSR, S_IWUSR

int check_internal(char **args);
int internal_cd(char **args);
int internal_export(char **args);
int internal_source(char **args);
int internal_jobs();
int internal_bg(char **args);
int internal_fg(char **args);

char *read_line(char *line);
int parse_args(char **args, char *line);
int execute_line(char *line);

void reaper(int signum);
void ctrlc(int signum);

static char mi_shell[COMMAND_LINE_SIZE]; //variable global para guardar el nombre del minishell

//static pid_t foreground_pid = 0;
struct info_process {
	pid_t pid;
	char status;
	char cmd[COMMAND_LINE_SIZE];
};
static struct info_process jobs_list[N_JOBS]; //Tabla de procesos. La posición 0 será para el foreground

void imprimir_prompt();

int check_internal(char **args) {
    if (!strcmp(args[0], "cd"))
        return internal_cd(args);
    if (!strcmp(args[0], "export"))
        return internal_export(args);
    if (!strcmp(args[0], "source"))
        return internal_source(args);
    if (!strcmp(args[0], "jobs"))
        return internal_jobs(args);
    if (!strcmp(args[0], "bg"))
        return internal_bg(args);
    if (!strcmp(args[0], "fg"))
        return internal_fg(args);
    if (!strcmp(args[0], "exit"))
        exit(0);
    return 0; // no es un comando interno
}

int internal_cd(char **args) {
    printf("[internal_cd()→ comando interno no implementado]\n");
    return 1;
} 

int internal_export(char **args) {
    printf("[internal_export()→ comando interno no implementado]\n");
    return 1;
}

int internal_source(char **args) {
    printf("[internal_source()→ comando interno no implementado]\n");
    return 1;
}

int internal_jobs(char **args) {
    #if DEBUGN1 
        printf("[internal_jobs()→ Esta función mostrará el PID de los procesos que no estén en foreground]\n");
    #endif
    return 1;
}

int internal_fg(char **args) {
    #if DEBUGN1 
        printf("[internal_fg()→ Esta función enviará un trabajo detenido al foreground reactivando su ejecución, o uno del background al foreground ]\n");
    #endif
    return 1;
}

int internal_bg(char **args) {
    #if DEBUGN1 
        printf("[internal_bg()→ Esta función reactivará un proceso detenido para que siga ejecutándose pero en segundo plano]\n");
    #endif
    return 1;
}

void imprimir_prompt() {
#if PROMPT_PERSONAL == 1
    printf(NEGRITA ROJO_T "%s" BLANCO_T ":", getenv("USER"));
    printf(AZUL_T "MINISHELL" BLANCO_T "%c " RESET_FORMATO, PROMPT);
#else
    printf("%c ", PROMPT);

#endif
    fflush(stdout);
    return;
}


char *read_line(char *line) {
  
    imprimir_prompt();
    char *ptr=fgets(line, COMMAND_LINE_SIZE, stdin); // leer linea
    if (ptr) {
        // ELiminamos el salto de línea (ASCII 10) sustituyéndolo por el \0
        char *pos = strchr(line, 10);
        if (pos != NULL){
            *pos = '\0';
        } 
	}  else {   //ptr==NULL por error o eof
        printf("\r");
        if (feof(stdin)) { //se ha pulsado Ctrl+D
            fprintf(stderr,"Bye bye\n");
            exit(0);
        }   
    }
    return ptr;
}

int parse_args(char **args, char *line) {
    int i = 0;

    args[i] = strtok(line, " \t\n\r");
    #if DEBUGN1 
        fprintf(stderr, GRIS "[parse_args()→ token %i: %s]\n" RESET_FORMATO, i, args[i]);
    #endif
    while (args[i] && args[i][0] != '#') { // args[i]!= NULL && *args[i]!='#'
        i++;
        args[i] = strtok(NULL, " \t\n\r");
        #if DEBUGN1 
            fprintf(stderr, GRIS "[parse_args()→ token %i: %s]\n" RESET_FORMATO, i, args[i]);
        #endif
    }
    if (args[i]) {
        args[i] = NULL; // por si el último token es el símbolo comentario
        #if DEBUGN1 
            fprintf(stderr, GRIS "[parse_args()→ token %i corregido: %s]\n" RESET_FORMATO, i, args[i]);
        #endif
    }
    return i;
}

int execute_line(char *line) {
    char *args[ARGS_SIZE];
    pid_t pid;
    char command_line[COMMAND_LINE_SIZE];

    //copiamos la línea de comandos sin '\n' para guardarlo en el array de structs de los procesos
    memset(command_line, '\0', sizeof(command_line)); 
    strcpy(command_line, line); //antes de llamar a parse_args() que modifica line

    if (parse_args(args, line) > 0) {
        if (check_internal(args)) {
            return 1;
        } else {
            pid = fork();
            if (pid==0){ //Hijo
                signal(SIGCHLD, SIG_DFL);
                signal(SIGINT, SIG_IGN);
                #if DEBUGN3
                    fprintf(stderr, GRIS "[execute_line()→ PID hijo: %d (%s)]\n" RESET_FORMATO, getpid(), command_line);
                #endif
                
                if(execvp(args[0], args)<0){
                    fprintf(stderr, RESET_FORMATO "%s: no se encontró la orden\n", command_line);
                    exit(-1);
                }
            } else {//Padre
                signal(SIGINT, ctrlc);
                #if DEBUGN3
                    fprintf(stderr, GRIS "[execute_line()→ PID padre: %d (%s)]\n" RESET_FORMATO, getpid(), mi_shell);
                #endif

                jobs_list[0].pid = pid;
                jobs_list[0].status = 'E';
                strcpy(jobs_list[0].cmd, command_line);
                while(jobs_list[0].pid>0){
                    pause();
                }
            }
        }
        
    }
    return 0;
}

void reaper(int signum){
    signal(SIGCHLD, reaper);

    int ended;
    int status;
    while ((ended=waitpid(-1, &status, WNOHANG)) > 0) {
        if(jobs_list[0].pid == ended){
            char mensaje[2*1024];
            if(WIFSIGNALED(status)){
                sprintf(mensaje, GRIS "\n[reaper())→ Proceso hijo %d (%s) finalizado por señal %d]\n" RESET_FORMATO, ended, jobs_list[0].cmd, WTERMSIG(status));
            } else if(WIFEXITED(status)){
                sprintf(mensaje, GRIS "[reaper())→ Proceso hijo %d (%s) finalizado con exit code %d]\n" RESET_FORMATO, ended, jobs_list[0].cmd, WEXITSTATUS(status));
            }
            write(2, mensaje, strlen(mensaje));
            jobs_list[0].pid = 0;
            jobs_list[0].status = 'F';
            memset(jobs_list[0].cmd, '\0', sizeof(jobs_list[0].cmd));
        }
    }
    
}

void ctrlc(int signum){
    signal(SIGINT, ctrlc);
    
    char mensaje[3*1024];
    sprintf(mensaje, GRIS "\n[ctrlc()→ Soy el proceso con PID %d (%s), el proceso en foreground es %d (%s)]" RESET_FORMATO,
            getpid(), mi_shell, jobs_list[0].pid, jobs_list[0].cmd);
    write(2, mensaje, strlen(mensaje));

    if(jobs_list[0].pid > 0){
        if(strcmp(jobs_list[0].cmd, mi_shell) != 0){
            kill(jobs_list[0].pid, SIGTERM);
            sprintf(mensaje, GRIS "\n[ctrlc()→ Señal %d enviada a %d (%s) por %d (%s)]" RESET_FORMATO,
            SIGTERM, jobs_list[0].pid, jobs_list[0].cmd, getpid(), mi_shell);
            write(2, mensaje, strlen(mensaje));
        } else {
            sprintf(mensaje, GRIS "\n[ctrlc()→ Señal %d no enviada por %d (%s) debido a que su proceso en foreground es el shell]" RESET_FORMATO,
            SIGTERM, getpid(), mi_shell);
            write(2, mensaje, strlen(mensaje));
        }
    } else {
        sprintf(mensaje, GRIS "\n[ctrlc()→ Señal %d no enviada por %d (%s) debiado a que no hay proceso en foreground]\n" RESET_FORMATO,
            SIGTERM, getpid(), mi_shell);
        write(2, mensaje, strlen(mensaje));
    }
    
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    char line[COMMAND_LINE_SIZE];
    memset(line, 0, COMMAND_LINE_SIZE);

	signal(SIGCHLD,reaper);
    signal(SIGINT,ctrlc);

    while (1) {
        if (read_line(line)) { // !=NULL
            //Copiamos el nombre del programa que actua como minishell en mi_shell.
            memset(mi_shell, '\0', sizeof(mi_shell));
            strcpy(mi_shell, argv[0]);
            //Inicializamos jobs_list[0] con pid a 0, status 'N' y cmd a '\0'.
            jobs_list[0].pid = 0;
            jobs_list[0].status = 'N';
            memset(jobs_list[0].cmd, '\0', sizeof(jobs_list[0].cmd));
            
            execute_line(line);
        }
    }
    return 0;
}