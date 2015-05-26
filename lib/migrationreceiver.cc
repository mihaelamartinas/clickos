#include <click/config.h>
#include <click/migrationsender.hh>
#include <click/args.hh>
#include <click/straccum.hh>
#include <click/error.hh>
#include <click/timer.hh>
#include <click/router.hh>

int MigrationReceiver :: connectToMachine()
{
	int ret = 0;

	TCPSocket listeningSocket(port);

	/* accept connection from the remote machine */
	if ( listeningSocket.listen() < 0) {
		ret = -1;
		goto out;
	}

	socket = listeningSocket.accept();
	if (socket == NULL) {
		click_chatter("Error accepting remote connection\n");
		ret = -1;
		goto free_socket;
	}

free_socket:
	listeningSocket.close();
out:
	return ret;
}
