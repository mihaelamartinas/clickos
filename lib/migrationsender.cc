#include <click/config.h>
#include <click/migrationsender.hh>
#include <click/args.hh>
#include <click/straccum.hh>
#include <click/error.hh>
#include <click/timer.hh>
#include <click/router.hh>

CLICK_DECLS
int MigrationSender :: connectToMachine()
{
	int ret;

	socket = new TCPSocket(TCPSocket :: ANY_PORT);

	/* connect to the remote machine */
	return socket->connect(port, hostname);
}

void MigrationSender :: run()
{
}


CLICK_ENDDECLS
