network: srv cli

srv: srv_socket.c
	gcc -o $@ $^ -g -pthread

cli: cli_socket.c
	gcc -o $@ $^ -g

clean:
	rm srv cli
