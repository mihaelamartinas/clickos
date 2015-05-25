#ifndef CGN_CONTROLLER_PROTOCOL
#define CGN_CONTROLLER_PROTOCOL

#include <stddef.h>
#include <string.h>

namespace CGNController
{

/* everything is in host-byte order (except for IPs) */
namespace Protocol
{

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
		T_INSTANTIATE,
		T_DESTROY,
		
		T_SET_PREFIX_LENGTH,
		
		T_MIGRATE,
		T_ACCEPT_MIGRATION,
		
		T_PRECOPY,
		T_ACCEPT_PRECOPY,
		
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

}

#endif /* CGN_CONTROLLER_PROTOCOL */
