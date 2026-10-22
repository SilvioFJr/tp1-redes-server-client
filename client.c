#include "common.h"

void usage(char **argv) {
  printf("usage: %s <server IP> <server port>\n", argv[0]);
  printf("example: %s 127.0.0.1 51511\n", argv[0]);
  exit(EXIT_FAILURE);
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

  char userBuf[BUFSZ];
  char filebuffer[BUFSZ];
  char filename[BUFSZ];
  char message[2 * BUFSZ];
  bool fileSelected = false;

  ssize_t count;

  while (1) {

    // Lê o comando do usuário; encerra se a entrada acabar (EOF)
    if (fgets(userBuf, BUFSZ, stdin) == NULL) {
      strcpy(userBuf, "exit");
    }

    // Remove o caracter '\n' do fim da string
    changeLastCharacter(userBuf, '\n', '\0');

    if (containsWord(userBuf, "exit")) {
      // SAIR: envia pedido de desconexão para o servidor
      count = send(newSocket, userBuf, strlen(userBuf) + 1, 0);
      if (count != (ssize_t)(strlen(userBuf) + 1)) {
        logexit("send");
      }
      break;

    } else if (containsWord(userBuf, "select")) {
      // SELECIONAR ARQUIVO
      filename[0] = '\0';
      getFileName(userBuf, filename);

      if (filename[0] == '\0') {
        printf("file name not provided\n");
      } else if (!isFileTypeValid(filename)) {
        printf("%s not valid!\n", filename);
      } else {
        FILE *file = fopen(filename, "r");
        if (file == NULL) {
          printf("%s does not exist\n", filename);
        } else {
          // Lê o arquivo para o buffer (com espaço para o '\0')
          size_t n = fread(filebuffer, sizeof(char), BUFSZ - 1, file);
          filebuffer[n] = '\0';
          fclose(file);
          fileSelected = true;
          printf("%s selected\n", filename);
        }
      }

    } else if (containsWord(userBuf, "send file")) {
      // ENVIAR ARQUIVO: mensagem = nome do arquivo + conteúdo
      if (!fileSelected) {
        printf("no file selected!\n");
        continue;
      }

      snprintf(message, sizeof(message), "%s%s", filename, filebuffer);

      count = send(newSocket, message, strlen(message) + 1, 0);
      if (count != (ssize_t)(strlen(message) + 1)) {
        logexit("send");
      }

    } else {
      // Comando inválido: solicita a desconexão do cliente
      count = send(newSocket, userBuf, strlen(userBuf) + 1, 0);
      if (count != (ssize_t)(strlen(userBuf) + 1)) {
        logexit("send");
      }
      break;
    }
  }

  close(newSocket);
  exit(EXIT_SUCCESS);

}