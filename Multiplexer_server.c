#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<unistd.h>

#define SOCKET_NAME "/tmp/socket"
#define BUFFER_SIZE 128
#define MAX_CLIENT_SUPPORTED   32

int monitored_fd_set[MAX_CLIENT_SUPPORTED]; /*set of socket file descriptor for multiple simultaneous connection */
int client_result[MAX_CLIENT_SUPPORTED] = {0}; /*Intermediate result of each connection is kept here */
int comm_socket_fd = -1;

static void initialize_monitor_fd_set(){/* remove all FDs, if any from the array*/
  for(int i=0; i<MAX_CLIENT_SUPPORTED; i++)
    monitored_fd_set[i] = -1;
}

static void add_monitor_fd_set(int skt_fd){/* add new FD to the array*/
  for(int i=0; i<MAX_CLIENT_SUPPORTED; i++){
    if(monitored_fd_set[i] != -1) continue;
    monitored_fd_set[i] = skt_fd;
    break;
  }
}

static void remove_monitor_fd_set(int skt_fd){/* remove the FD from the array*/
  for(int i=0; i<MAX_CLIENT_SUPPORTED; i++){
    if(monitored_fd_set[i] != skt_fd) continue;
    monitored_fd_set[i] = -1;
    break;
  }
}

static void refresh_fd_set(fd_set *fd_set_ptr){/* Clone all the FDs in the array into fd_set data structure*/
  FD_ZERO(fd_set_ptr);
  for(int i=0; i<MAX_CLIENT_SUPPORTED; i++){
    if(monitored_fd_set[i] != -1){
        FD_SET(monitored_fd_set[i], fd_set_ptr); /* copy monitored_fd_set[i] to fd_set_ptr */
    }
  }
}

static int get_max_fd(){/* get the numerical max value among all FDs which server is monitoring*/
  int max = -1;
  for(int i=0; i<MAX_CLIENT_SUPPORTED; i++){
      if(monitored_fd_set[i]> max){
        max = monitored_fd_set[i];
      }
  }
  return max;
}

int main(int argc, char **argv){
  struct sockaddr_un name;
  int ret;
  int connection_socket;
  int data_socket;
  int result;
  int data;
  fd_set readfds;
  int comm_socket_fd, i;
  char buffer[BUFFER_SIZE];
  initialize_monitor_fd_set();
  add_monitor_fd_set(0); // filde descriptor for console = monitored_fd_set[0]
  /* In case the program exited inadvertently on the last run,remove the socket */
  unlink(SOCKET_NAME); // A precaution to avoid using two sockets with the same name
  //create master socket
  connection_socket = socket(AF_UNIX, SOCK_STREAM, 0);
  if(connection_socket == -1){
    perror("socket");
    exit(EXIT_FAILURE);
  }
  printf("master socket created\n");

  memset(&name, 0, sizeof(struct sockaddr_un));
  name.sun_family = AF_UNIX;
  strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);
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
  add_monitor_fd_set(connection_socket); /* add master socket FD to the array, connection_socket = monitored_fd_set[1]*/

  for(;;){

    refresh_fd_set(&readfds); /* copy the entire monitored FDs to readfds */
    printf("waiting of accept() system call\n");
    printf("$\n");
    select(get_max_fd()+1, &readfds, NULL, NULL, NULL);/* server process gets blocked here. OS keeps the process blocked
    until the connection initiation request or data requests arrives on any of the FDs in the readfds.
    */

    if(FD_ISSET(connection_socket, &readfds)){  /* connection initiaztion part */
      printf("New connection recieved\n");
      data_socket = accept(connection_socket, NULL, NULL);
      if(data_socket == -1){
        perror("accpet");
        exit(EXIT_FAILURE);
      }
      printf("connection accepted\n");
      add_monitor_fd_set(data_socket);
    }

    else if(FD_ISSET(0, &readfds)){/* input from console */
      char op[BUFFER_SIZE];
      ret = read(0, op, BUFFER_SIZE -1);
      op[strcspn(op, "\r\n")] = 0; // flush new line
      if(ret < 0){
          printf("Insert valid operation\n");
          break;
      }
      op[ret] = 0;
      printf("input from console:\n%s\n", op);
    }

    else { /* data strives on some other client's FDs. Find the client which has sent us the data request */

      for(int i=2; i< MAX_CLIENT_SUPPORTED; i++){
        if(FD_ISSET(monitored_fd_set[i], &readfds)){
          printf("1\n");
          comm_socket_fd = monitored_fd_set[i];
          memset(buffer, 0 , BUFFER_SIZE);
          ret = read(comm_socket_fd, buffer, sizeof(int)); //returns the number of bytes recieved
          if(ret == -1){
            perror("read");
            exit(EXIT_FAILURE);
          }
          memcpy(&data, buffer, sizeof(int));
          /* substitute any operation here */
          if(data == 0){
            memset(buffer, 0 , BUFFER_SIZE);
            sprintf(buffer, "Result = %d", client_result[i]);
            printf("sending final result back to the client\n");
            ret = write(data_socket, buffer, BUFFER_SIZE);
            if(ret == -1){
              perror("read");
              exit(EXIT_FAILURE);
            }
            close(comm_socket_fd);
            client_result[i] = 0;
            remove_monitor_fd_set(comm_socket_fd);
            break;
          }
          client_result[i] += data;
          printf("%d\n", client_result[i]);
          break;
          /* substitute any operation here */
        }
      }
    }
    printf("#\n");
  }

  close(connection_socket);
  remove_monitor_fd_set(connection_socket);
  printf("connection closed...\n");
  unlink(SOCKET_NAME);
  exit(EXIT_SUCCESS);
}
