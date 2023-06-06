#include "common.h"

void logexit(const char *msg) { // Apresenta o erro e sai do programa
    perror(msg);
    exit(EXIT_FAILURE);
}

int addrparse(const char *addrstr, const char *portstr, struct sockaddr_storage *storage) { // Função que passa o ponteiro pra struck
    if (addrstr == NULL || portstr == NULL) { // Se receber errado
        return -1;
    }


    uint16_t port = (uint16_t)atoi(portstr);
    // int port = PORT;
    if (port == 0) {
        return -1;
    }
    port = htons(port); // host to network short

    struct in_addr inaddr4; // 32-bit IPv4
    if (inet_pton(AF_INET, addrstr, &inaddr4)) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        addr4->sin_addr = inaddr4;
        return 0;
    }

    struct in6_addr inaddr6; // 128-bit IPv6
    if (inet_pton(AF_INET6, addrstr, &inaddr6)) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = port;
        memcpy(&(addr6->sin6_addr), &inaddr6, sizeof(inaddr6));
        return 0;
    }

    return -1;
}

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize) {
    int version;
    char addrstr[INET6_ADDRSTRLEN + 1] = "";
    uint16_t port;

    if (addr->sa_family == AF_INET) {
        version = 4;
        struct sockaddr_in *addr4 = (struct sockaddr_in *)addr;
        if (!inet_ntop(AF_INET, &(addr4->sin_addr), addrstr,
            INET6_ADDRSTRLEN + 1)) {
            logexit("ntop");
        }
        port = ntohs(addr4->sin_port); // network to host short
    } else if (addr->sa_family == AF_INET6) {
        version = 6;
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr;
        if (!inet_ntop(AF_INET6, &(addr6->sin6_addr), addrstr,
            INET6_ADDRSTRLEN + 1)) {
            logexit("ntop");
        }
        port = ntohs(addr6->sin6_port); // network to host short
    } else {
        logexit("unknown protocol family.");
    }
    if (str) {
        snprintf(str, strsize, "IPv%d %s %hu", version, addrstr, port);
    }
}

int server_sockaddr_init(const char *proto, const char *portstr, struct sockaddr_storage *storage) {
    uint16_t port = (uint16_t)atoi(portstr); // unsigned short
    if (port == 0) {
        return -1;
    }
    port = htons(port); // host to network short

    // inicializa com 0
    memset(storage, 0, sizeof(*storage));

    if (strcmp(proto, "v4") == 0) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        addr4->sin_addr.s_addr = INADDR_ANY;
        return 0;
    } else if (strcmp(proto, "v6") == 0) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = port;
        addr6->sin6_addr = in6addr_any;
        return 0;
    } else {
        return -1;
    }
}


void changeLastCharacter(char buffer[], char oldCharacter, char newCharacter) {
    size_t length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == oldCharacter) {
        buffer[length - 1] = newCharacter;
    }
}


bool containsWord(const char *str, const char *word) {
    const char *ptr = strstr(str, word);
    return (ptr != NULL);
}

void createFileWithMessage(const char *message, const char *filename) {
    // Abre o arquivo no modo de escrita
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error opening file.\n");
        return;
    }


    // Escreve a mensagem no arquivo
    fprintf(file, "%s", message);

    // Fecha o arquivo
    fclose(file);
}

bool fileExists(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file != NULL) {
        fclose(file);
        return true;
    } else {
        return false;
    }
}


int isFileTypeValid(const char *filename) {
    const char *validExtensions[] = {".txt", ".c", ".cpp", ".py", ".tex", ".java"};
    const int numExtensions = sizeof(validExtensions) / sizeof(validExtensions[0]);

    const char *fileExtension = strrchr(filename, '.');

    if (fileExtension != NULL) {
        for (int i = 0; i < numExtensions; i++) {
            if (strcmp(fileExtension, validExtensions[i]) == 0) {
                return 1; // Arquivo tem uma extensão válida
            }
        }
    }

    return 0; // Arquivo não tem uma extensão válida
}

void getFileName(const char* str, char* filename) {
    char* token;
    char tempStr[strlen(str) + 1];
    strcpy(tempStr, str);

    // Obtém o primeiro token
    token = strtok(tempStr, " ");

    // Procura o token correspondente ao nome do arquivo
    while (token != NULL) {
        if (strcmp(token, "select") != 0) {
            strcpy(filename, token);
            break;
        }
        token = strtok(NULL, " ");
    }
}

void handleMessage(const char *string, char *fileName, char *conteudo) {
    const char *delimitadores[] = {".txt", ".c", ".cpp", ".py", ".tex", ".java"};
    const int numDelimitadores = sizeof(delimitadores) / sizeof(delimitadores[0]);

    const char *posDelimitador = NULL;
    int i;

    // Encontrar o primeiro delimitador na string
    for (i = 0; i < numDelimitadores; i++) {
        posDelimitador = strstr(string, delimitadores[i]);
        if (posDelimitador != NULL) {
            break;
        }
    }

    if (posDelimitador != NULL) {
        // Tamanho do fileName
        size_t tamanhoFileName = posDelimitador - string + strlen(delimitadores[i]);

        // Copiar o fileName
        strncpy(fileName, string, tamanhoFileName);
        fileName[tamanhoFileName] = '\0';

        // Copiar o conteúdo
        strcpy(conteudo, posDelimitador + strlen(delimitadores[i]));
    } else {
        // Caso nenhum delimitador seja encontrado, consideramos a string inteira como o fileName e o conteúdo será vazio
        strcpy(fileName, string);
        conteudo[0] = '\0';
    }
}

void replaceNullDelimiter(char *str) {
    size_t length = strlen(str);

    for (size_t i = 0; i < length; i++) {
        if (str[i] == '\0') {
            strcpy(&str[i], "\\end");
            length = strlen(str);  // Atualiza o tamanho da string
            break;
        }
    }
}









