#include <click/config.h>
#include <click/migrationsender.hh>
#include <click/args.hh>
#include <click/straccum.hh>
#include <click/error.hh>
#include <click/timer.hh>
#include <click/router.hh>
#include <click/protocol.hh>

CLICK_DECLS
int MigrationSender :: connectToMachine()
{
	int ret;

	socket = new TCPSocket(TCPSocket :: ANY_PORT);

	/* connect to the remote machine */
	return socket->connect(port, hostname);
}

void MigrationSender :: run(Map *tcp_map, Map *udp_map, IPRewriterHeap **heap)
{
	ssize_t sent;
	/* send first packet containing size information */
	Protocol::MigrationHeader header;

	header.type = Protocol::MigrationHeader::T_INFO;
	header.migrationInfo.no_mappings_tcp = tcp_map->size();
	header.migrationInfo.no_mappings_udp = udp_map->size();
	header.migrationInfo.no_heap_best_effort = (*heap)->size();
	click_chatter("Best_effort %d\n", (*heap)->size());
	header.migrationInfo.no_heap_guranteed = (*heap)->size();

	print_info_header(header);

	sent = socket->send(&header, sizeof(Protocol::MigrationHeader::MigrationType) + sizeof(Protocol::MigrationInfo));
	if (sent < 0) {
		click_chatter("Error sending information header\n");
		goto err_out;
	}

err_out:
	socket->close();
}


CLICK_ENDDECLS
