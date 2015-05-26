#ifndef CLICK_MIGRATION_ACTIONS_HH
#define CLICK_MIGRATION_ACTIONS_HH

#include <click/tcpsocket.hh>
#include <stdlib.h>

CLICK_DECLS

class MigrationActions {
	public:
		MigrationActions() : port(0), hostname(NULL) {};
		MigrationActions(uint16_t port, char *hostname) :
			port(port), hostname(hostname) {};
		virtual void run() {};
		virtual int connectToMachine() { return 0; };
	protected:
		uint16_t port;
		char *hostname;
		TCPSocket *socket;
};

CLICK_ENDDECLS
#endif /* CLICK_MIGRATION_ACTIONS_HH */
