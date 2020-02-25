TARGET: exe
exe:
	gcc -g server.c -o server
	gcc -g client.c -o client

clean:
	rm -rf server client client.dSYM server.dSYM
