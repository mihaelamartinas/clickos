#include <click/config.h>
#include <click/tcpsocket.hh>
#include <click/args.hh>
#include <click/straccum.hh>
#include <click/error.hh>
#include <click/timer.hh>
#include <click/router.hh>

#ifdef CLICK_USERLEVEL
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

CLICK_DECLS

#ifdef CLICK_USERLEVEL
TCPSocket::TCPSocket(uint16_t port)
{
	struct sockaddr_in local_addr;
	int ret;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < -1) {
		click_chatter("Error creating socket on port %d\n", port);
		return;
	}
	
	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = INADDR_ANY;
	local_addr.sin_port = htons(port);

	ret = bind(fd, (struct sockaddr *)&local_addr, sizeof(local_addr));
	if (ret < 0) {
		click_chatter("Error: unable to bind\n");
		return;
	}
	tx = rx = 0;
}

int TCPSocket :: connect(uint16_t port, char *server_ip)
{
	struct sockaddr_in server_addr;
	int ret;

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(server_ip);
	server_addr.sin_port = htons(port);

	ret = ::connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (ret < 0)
		click_chatter("Error: failed to connect to server %s\n", server_ip);

	return ret;
}

TCPSocket TCPSocket :: accept()
{
	int accepted_fd = ::accept(fd, NULL, NULL);

	if (accepted_fd < 0) {
		click_chatter("Error: Failed to accept connections\n");
		return TCPSocket();
	}

	return TCPSocket(accepted_fd);

}

int TCPSocket :: listen(int backlog)
{
	int ret = ::listen(fd, backlog);

	if (ret < 0)
		return -errno;

	return ret;
}


ssize_t TCPSocket :: send(void *buf, size_t len)
{
	ssize_t count = ::send(fd, buf, len, 0);
	if (count < 0)
		click_chatter("Error: unable to send data\n");

	tx += count;

	return count;
}

ssize_t TCPSocket :: recv(void *buf, size_t len)
{
	ssize_t count = ::recv(fd, buf, len, 0);
	if (count < 0)
		click_chatter("Error: unable to receive data\n");

	rx += count;

	return count;
}

ssize_t TCPSocket :: getTx()
{
	return tx;
}

ssize_t TCPSocket :: getRx()
{
	return rx;
}

#endif

CLICK_ENDDECLS
EXPORT_ELEMENT(TCPSocket)
