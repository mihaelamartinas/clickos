#include <click/config.h>
#include <click/etheraddress.hh>
#include <click/args.hh>
#include <click/confparse.hh>
#include <click/error.hh>
#include "cgnfixheader.hh"

using namespace CGN;

CLICK_DECLS

void CGNFixHeader::push(int, Packet *p)
{
	output(0).push(process(p));
}

Packet *CGNFixHeader::pull(int)
{
	Packet *p = input(0).pull();
	
	if (!p)
		return NULL;
	
	return process(p);
}

CLICK_ENDDECLS

EXPORT_ELEMENT(CGNFixHeader)

