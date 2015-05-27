#include <click/config.h>
#include <click/migrationreceiver.hh>
#include <click/args.hh>
#include <click/straccum.hh>
#include <click/error.hh>
#include <click/timer.hh>
#include <click/router.hh>

int MigrationReceiver :: connectToMachine()
{
	int ret = 0;

	TCPSocket socketTmp;
	TCPSocket listeningSocket(port);

	/* accept connection from the remote machine */
	if ( listeningSocket.listen() < 0) {
		ret = -1;
		goto out;
	}

	socketTmp = listeningSocket.accept();
	if (socket == NULL) {
		click_chatter("Error accepting remote connection\n");
		ret = -1;
		goto free_socket;
	}

	socket = &socketTmp;
	click_chatter("Accepted connection from sender\n");

free_socket:
	listeningSocket.close();
out:
	return ret;
}

void MigrationReceiver :: run(Map *tcp_map, Map *udp_map, IPRewriterHeap *heap)
{
	ssize_t recv_size;
	Protocol::MigrationHeader header;
	
	while(true) {
		recv_size = socket->recv(&header, sizeof(Protocol::MigrationHeader));
		if (recv < 0) {
			click_chatter("Error reading message header\n");
			goto out;
		}

		switch(header.type) {
			case Protocol::MigrationHeader::T_INFO:
										   print_info_header(header);
										   goto out;
										   break;
			case Protocol::MigrationHeader::T_MAP_TCP:
										   break;
			case Protocol::MigrationHeader::T_MAP_UDP:
										   break;
			case Protocol::MigrationHeader::T_HEAP_GUARANTEED:
										   break;
			case Protocol::MigrationHeader::T_HEAP_BEST_EFFORT:
										   goto out;
										   break;
		}
	}
out:
	socket->close();
}
