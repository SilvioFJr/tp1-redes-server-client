# TP1 Redes — Cliente/Servidor TCP

Trabalho prático 1 da disciplina de **Redes de Computadores (UFMG)**: transferência de arquivos de texto entre um cliente e um servidor TCP em **C**, com suporte a **IPv4 e IPv6**.

## Como funciona

O cliente lê comandos do teclado:

| Comando | O que faz |
|---|---|
| `select <arquivo>` | Seleciona um arquivo local (`.txt`, `.c`, `.cpp`, `.py`, `.tex`, `.java`) |
| `send file` | Envia o arquivo selecionado ao servidor |
| `exit` | Encerra a conexão |

O servidor grava o arquivo recebido no diretório atual (`received` ou `overwritten`). Qualquer outro comando encerra a conexão.

## Compilar e executar

```bash
make
./server v4 51511            # ou: ./server v6 51511
./client 127.0.0.1 51511     # ou: ./client ::1 51511
```

```text
select file.txt
send file
exit
```

## Estrutura

- `common.c/.h` — parsing de endereços (IPv4/IPv6), utilitários de arquivo e de mensagem
- `server.c` — aceita uma conexão e grava os arquivos recebidos
- `client.c` — interpreta os comandos e envia os arquivos
