#include "common.h"

#include <pthread.h> // Criação das threads
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <ctype.h>

// Tamanho do buffer
#define BUFSZ 9000

// Tamanho da fila de conexões pendentes
#define maxQueueLimit 128


//COMANDOS
#define SELECT "SELECT"
#define SEND "SEND"
#define EXIT "EXIT"

void usage(int argc, char **argv) {
  printf("usage: %s <v4|v6> <server port>\n", argv[0]);
  printf("example: %s v4 51511\n", argv[0]);
  exit(EXIT_FAILURE);
}
void stringToUpper(char *str) {
  int i = 0;
  while (str[i] != '\0') {
    str[i] = toupper(str[i]);
    i++;
  }
}

struct client_data {
  int clientSocket;
  struct sockaddr_storage storage;
};

void *client_thread(void *data) {
  struct client_data *cdata = (struct client_data *)data;
  struct sockaddr *caddr = (struct sockaddr *)(&cdata->storage);

  char caddrstr[BUFSZ];
  addrtostr(caddr, caddrstr, BUFSZ);
  printf("[log] connection from %s\n", caddrstr);

  char buf[BUFSZ];
  memset(buf, 0, BUFSZ);
  size_t count = recv(cdata->clientSocket, buf, BUFSZ - 1, 0);

  if ((int)count > 500) { // here
    printf("\nSua mensagem ultrapassou 500 charracteres, você será desconectado\n");
    close(cdata->clientSocket);

    pthread_exit(EXIT_SUCCESS);
  } else {
    printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf);
    if (0 == strcmp(buf, "##kill\n")) {
      exit(0);
    }

    sprintf(buf, "remote endpoint: %.1000s\n", caddrstr);
    count = send(cdata->clientSocket, buf, strlen(buf) + 1, 0);
    if (count != strlen(buf) + 1) {
      logexit("send");
    }
    close(cdata->clientSocket);

    pthread_exit(EXIT_SUCCESS);
  }
}

int main(int argc, char **argv) {

  if (argc < 3) {
    usage(argc, argv);
  }

  struct sockaddr_storage storage;

  if (server_sockaddr_init(argv[1], argv[2], &storage) != 0) {
    usage(argc, argv);
  }

  int newSocket = socket(storage.ss_family, SOCK_STREAM, 0);
  if (newSocket == -1) {
    logexit("socket");
  }

  // Reutiliza o endereço da porta, mesmo se ele já estiver sendo utilizado...
  int enable = 1;
  if (setsockopt(newSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) != 0) {
    logexit("setsockopt");
  }

  struct sockaddr *addr = (struct sockaddr *)(&storage);

  if (bind(newSocket, addr, sizeof(storage)) != 0) {
    logexit("bind");
  }

  if (listen(newSocket, maxQueueLimit) != 0) {
    logexit("listen");
  }

  char addrstr[BUFSZ];
  addrtostr(addr, addrstr, BUFSZ);
  printf("bound to %s, waiting connections\n", addrstr);

  while (1) {
    struct sockaddr_storage clientStorage;
    struct sockaddr *caddr = (struct sockaddr *)(&clientStorage);
    socklen_t caddrlen = sizeof(clientStorage);

    int clientSocket = accept(newSocket, caddr, &caddrlen); // socket do cliente
    if (clientSocket == -1) {
      logexit("accept");
    }

    char caddrstr[BUFSZ];
    addrtostr(caddr, caddrstr, BUFSZ);
    printf("[log]: connection from %s\n", caddrstr);




    char buf[BUFSZ];
    char serverBuf[BUFSZ];
    memset(buf, 0, BUFSZ); // Inicia o buffer com 0 

    while (1) {
      char command[BUFSIZ];
      char message[BUFSIZ];
      char filename[BUFSIZ];

      int bytesReceived = recv(clientSocket, buf, BUFSZ - 1, 0);

      if (bytesReceived > 0) {

        //Trata a mensagem recebida
        printf("[receved]:  %s\n", buf);
        removeNullChar(buf);
        extractCommandAndMessage(buf, command, message);
        printf("[command]:  %s\n", command);
        printf("[message]:  %s\n", message);

        stringToUpper(command);

        //Desconectar cliente
        if (containsWord(command, EXIT)) {
          printf("connection closed\n");
          close(clientSocket);
          break;
        }
      }

      //guardar nome do arquivo......
      if (containsWord(command, SELECT)) {
        if (filename != NULL) {
          strcpy(filename, message);
          printf("[filename]:  %s\n", filename);
        }
      }



      // // RECEBER ARQUIVO
      // if (containsWord(command, "LOREM")) {
      //   // Lê o arquivo e envia ao servidor
      //   printf("Entrei no SEND>> %s\n", filename);

      //   // int n;

      //   // Verifica se o arquivo existe
      //   if (access(serverBuf, F_OK) == 0) {
      //     // Arquivo existe

      //     FILE *file = fopen("recebido.txt", "w");
      //     if (file == NULL) {
      //       printf("Error opening file\n");
      //       return -1;
      //     }
      //     // while ((n = read(clientSocket, buf, BUFSIZ + 1)) > 0) {
      //     // }
      //     fwrite(buf, sizeof(char), BUFSIZ, file);
      //     // if (n < 0) {
      //     //   logexit("Error reading from socket");
      //     // }

      //     fclose(file);

      //     printf("file %s overwritten\n", serverBuf);

      //   } else {

      //     //cria o novo arquivo
      //     FILE *file = fopen("recebido.txt", "w");
      //     if (file == NULL) {
      //       printf("Error opening file\n");
      //       return -1;
      //     }
      //     // while ((n = read(clientSocket, buf, BUFSIZ + 1)) > 0) {
      //     // }
      //     size_t elementsWritten = fwrite(buf, sizeof(char), BUFSIZ, file);

      //     if (elementsWritten != BUFSIZ) {
      //       fprintf(stderr, "Erro ao gravar os dados no arquivo.\n");
      //       exit(EXIT_FAILURE);
      //     }
      //     // if (n < 0) {
      //     //   logexit("Error reading from socket");
      //     // }
      //     fclose(file);

      //     printf("file %s received", serverBuf);

      //   }

      // }

      memset(buf, 0, BUFSZ);


      //SELECT
      // char *result = strstr(buf, "select");

      // if (result != NULL) {



      //   sprintf(buf, "File received successfully\n %.1000s\n", caddrstr);
      //   count = send(clientSocket, buf, strlen(buf) + 1, 0);
      //   // send(clientSocket, "File received successfully\n", strlen(buf) + 1, 0);

      //   // printf("[msg33333]: %s\n", buf);
      //   if (count != strlen(buf) + 1) {
      //     logexit("send");
      //   }

      // }



      // if(arquivo recebido){
      //   sprintf(buf, "remote endpoint: %.1000s\n", caddrstr);
      //   count = send(clientSocket, buf, strlen(buf) + 1, 0);
      // }

      // count = send(clientSocket, buf, strlen(buf) + 1, 0);


      // // }




      // Inicia a criação de threads

      // struct client_data * cdata = malloc(sizeof( * cdata));
      // if (!cdata) {
      //   logexit("malloc");
      // }
      // cdata -> clientSocket = clientSocket;
      // memcpy( &(cdata -> storage), & clientStorage, sizeof(clientStorage));

      // pthread_t tid;
      // pthread_create( & tid, NULL, client_thread, cdata);
    }

    printf("------------------ SERVIDOR DISPONÍVEL ---------------------------\n");

  }

  close(newSocket);
  exit(EXIT_SUCCESS);
}

