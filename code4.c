#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#define BUFFER_SIZE 5000     //размер буфера

int main(int argc, char **argv) {
    if (argc != 4) {
        perror("Wrong input data\n");
        return 1;
    }

    FILE *input_file = fopen(*(argv + 2), "r");
    FILE *output_file = fopen(*(argv + 3), "w");

    if (input_file == NULL || output_file == NULL) {     //проверка файлов
        perror("File opening error");
        return 1;
    }

    int pipe1[2];
    pid_t pid1, pid2, pid3;
    char buffer[BUFFER_SIZE];

    if (pipe(pipe1) < 0) {      // Создание неименованного канала и проверка
        perror("Pipe creation error");
        return 1;
    }

    pid1 = fork();  // Создание первого дочернего процесса

    if (pid1 == -1) {   //проверка первого дочернего процесса
        perror("Fork error");
        return 1;
    }

    if (pid1 == 0) { // Первый дочерний процесс
        close(pipe1[0]);
        char message[BUFFER_SIZE];
        fgets(message, BUFFER_SIZE, input_file);        //получение данных из файла
        write(pipe1[1], message, sizeof(message));  //запись данных в пайп
        // запись в канал
        close(pipe1[1]);
    } else {
        pid2 = fork();  // Создание второго дочернего процесса
        if (pid2 < 0) {
            perror("Fork error");
            return 1;
        }
        if (pid2 == 0) { //Второй дочерний процесс
            read(pipe1[0], buffer, sizeof(buffer));// чтение из канала
            close(pipe1[0]);
            char result[BUFFER_SIZE] = "";
            char num_str[BUFFER_SIZE] = "";
            int num;
            int len = strlen(buffer);
            int j, k = 0;

            for (int i = 0; i < len; i++) { //нахождение ответа на поставленную задачу
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
            //
            char res[BUFFER_SIZE];
            sprintf(res, "%s", result);
            write(pipe1[1], res, sizeof(buffer));   //запись ответа в канал
            close(pipe1[1]);
        } else {
            pid3 = fork();  //создание третьего дочернего процесса
            if (pid3 == -1) {
                perror("Fork error");
                return 1;
            }
            if (pid3 == 0) {
                char output[BUFFER_SIZE];
                read(pipe1[0], output, sizeof(output)); //чтение ответа из канала
                fprintf(output_file, "%s", output);     //вывод ответа в файл
                close(pipe1[0]);
            }
        }
    }

    return 0;
}
