#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 1024
#define READ     0
#define WRITE    1

char** eliminateToken(char **token, int size, int pos1, int pos2);

int main(int argc, char **argv) {
  char *buffer = malloc(sizeof(char) * BUF_SIZE);
  char **token = malloc(sizeof(char*) * BUF_SIZE);
  int c, status, position, fd;
  pid_t pid;

  do {
    position = 0;
    printf("$ ");
    while (true) {
      c = getchar();

      if (c == EOF || c == '\n') {
        buffer[position] = '\0';
        break;
      } else {
        buffer[position] = c;
      }
      position++;
    }
    token[0] = strtok(buffer, " ");
    for(position = 1; position < BUF_SIZE; position++) {
      token[position] = strtok(NULL, " ");
      if (token[position] == NULL) { break; }
    }
    token = realloc(token, sizeof(char*) * position);

    pid = fork();
    if (pid < 0) {
      perror("fork error");
      exit(EXIT_FAILURE);
    } else if (pid == 0) {
      for (int i = 0; i < position; i++) {
        if (!strncmp(token[i], ">>", 2)) {
          if ((fd = open(token[i+1], O_RDWR | O_APPEND | O_CREAT, 0666)) < 0) {
            perror("open error");
            close(fd);
            exit(EXIT_FAILURE);
          }
          dup2(fd, WRITE);
          token = eliminateToken(token, position, i, i+1);
          break;
        } else if (!strncmp(token[i], ">", 1)) {
          if ((fd = open(token[i+1], O_RDWR | O_TRUNC | O_CREAT, 0666)) < 0) {
            perror("open error");
            close(fd);
            exit(EXIT_FAILURE);
          }
          dup2(fd, WRITE);
          token = eliminateToken(token, position, i, i+1);
          break;
        }
      }
      if (execvp(token[0], token) == -1) {
        perror("exec error");
        exit(EXIT_FAILURE);
      }
      close(fd);
    } else {
      waitpid(pid, &status, WUNTRACED);
    }

    puts("");
    token = calloc(position, sizeof(char*) * BUF_SIZE);
  } while (!status);

  free(buffer);
  free(token);
  return EXIT_SUCCESS;
}

char** eliminateToken(char **token, int size, int pos1, int pos2) {
  char **eliminate_buffer = malloc(sizeof(char*) * BUF_SIZE);
  int position = 0;

  for (int i = 0; i < size; i++) {
    if (i != pos1 && i != pos2) {
      eliminate_buffer[position] = token[i];
      position++;
    }
  }

  return eliminate_buffer;
}
