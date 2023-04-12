#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define ENV_VAR "CHILD_PATH"

char *find_var(char *env[]){
    char *buf = (char*)calloc(255, sizeof(char));
    for(int i = 0; env[i] != NULL; i++){
        if(strlen(ENV_VAR) < strlen(env[i])){
            if(strstr(env[i], ENV_VAR) != NULL) {
                strcpy(buf, strstr(env[i], ENV_VAR) + strlen(ENV_VAR) + 1);
                if(buf == NULL){
                    fprintf(stderr,"Something's wrong with environment\n");
                    exit(1);
                }
                return buf;
            }
        }
    }
    return NULL;
}

void print_params_list(char *envp[]){
    char **param_list = (char**)calloc(0, sizeof(char*));
    int list_size;
    for(int i = 0; envp[i] != NULL; i++){
        param_list = realloc(param_list, (i + 1) * sizeof(char*));
        param_list[i] = (char*)calloc(strlen(envp[i]) + 1, sizeof(char));
        strcpy(param_list[i], envp[i]);
        list_size = i + 1;
    }
    for (int i = 0; i < list_size; i++){
        int min = i;
        for(int j = i + 1; j < list_size; j++){
            if(strcoll(param_list[min], param_list[j]) > 0)
                min = j;
            if(min != i){
                char *buf = (char*)calloc(strlen(param_list[min]) + 1, sizeof(char));
                strcpy(buf, param_list[min]);
                param_list[min] = realloc(param_list[min] ,strlen(param_list[i]) + 1);
                strcpy(param_list[min], param_list[i]);
                param_list[i] = realloc(param_list[i] ,strlen(buf) + 1);
                strcpy(param_list[i], buf);
            }
        }
    }

    for(int i = 0; i < list_size; i++){
        fprintf(stdout, "%s\n", param_list[i]);
    }
}

int main(int argc,char *argv[], char *envp[]) {
    int child_number = 0;
    char child_out[] = "CHILD_#";
    char counter_buf[20];
    if(!getenv(ENV_VAR)){
        fprintf(stderr,"Something's wrong with environment\n");
        exit(1);
    }
    if (argc != 2) {
        fprintf(stderr, "Wrong number of arguments(must be 2)\n");
        exit(1);
    }

    print_params_list(envp);
    while (1) {
        char except;
        int child_status;
        char choice;
        scanf("%c", &choice);
        if (scanf("%c", &except) && except != '\n') {
            fprintf(stderr, "Wrong param input\n");
            continue;
        }
            child_number++;
            char *child_info = (char *) calloc(255, sizeof(char));
            switch (choice) {
                case '+':{
                    pid_t proc = fork();
                    if(proc == -1){
                        fprintf(stderr, "Cannot create new process\n");
                        exit(1);
                    }
                    else if(proc == 0){
                        child_info = getenv(ENV_VAR);
                        sprintf(counter_buf, "%d", child_number);
                        char *args[4] = {strcat(child_out, counter_buf), argv[1], "+", NULL};
                        execve(child_info, args, envp);
                    }
                    wait(&child_status);
                    fprintf(stdout, "Child ended with %d status\n", child_status);
                    break;
                }
                case '*':{
                    pid_t proc = fork();
                    if(proc == -1){
                        fprintf(stderr, "Cannot create new process\n");
                        exit(1);
                    }
                    else if(proc == 0) {
                        child_info = find_var(envp);
                        fprintf( stdout,"%s\n", child_info);
                        sprintf(counter_buf, "%d", child_number);
                        char *args[4] = {strcat(child_out, counter_buf), argv[1], "*", NULL};
                        execve(child_info, args, envp);
                    }
                    wait(&child_status);
                    fprintf(stdout, "Child ended with %d status\n", child_status);
                    break;
                }
                case '&':{
                    pid_t proc = fork();
                    if(proc == -1){
                        fprintf(stderr, "Cannot create new process\n");
                        exit(1);
                    }
                    else if(proc == 0) {
                        child_info = find_var(__environ);
                        fprintf( stdout,"%s\n", child_info);
                        sprintf(counter_buf, "%d", child_number);
                        char *args[4] = {strcat(child_out, counter_buf), argv[1], "&", NULL};
                        execve(child_info, args, envp);
                    }
                    wait(&child_status);
                    fprintf(stdout, "Child ended with %d status\n", child_status);
                    break;
                }
                case 'q':{
                    fprintf(stdout, "Successfully ended \n");
                    exit(0);
                }
            }

        }
    }
