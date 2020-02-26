The implementation of unix domain socket

Server:
socket() is called master socket file descriptor and responsible for creating handlers corresponded to the incoming client's
connection initiation request. Once the request arrives, the socket() function forks a handler function for
the client to communicate with the server. Each client request has its own handler function inside the server.
The bind() function dictates the OS  the criteria of receiving data. In other word, bind syscall tells the OS
that if the sender process (which is in the same machine as receiver) sends the data destined to socket "~/Desktop/socket",
then such data needs to be delivered to this process
accept() and read() system calls are blocking ones.

-------------------
The main drawback of the server.c is that it queues more than one incoming client request. So in Multiplexer_server.c this issue is handled by using select() system call. The data structure df_set is used to keep track of all simultaneous incoming requests from clients.
