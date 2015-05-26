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


void MigrationReceiver :: run()
{
}
