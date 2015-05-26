#ifndef CLICK_MIGRATION_RECEIVER_HH
#define CLICK_MIGRATION_RECEIVER_HH

#include <click/migrationactions.hh>
CLICK_DECLS

class MigrationReceiver : public MigrationActions {
	public:
		MigrationReceiver(uint16_t remotePort, uint16_t listenPort, char *hostname) :
			MigrationActions(remotePort, listenPort,  hostname) {};

}

CLICK_ENDDECLS
#endif /* CLICK_MIGRATION_SENDER_HH */
