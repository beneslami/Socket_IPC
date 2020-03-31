TARGET: exe
exe:
	gcc Multiplexer_server.c -o server
	gcc client.c -o client

clean:
	rm -rf server client client.dSYM server.dSYM
