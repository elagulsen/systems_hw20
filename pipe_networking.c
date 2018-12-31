#include "pipe_networking.h"


static void sighandler(int signo) {
  remove("wkp");
  exit(0);
}

/*=========================
  server_handshake
  args: int * to_client

  Performs the client side pipe 3 way handshake.
  Sets *to_client to the file descriptor to the downstream pipe.

  returns the file descriptor for the upstream pipe.
  =========================*/
int server_handshake(int *to_client) {
  signal(SIGINT, sighandler);
  printf("SERVER HANDSHAKE\n");
  
  mkfifo("wkp", 0644);
  printf("Made a well known pipe.\n");
  
  int to_server = open("wkp", O_RDONLY);
  char private[HANDSHAKE_BUFFER_SIZE];
  read(to_server, private, HANDSHAKE_BUFFER_SIZE);
  remove("wkp"); //unnamed pipe now
  printf("Read the message %s and removed the well known pipe.\n", private);
  
  int from_server = open(private, O_WRONLY);
  write(from_server, ACK, HANDSHAKE_BUFFER_SIZE);
  printf("Transmitted %s to the client.\n", ACK);
  
  char conf[HANDSHAKE_BUFFER_SIZE];
  read(to_server, conf, HANDSHAKE_BUFFER_SIZE);
  printf("Received a confirmation: %s.\n\n", conf);
  
  char * message = malloc(BUFFER_SIZE);
  
  while (1) {
    read(to_server, message, BUFFER_SIZE);
    printf("Received a message: %s.\n", message);
    printf("Processing it... Separating the message by its first space.\n");
    char new_mes[BUFFER_SIZE];
    strcpy(new_mes, strcat(message, "!!!"));
    printf("New message: %s.\n", new_mes);
    write(from_server, new_mes, sizeof(new_mes));
    printf("Sent the message.\n\n");
  }
  
  to_client = &from_server;
  return to_server;
}


/*=========================
  client_handshake
  args: int * to_server

  Performs the client side pipe 3 way handshake.
  Sets *to_server to the file descriptor for the upstream pipe.

  returns the file descriptor for the downstream pipe.
  =========================*/
int client_handshake(int *to_server) {
  printf("CLIENT HANDSHAKE\n");
  mkfifo("client_p", 0644);
  printf("Made the client pipe.\n");
  
  *to_server = open("wkp", O_WRONLY);
  write(*to_server, "client_p", HANDSHAKE_BUFFER_SIZE);
  printf("Transmitted the name of the client pipe to the well known pipe.\n");
  
  int from_server = open("client_p", O_RDONLY);
  char message[HANDSHAKE_BUFFER_SIZE];
  read(from_server, message, HANDSHAKE_BUFFER_SIZE);
  printf("Received the message %s\n", message);
  
  remove("client_p"); //unnamed pipe now
  write(*to_server, message, HANDSHAKE_BUFFER_SIZE);
  printf("Messaged the server.\n\n");
  
  while (1) {
    printf("Please enter a message: \n");
    char mes[BUFFER_SIZE];
    fgets(mes, BUFFER_SIZE, stdin);
    char * mes2 = mes;
    strcpy(mes2, strsep(&mes2, "\n"));

    write(*to_server, mes, BUFFER_SIZE);
    printf("Sent your message, \"%s\", to the server.\n", mes);

    char new_mes[BUFFER_SIZE];
    read(from_server, new_mes, BUFFER_SIZE);
    printf("Received the message %s.\n\n", new_mes);
  }
  
  return from_server;
}
