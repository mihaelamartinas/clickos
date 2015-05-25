#ifndef CLICK_TCPSOCKET_HH
#define CLICK_TCPSOCKET_HH

CLICK_DECLS
class TCPSocket {
public:

#ifdef CLICK_USERLEVEL
	int fd;
#endif

	TCPSocket() { };
	TCPSocket(uint16_t port);

#ifdef CLICK_USERLEVEL
	TCPSocket(int fd) :
		fd(fd), tx(0), rx(0) {}
	int connect(uint16_t port, char *server_ip);
	int listen(int backlog);
	TCPSocket accept();

	ssize_t send(void *buf, size_t len);
	ssize_t recv(void *buf, size_t len);
	ssize_t sendNarrowed(void *buf, size_t len);
	ssize_t recvNarrowed(void *buf, size_t len);

	ssize_t getTx();
	ssize_t getRx();
#endif

private:
	ssize_t tx, rx;

};
CLICK_ENDDECLS
#endif
