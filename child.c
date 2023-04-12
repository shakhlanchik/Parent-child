#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[], char *env[]){
    if(argc != 3){
        fprintf(stdout, "Arguments error occurred\n");
        exit(1);
    }
    fprintf(stdout, " %s started\n PID: %d\n PPID: %d\n", argv[0], getpid(),getppid());
    FILE *file_stream = fopen(argv[1],"r");
    if (file_stream == NULL) {
        printf("Can't open file %s %s\n",argv[1], strerror(errno) );
        exit(-1);
    }
    char env_string[256];
    while (1) {
        int i = 0;
        fscanf(file_stream, "%s", env_string);
        if (feof(file_stream)) { //EOF
            break;
        }
        if (strcpy(argv[2],"+")) {
            fprintf(stdout, "%s = %s\n", env_string, getenv(env_string));
        }
        else if (strcpy(argv[2],"*")) {
            while (strstr(env[i++], env_string) == NULL);
            fprintf(stdout,"%s\n", env[i]);
        }
        else if (strcpy(argv[2],"&")) {
            while (strstr(__environ[i++], env_string) == NULL);
            fprintf(stdout,"%s\n", __environ[i]);
        }
    }
    exit(0);
}