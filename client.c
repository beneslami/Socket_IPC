#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<unistd.h>

#define SOCEKT_NAME "/tmp/socket"
#define BUFFER_SIZE 128

int main(int argc, char **argv){
  struct sockaddr_un addr;
  int ret;
  int i;
  int data_socket;
  char buffer[BUFFER_SIZE];

  data_socket = socket(AF_UNIX, SOCK_STREAM, 0);
  if(data_socket == -1){
    perror("socket");
    exit(EXIT_FAILURE);
  }
  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCEKT_NAME, sizeof(addr.sun_path));

  ret = connect(data_socket, (const struct sockaddr*) &addr, sizeof(struct sockaddr_un));
  if(ret == -1){
    perror("connect");
    exit(EXIT_FAILURE);
  }

  /*send loop*/
  do{
    printf("Enter the number to send to the server\n");
    scanf("%d", &i);
    ret = write(data_socket, &i, sizeof(int));
    if(ret == -1){
      perror("write");
      exit(EXIT_FAILURE);
    }
    printf("number of bytes sent = %d, data sent = %d\n", ret, i);
  }while(i);

  memset(buffer, 0, BUFFER_SIZE);
  ret = read(data_socket, buffer, BUFFER_SIZE);
  if(ret == -1){
    perror("read");
    exit(EXIT_FAILURE);
  }
  printf("recieved from server : %s\n", buffer);
  close(data_socket);
  exit(EXIT_SUCCESS);
}
