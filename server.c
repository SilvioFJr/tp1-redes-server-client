#include "common.h"

void usage(char **argv) {
  printf("usage: %s <v4|v6> <server port>\n", argv[0]);
  printf("example: %s v4 51511\n", argv[0]);
  exit(EXIT_FAILURE);
}

struct client_data {
  int clientSocket;
  struct sockaddr_storage storage;
};

int main(int argc, char **argv) {

  if (argc < 3) {
    usage(argv);
  }

  struct sockaddr_storage storage;

  if (server_sockaddr_init(argv[1], argv[2], &storage) != 0) {
    usage(argv);
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

  // Inicia a escuta por conexões
  if (listen(newSocket, MAX_CLIENTS) == -1) {
    logexit("listen");
  }

  char addrstr[BUFSZ];
  addrtostr(addr, addrstr, BUFSZ);
  printf("bound to %s, waiting connections\n", addrstr);

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

  char buf[2 * BUFSZ];
  char fileName[2 * BUFSZ];
  char conteudo[2 * BUFSZ];
  while (1) {
    ssize_t received = recv(clientSocket, buf, sizeof(buf) - 1, 0);
    if (received <= 0) { // Cliente desconectou ou erro
      printf("connection closed\n");
      close(clientSocket);
      break;
    }
    buf[received] = '\0';

    handleMessage(buf, fileName, conteudo);

    if (conteudo[0] != '\0') {
      // RECEBER ARQUIVO (só no diretório atual: recusa caminhos)
      if (strchr(fileName, '/') != NULL) {
        printf("invalid file name %s\n", fileName);
        continue;
      }
      bool existed = fileExists(fileName);
      createFileWithMessage(conteudo, fileName);
      printf(existed ? "file %s overwritten\n" : "file %s received\n", fileName);
    } else { // Desconecta o cliente em caso de exit ou comandos errados
      printf("connection closed\n");
      close(clientSocket);
      break;
    }
  }

  close(newSocket);
  exit(EXIT_SUCCESS);
}

