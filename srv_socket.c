#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h>

char buffer[1024];
pthread_t tids[100];
int thds;

int pid;
static void * handle(void *);
int main(int argc, char *argv[])
{
	int srv_sock, cli_sock;
	int port_num, ret;
	struct sockaddr_in addr;
	int len;

	// arg parsing
	if (argc != 2) {
		printf("usage: srv port\n");
		return 0;
	}
	port_num = atoi(argv[1]);

	// socket creation
	srv_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (srv_sock == -1) {
		perror("Server socket CREATE fail!!");
		return 0;
	}

	// addr binding
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htons (INADDR_ANY); // 32bit IPV4 addr that not use static IP addr
	addr.sin_port = htons (port_num); // using port num
	
	ret = bind (srv_sock, (struct sockaddr *)&addr, sizeof(addr));
	
	if (ret == -1) {
		perror("BIND error!!");
		close(srv_sock);
		return 0;
	}

	for (;;) {
	// Listen part
	ret = listen(srv_sock, 0);

	if (ret == -1) {
		perror("LISTEN stanby mode fail");
		close(srv_sock);
		return 0;
	}

	// Accept part ( create new client socket for communicate to client ! )
	cli_sock = accept(srv_sock, (struct sockaddr *)NULL, NULL); // client socket
	if (cli_sock == -1) {
		perror("cli_sock connect ACCEPT fail");
		close(srv_sock);
	}
	thds++;
	// cli handler
	pthread_create(&tids[thds], NULL, handle, &cli_sock);
	} // end for
	return 0;
}

static void * handle(void * arg)
{
	int cli_sockfd = *(int *)arg;
	int ret = -1;
	char *recv_buffer = (char *)malloc(1024);
	char *send_buffer = (char *)malloc(1024);
	char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
           
	/* get peer addr */
	struct sockaddr peer_addr;
	socklen_t peer_addr_len;
	memset(&peer_addr, 0, sizeof(peer_addr));
	peer_addr_len = sizeof(peer_addr);
	ret = getpeername(cli_sockfd, &peer_addr, &peer_addr_len);
	ret = getnameinfo(&peer_addr, peer_addr_len, 
		hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), 
		NI_NUMERICHOST | NI_NUMERICSERV); 

	if (ret != 0) {
		ret = -1;
		pthread_exit(&ret);
	}
	/* read from client host:port */

	while (1) {
	int len = 0;

	printf("from client ----\n");
	memset(recv_buffer, 0, sizeof(recv_buffer));
	len = recv(cli_sockfd, recv_buffer, sizeof(recv_buffer), 0);
	if (len == 0) continue;
	printf("%s\n len:%d\n", recv_buffer, len);
	memset(send_buffer, 0, sizeof(send_buffer));
	sprintf(send_buffer, "[%s:%s]%s len:%d\n", 
				hbuf, sbuf, recv_buffer, len);
	len = strlen(send_buffer);

	ret = send(cli_sockfd, send_buffer, len, 0);
	if (ret == -1) break;
	printf("----\n");
	fflush(NULL);

	}
	close(cli_sockfd);
	ret = 0;
	pthread_exit(&ret);
}

