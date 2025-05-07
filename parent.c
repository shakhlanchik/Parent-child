#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define ENV_VAR "CHILD_PATH"

// Функция для поиска значения переменной окружения CHILD_PATH
char *find_var(char *env[]) {
    char *buf = (char *)calloc(255, sizeof(char));
    if (buf == NULL) { // Проверка на ошибку выделения памяти
        perror("calloc in find_var");
        exit(1);
    }
    for (int i = 0; env[i] != NULL; i++) {
        // Исправлена опечатка: env[i  ] -> env[i]
        if (strlen(ENV_VAR) < strlen(env[i])) {
            if (strstr(env[i], ENV_VAR) != NULL) {
                // Проверяем, есть ли символ '=' после имени переменной
                char *equals_sign = strchr(env[i], '=');
                if (equals_sign != NULL) {
                    strcpy(buf, equals_sign + 1);
                    return buf;
                } else {
                    fprintf(stderr, "Environment variable %s has no value\n", ENV_VAR);
                    free(buf);
                    return NULL;
                }
                // Убрана лишняя проверка buf == NULL после strcpy.
                // Если strcpy сработал, buf уже содержит данные.
            }
        }
    }
    free(buf); // Освобождаем выделенную память, если переменная не найдена
    return NULL;
}

// Функция для печати и сортировки параметров командной строки
void print_params_list(char *envp[]) {
    int list_size = 0;
    // Сначала посчитаем количество параметров
    for (int i = 0; envp[i] != NULL; i++) {
        list_size++;
    }

    char **param_list = (char **)calloc(list_size, sizeof(char *));
    if (param_list == NULL) { // Проверка на ошибку выделения памяти
        perror("calloc in print_params_list");
        exit(1);
    }

    // Копируем параметры в наш список
    for (int i = 0; i < list_size; i++) {
        param_list[i] = (char *)calloc(strlen(envp[i]) + 1, sizeof(char));
        if (param_list[i] == NULL) { // Проверка на ошибку выделения памяти
            perror("calloc inside loop in print_params_list");
            // Освобождаем ранее выделенную память в случае ошибки
            for (int j = 0; j < i; j++) {
                free(param_list[j]);
            }
            free(param_list);
            exit(1);
        }
        strcpy(param_list[i], envp[i]);
    }

    // Сортировка списка параметров (выборкой)
    for (int i = 0; i < list_size - 1; i++) {
        int min = i;
        for (int j = i + 1; j < list_size; j++) {
            if (strcoll(param_list[min], param_list[j]) > 0) {
                min = j;
            }
        }
        if (min != i) {
            char *temp = param_list[i];
            param_list[i] = param_list[min];
            param_list[min] = temp;
            // Убрано лишнее выделение и копирование памяти для обмена.
            // Просто меняем указатели.
        }
    }

    // Вывод отсортированного списка
    for (int i = 0; i < list_size; i++) {
        fprintf(stdout, "%s\n", param_list[i]);
        free(param_list[i]); // Освобождаем выделенную память для каждого параметра
    }
    free(param_list); // Освобождаем память для самого списка указателей
}

int main(int argc, char *argv[], char *envp[]) {
    int child_number = 0;
    //char child_out[] = "CHILD_#";
    char counter_buf[20];

    // Проверяем наличие переменной окружения CHILD_PATH
    if (getenv(ENV_VAR) == NULL) {
        fprintf(stderr, "Environment variable %s is not set\n", ENV_VAR);
        exit(1);
    }

    // Проверяем количество аргументов командной строки
    if (argc != 2) {
        fprintf(stderr, "Wrong number of arguments (must be 2)\n");
        exit(1);
    }

    print_params_list(envp);

    while (1) {
        char choice;
        char except;
        int child_status;

        // Считываем выбор пользователя
        if (scanf(" %c", &choice) != 1) { // Добавлен пробел для пропуска whitespace
            fprintf(stderr, "Error reading input\n");
            break; // Выходим из цикла при ошибке чтения
        }

        // Проверяем, есть ли лишние символы во вводе
        if (scanf("%c", &except) == 1 && except != '\n') {
            fprintf(stderr, "Wrong param input\n");
            // Очищаем остаток буфера ввода
            while (getchar() != '\n');
            continue;
        }

        child_number++;
        char *child_info = NULL; // Инициализируем child_info

        switch (choice) {
            case '+': {
                pid_t proc = fork();
                if (proc == -1) {
                    perror("fork");
                    exit(1);
                } else if (proc == 0) {
                    child_info = getenv(ENV_VAR);
                    sprintf(counter_buf, "%d", child_number);
                    char *args[4];
                    // ОСТОРОЖНО: strcat изменяет первый аргумент. Лучше использовать snprintf.
                    char child_name_buf[30]; // Достаточно большой буфер
                    snprintf(child_name_buf, sizeof(child_name_buf), "CHILD_%02d", child_number);
                    args[0] = child_name_buf;
                    args[1] = argv[1];
                    args[2] = "+";
                    args[3] = NULL;
                    execve(child_info, args, envp);
                    perror("execve"); // execve возвращает только при ошибке
                    exit(1);
                }
                wait(&child_status);
                fprintf(stdout, "Child ended with status %d\n", child_status);
                break;
            }
            case '*': {
                pid_t proc = fork();
                if (proc == -1) {
                    perror("fork");
                    exit(1);
                } else if (proc == 0) {
                    child_info = find_var(envp);
                    if (child_info != NULL) {
                        fprintf(stdout, "Child path found: %s\n", child_info);
                        sprintf(counter_buf, "%d", child_number);
                        char *args[4];
                        char child_name_buf[30];
                        snprintf(child_name_buf, sizeof(child_name_buf), "CHILD_%02d", child_number);
                        args[0] = child_name_buf;
                        args[1] = argv[1];
                        args[2] = "*";
                        args[3] = NULL;
                        execve(child_info, args, envp);
                        perror("execve");
                        exit(1);
                    } else {
                        fprintf(stderr, "Environment variable %s not found or has no value\n", ENV_VAR);
                        exit(1);
                    }
                }
                wait(&child_status);
                fprintf(stdout, "Child ended with status %d\n", child_status);
                break;
            }
            case '&': {
                pid_t proc = fork();
                if (proc == -1) {
                    perror("fork");
                    exit(1);
                } else if (proc == 0) {
                    // Использование __environ небезопасно и нестандартно.
                    // Лучше использовать envp, который был передан в main.
                    child_info = find_var(envp);
                    if (child_info != NULL) {
                        fprintf(stdout, "Child path found: %s\n", child_info);
                        sprintf(counter_buf, "%d", child_number);
                        char *args[4];
                        char child_name_buf[30];
                        snprintf(child_name_buf, sizeof(child_name_buf), "CHILD_%02d", child_number);
                        args[0] = child_name_buf;
                        args[1] = argv[1];
                        args[2] = "&";
                        args[3] = NULL;
                        execve(child_info, args, envp);
                        perror("execve");
                        exit(1);
                    } else {
                        fprintf(stderr, "Environment variable %s not found or has no value\n", ENV_VAR);
                        exit(1);
                    }
                }
                wait(&child_status);
                fprintf(stdout, "Child ended with status %d\n", child_status);
                break;
            }
            case 'q': {
                fprintf(stdout, "Successfully ended\n");
                exit(0);
            }
            default: {
                fprintf(stderr, "Invalid choice: %c\n", choice);
                // Очищаем остаток буфера ввода
                while (getchar() != '\n');
            }
        }
        free(child_info); // Освобождаем child_info после использования (хотя он может быть NULL)
    }

    return 0;
}
