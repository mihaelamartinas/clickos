#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <clicknet/ether.h>
#include <click/etheraddress.hh>
#include "util/macint64.hh"
#include "util/staticbyteswap.hh"
#include "cgnsplitter.hh"

CLICK_DECLS

using namespace CGN;

int CGNSplitter::configure(Vector<String> &conf, ErrorHandler *errh)
{
	uint8_t insideMACArr[6];
	uint8_t outsideMACArr[6];
	int index;
	
	if (Args(conf, this, errh)
		.read_mp_with("INDEX", BoundedIntArg(1, 126), index)
		.complete() < 0)
		return -1;
	
	memset(insideMACArr, 0, 6);
	memset(outsideMACArr, 0, 6);
	
	insideMACArr[5]  = daisyChainAnnotation(index, true);
	outsideMACArr[5] = daisyChainAnnotation(index, false);
	
	insideMAC  = MacInt64::getSafe(insideMACArr);
	outsideMAC = MacInt64::getSafe(outsideMACArr);
	
	return 0;
}

void CGNSplitter::push(int, Packet *p)
{
	const click_ether *etherHeader = p->ether_header();
	uint64_t destMAC = MacInt64::getUnsafe(etherHeader->ether_dhost);
	
	if (etherHeader->ether_type != StaticHTONS<ETHERTYPE_IP>::value)
		output(PD_HOST).push(p);
	else if (destMAC == outsideMAC)
		output(PD_INBOUND).push(p);
	else if (destMAC == insideMAC)
		output(PD_OUTBOUND).push(p);
	else
		output(PD_HOST).push(p);
}

CLICK_ENDDECLS

EXPORT_ELEMENT(CGNSplitter)

