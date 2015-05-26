#ifndef CLICK_MIGRATION_SENDER_HH
#define CLICK_MIGRATION_SENDER_HH

#include <click/migrationactions.hh>
CLICK_DECLS

class MigrationSender : public MigrationActions {
	public:
		MigrationSender(uint16_t remotePort, uint16_t listenPort, char *hostname) :
			MigrationActions(remotePort, listenPort, hostname) {};
}

CLICK_ENDDECLS
#endif /* CLICK_MIGRATION_SENDER_HH */
