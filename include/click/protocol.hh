#ifndef CLICKOS_MIGRATION_PROTOCOL
#define CLICKOS_MIGRATION_PROTOCOL

#include <click/config.h>

CLICK_DECLS
namespace Protocol {
	struct Instantiate
	{
		uint32_t ip;
		uint32_t subnetCount;
	};

	struct Destroy
	{
		uint32_t ip;
	};

	struct SetPrefixLength
	{
		uint8_t length;
	};

	struct Migrate
	{
		uint32_t ip;
		uint8_t index;
		uint8_t destinationLength;
		uint16_t destinationPort; /* in host byte order */
	};

	struct AcceptMigration
	{
		uint32_t ip;
		uint32_t subnetCount;
	};

	struct Header
	{
		enum Type
		{
			T_MIGRATE,
			T_ACCEPT_MIGRATION,

			T_ACK,
			T_NACK,
		};

		Type type;

		union
		{
			Instantiate instantiate;
			Destroy destroy;
			SetPrefixLength setPrefixLength;
			Migrate migrate;
			AcceptMigration acceptMigration;
		};

		uint32_t subnets[0];
		char destination[0];
	};
}
CLICK_ENDDECLS
#endif /* CGN_CONTROLLER_PROTOCOL */
