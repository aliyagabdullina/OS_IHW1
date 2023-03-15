#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define FIFO_FILE "/tmp/myfifo"  //расположение файла
#define BUFFER_SIZE 5000        //размер буфера

int main(int argc, char **argv) {
    if (argc != 4) {
        perror("Wrong input data\n");
        return 1;
    }

    FILE *input_file = fopen(*(argv + 2), "r");
    FILE *output_file = fopen(*(argv + 3), "w");

    if (input_file == NULL || output_file == NULL) {
        perror("File opening error");
        return 1;
    }

    int pipe1;
    pid_t pid1, pid2;
    char buffer[BUFFER_SIZE];
    mkfifo(FIFO_FILE, 0666); // Создание именованного канала

    pid1 = fork();  // Создание первого дочернего процесса

    if (pid1 == -1) {
        printf("Fork error");
        return 0;
    }

    if (pid1 == 0) {
        char message[BUFFER_SIZE];
        fgets(message, BUFFER_SIZE, input_file);   //получение данных из файла
        pipe1 = open(FIFO_FILE, O_WRONLY);
        if (pipe1 == -1) {
            printf("Error opening named pipe");
            return 0;
        }
        write(pipe1, message, sizeof(message));     //запись аднных в пайп
        close(pipe1);
    } else {
        pid2 = fork();          //создание второго дочернего процесса
        if (pid2 == -1) {
            printf("Fork error");
            return 0;
        }
        if (pid2 == 0) {
            pipe1 = open(FIFO_FILE, O_RDONLY);
            if (pipe1 == -1) {
                printf("Error opening named pipe");
                return 0;
            }
            read(pipe1, buffer, sizeof(buffer));    //чтение данных из пайпа
            char result[BUFFER_SIZE] = "";
            char num_str[BUFFER_SIZE] = "";
            int num;
            int len = strlen(buffer);
            int j, k = 0;

            for (int i = 0; i < len; i++) {     //вычисление ответа
                if (isdigit(buffer[i])) {
                    num_str[j++] = buffer[i];
                } else {
                    if (j > 0) {
                        num_str[j] = '\0';
                        num = atoi(num_str);
                        if (k > 0) {
                            strcat(result, "+");
                        }
                        sprintf(num_str, "%d", num);
                        strcat(result, num_str);
                        j = 0;
                        k++;
                    }
                }
            }

            if (j > 0) {
                num_str[j] = '\0';
                num = atoi(num_str);
                if (k > 0) {
                    strcat(result, "+");
                }
                sprintf(num_str, "%d", num);
                strcat(result, num_str);
            }
            char res[BUFFER_SIZE];
            sprintf(res, "%s", result);
            close(pipe1);
            pipe1 = open(FIFO_FILE, O_WRONLY);
            write(pipe1, res, sizeof(res)); //запись ответа в канал
            close(pipe1);
        } else {
            pid1 = fork();
            if (pid1 == -1) {
                printf("Fork error");
                return 0;
            }
            if (pid1 == 0) {
                char output[5000];
                pipe1 = open(FIFO_FILE, O_RDONLY);
                if (pipe1 == -1) {
                    printf("Error opening named pipe");
                    return 0;
                }
                read(pipe1, output, sizeof(output));   //чтение ответа из канала
                fprintf(output_file, "%s", output);     //вывод ответа в файл
                close(pipe1);
            }
        }
    }

    return 0;
}
