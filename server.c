#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<unistd.h>

#define SOCEKT_NAME "~/Desktop/socket"
#define BUFFER_SIZE 128

int main(int argc, char **argv){
  struct sockaddr_un name;

  #if 0
    struct sockaddr_un {
      sa_family_t sun_family;    /*AF_UNIX*/
      char sun_path[100];        /*path name*/
    };
  #endif

  int ret;
  int connection_socket;
  int data_socket;
  int result;
  int data;
  char buffer[BUFFER_SIZE];

  /* In case the program exited inadvertently on the last run,
      remove the socket */
  unlink(SOCEKT_NAME); // A precaution to avoid using two sockets with the same name

  //create master socket
  connection_socket = socket(AF_UNIX, SOCK_STREAM, 0);
  if(connection_socket == -1){
    perror("socket");
    exit(EXIT_FAILURE);
  }
  printf("master socket created\n");

  memset(&name, 0, sizeof(struct sockaddr_un));
  name.sun_family = AF_UNIX;
  strncpy(name.sun_path, SOCEKT_NAME, sizeof(name.sun_path) - 1);
  ret = bind(connection_socket, (const struct sockaddr *) &name, sizeof(struct sockaddr_un));
  if(ret == -1){
    perror("bind");
    exit(EXIT_FAILURE);
  }
  printf("bind succedded\n");

  ret = listen(connection_socket, 20); //20 is the number of clients simultaneously try to connect.
  if(ret == -1){
    perror("listen");
    exit(EXIT_FAILURE);
  }

  for(;;){
    printf("waiting of accept() system call\n");
    data_socket = accept(connection_socket, NULL, NULL);
    if(data_socket == -1){
      perror("accpet");
      exit(EXIT_FAILURE);
    }
    printf("connection accepted from client\n");
    result = 0;

    for(;;){
      memset(buffer, 0, BUFFER_SIZE);
      printf("waiting for data from the client\n");
      ret = read(data_socket, buffer, BUFFER_SIZE); //returns the number of bytes recieved
      if(ret == -1){
        perror("read");
        exit(EXIT_FAILURE);
      }
      memcpy(&data, buffer, sizeof(int));
      if(data == 0) break;
      result += data;
    }
    memset(buffer, 0, BUFFER_SIZE);
    sprintf(buffer, "Result = %d", result);
    printf("sending final result back to the client\n");
    ret = write(data_socket, buffer, BUFFER_SIZE);
    if(ret == -1){
      perror("read");
      exit(EXIT_FAILURE);
    }
    close(data_socket);
  }
  close(connection_socket);
  printf("connection closed\n");
}
