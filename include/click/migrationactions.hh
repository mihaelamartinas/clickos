#ifndef CLICK_MIGRATION_ACTIONS_HH
#define CLICK_MIGRATION_ACTIONS_HH

#include <click/tcpsocket.hh>

CLICK_DECLS

class MigrationActions {
	public:
		MigrationActions(uint16_t port, char *hostname) :
			port(port), hostname(hostname) {};
		virtual void run() = 0;
		virtual int connectToMachine() = 0;
	protected:
		uint16_t port;
		char *hostname;
		TCPSocket socket;
}

CLICK_ENDDECLS
#endif /* CLICK_MIGRATION_ACTIONS_HH */
