#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <ctype.h>

#define BUFSZ 1024

//COMANDOS
#define SELECT "SELECT"
#define SEND "SEND"
#define EXIT "EXIT"

void usage(char **argv) {
  printf("usage: %s <server IP> <server port>\n", argv[0]);
  printf("example: %s 127.0.0.1 51511\n", argv[0]);
  exit(EXIT_FAILURE);
}

void stringToUpper(char *str) {
  int i = 0;
  while (str[i] != '\0') {
    str[i] = toupper(str[i]);
    i++;
  }
}


int main(int argc, char **argv) {
  if (argc < 3) { // Verifica ip correto
    usage(argv);
  }

  struct sockaddr_storage storage; // Parseia qual o endereço do servidor, esse ponteiro vai armazenar
  if (addrparse(argv[1], argv[2], &storage) != 0) {
    usage(argv);
  }

  int newSocket = socket(storage.ss_family, SOCK_STREAM, 0); // Cria Socket, ss_family, é a família do IP
  if (newSocket == -1) { // Verifica se deu erro de Socket
    logexit("socket");
  }
  struct sockaddr *addr = (struct sockaddr *)(&storage); // Conecta
  if (connect(newSocket, addr, sizeof(storage)) != 0) { // Verifica se deu erro de conexão
    logexit("connect");
  }


  char addrstr[BUFSZ];
  addrtostr(addr, addrstr, BUFSZ); // Converte pra printar o endereço
  printf("connected to %s\n", addrstr);

  char userBuf[BUFSIZ];
  char filebuffer[BUFSIZ];
  char filename[BUFSIZ];

  size_t count;

  while (1) {

    //Recebe os dados do cliente
    fgets(userBuf, BUFSZ - 1, stdin);

    //Remove o caracter '\n' e coloca o '\0' no fim da string
    changeLastCharacter(userBuf, '\n', '\0');

    //SAIR
    if (containsWord(userBuf, "exit")) {
      // Envia pedido de desconexão para o servidor
      count = send(newSocket, userBuf, strlen(userBuf) + 1, 0);

      if (count != strlen(userBuf) + 1) { // Erro no envio
        logexit("send");
      }

      memset(userBuf, 0, BUFSZ);
      close(newSocket);
      break;
      
    } else if (containsWord(userBuf, "select")) {
      //SELECIONAR ARQUIVO

      getFileName(userBuf, filename);

      if (isFileTypeValid(filename)) {
        FILE *file = fopen(filename, "r");
        if (file != NULL) {
          //Lê o arquivo, guarda os dados no buffer de arquivos
          fread(filebuffer, sizeof(char), BUFSZ, file);
          fclose(file);

          printf("%s selected\n", filename);

        } else {
          if (filename == NULL) {
            printf("file name not provided\n");
          } else {
            printf("%s does not exist\n", filename);
          }
        }
      } else {
        printf("%s not valid!\n", filename);
      }

      memset(userBuf, 0, BUFSZ);

    } else if (containsWord(userBuf, "send file")) {
      //ENVIAR ARQUIVO
      if (filebuffer != NULL) {
        char message[BUFSIZ];
        strcat(message, filename);
        strcat(message, filebuffer);

        count = send(newSocket, message, strlen(message) + 1, 0); // Envia dados para o servidor

        if (count != strlen(message) + 1) { // Erro no envio
          logexit("send");
        }
      } else {
        printf("no file selected!\n");
      }
      memset(userBuf, 0, BUFSZ);
    } else {
      //Se não houver comando válido, solicita a desconexão do cliente
      count = send(newSocket, userBuf, strlen(userBuf) + 1, 0);

      if (count != strlen(userBuf) + 1) { // Erro no envio
        logexit("send");
      }

      memset(userBuf, 0, BUFSZ);
      close(newSocket);
      break;
    }

  }

  exit(EXIT_SUCCESS);

}