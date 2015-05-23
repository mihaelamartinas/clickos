/*
 * iprewriter.{cc,hh} -- rewrites packet source and destination
 * Max Poletto, Eddie Kohler
 *
 * Copyright (c) 2000 Massachusetts Institute of Technology
 * Copyright (c) 2008-2010 Meraki, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Click LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Click LICENSE file; the license in that file is
 * legally binding.
 */

#include <click/config.h>
#include "iprewriter.hh"
#include <clicknet/ip.h>
#include <clicknet/tcp.h>
#include <clicknet/udp.h>
#include <click/args.hh>
#include <click/straccum.hh>
#include <click/error.hh>
#include <click/timer.hh>
#include <click/router.hh>
CLICK_DECLS

IPRewriter::IPRewriter()
	:_udp_map(0), _udp_map_lock(new SimpleSpinlock)
{
#ifdef CLICK_USERLEVEL
	migration_thread_id = 0;
	thread_started = false;
#endif
}

IPRewriter::~IPRewriter()
{
#ifdef CLICK_USERLEVEL
	int ret;
	if (thread_started) {
		ret = pthread_join(migration_thread_id, NULL);
		if (ret != 0) {
			click_chatter("Error stopping migration thread!\n");
		} else
			thread_started = false;
	}
#endif
}

#ifdef CLICK_USERLEVEL
void *
IPRewriter::migration_run(void *migration_data)
{
	migration_data = NULL;
	click_chatter("Migration function thread has been called\n");
	return NULL;
}
#endif

void *
IPRewriter::cast(const char *n)
{
    if (strcmp(n, "IPRewriterBase") == 0)
	return (IPRewriterBase *)this;
    else if (strcmp(n, "TCPRewriter") == 0)
	return (TCPRewriter *)this;
    else if (strcmp(n, "IPRewriter") == 0)
	return this;
    else
	return 0;
}

int
IPRewriter::configure(Vector<String> &conf, ErrorHandler *errh)
{
	int ret;
    bool has_udp_streaming_timeout = false;
    _udp_timeouts[0] = 60 * 5;	// 5 minutes
    _udp_timeouts[1] = 5;	// 5 seconds

    if (Args(this, errh).bind(conf)
	.read("UDP_TIMEOUT", SecondsArg(), _udp_timeouts[0])
	.read("UDP_STREAMING_TIMEOUT", SecondsArg(), _udp_streaming_timeout).read_status(has_udp_streaming_timeout)
	.read("UDP_GUARANTEE", SecondsArg(), _udp_timeouts[1])
	.consume() < 0)
	return -1;

    if (!has_udp_streaming_timeout)
	_udp_streaming_timeout = _udp_timeouts[0];
    _udp_timeouts[0] *= CLICK_HZ; // change timeouts to jiffies
    _udp_timeouts[1] *= CLICK_HZ;
    _udp_streaming_timeout *= CLICK_HZ; // IPRewriterBase handles the others

    ret = TCPRewriter::configure(conf, errh);
	if (ret != 0)
		return ret;
#ifdef CLICK_USERLEVEL
	/* if the TCPRewriter has been configured, it is safe to start the
	 * migration thread */
	if (pthread_create(&migration_thread_id, NULL, migration_run, NULL))
		return errh->error("Failed to create migration thread\n");
	thread_started = true;
#endif
	return ret;
}

inline IPRewriterEntry *
IPRewriter::get_entry(int ip_p, const IPFlowID &flowid, int input)
{
    if (ip_p == IP_PROTO_TCP)
	return TCPRewriter::get_entry(ip_p, flowid, input);
    if (ip_p != IP_PROTO_UDP)
	return 0;

    _udp_map_lock->acquire();
    IPRewriterEntry *m = _udp_map.get(flowid);
    _udp_map_lock->release();

    if (!m && (unsigned) input < (unsigned) _input_specs.size()) {
	IPRewriterInput &is = _input_specs[input];
	IPFlowID rewritten_flowid = IPFlowID::uninitialized_t();
	if (is.rewrite_flowid(flowid, rewritten_flowid, 0, IPRewriterInput::mapid_iprewriter_udp) == rw_addmap)
	    m = IPRewriter::add_flow(0, flowid, rewritten_flowid, input);
    }
    return m;
}

IPRewriterEntry *
IPRewriter::add_flow(int ip_p, const IPFlowID &flowid,
		     const IPFlowID &rewritten_flowid, int input)
{
    if (ip_p == IP_PROTO_TCP)
	return TCPRewriter::add_flow(ip_p, flowid, rewritten_flowid, input);

    void *data;
    if (!(data = _udp_allocator.allocate()))
	return 0;

    IPRewriterEntry *entry;
    IPRewriterInput *rwinput = &_input_specs[input];
    IPRewriterFlow *flow = new(data) IPRewriterFlow
	(rwinput, flowid, rewritten_flowid, ip_p,
	 !!_udp_timeouts[1], click_jiffies() + relevant_timeout(_udp_timeouts));

    _udp_map_lock->acquire();
    entry = store_flow(flow, input, _udp_map, &reply_udp_map(rwinput));
    _udp_map_lock->release();

    return entry;
}

void
IPRewriter::push(int port, Packet *p_in)
{
    WritablePacket *p = p_in->uniqueify();
    click_ip *iph = p->ip_header();
	Map *map;
	SimpleSpinlock *lock;

    // handle non-first fragments
    if ((iph->ip_p != IP_PROTO_TCP && iph->ip_p != IP_PROTO_UDP)
	|| !IP_FIRSTFRAG(iph)
	|| p->transport_length() < 8) {
	const IPRewriterInput &is = _input_specs[port];
	if (is.kind == IPRewriterInput::i_nochange)
	    output(is.foutput).push(p);
	else
	    p->kill();
	return;
    }

    IPFlowID flowid(p);
	if (iph->ip_p == IP_PROTO_TCP) {
		lock = _map_lock;
		lock->acquire();
		map = &_map;
		lock->release();
	} else {
		lock = _udp_map_lock;
		lock->acquire();
		map = &_udp_map;
		lock->release();
	}
    lock->acquire();
    IPRewriterEntry *m = map->get(flowid);
    lock->release();

    if (!m) {			// create new mapping
	IPRewriterInput &is = _input_specs.unchecked_at(port);
	IPFlowID rewritten_flowid = IPFlowID::uninitialized_t();
	int result = is.rewrite_flowid(flowid, rewritten_flowid, p, iph->ip_p == IP_PROTO_TCP ? 0 : IPRewriterInput::mapid_iprewriter_udp);
	if (result == rw_addmap)
	    m = IPRewriter::add_flow(iph->ip_p, flowid, rewritten_flowid, port);
	if (!m) {
	    checked_output_push(result, p);
	    return;
	} else if (_annos & 2)
	    m->flow()->set_reply_anno(p->anno_u8(_annos >> 2));
    }

    click_jiffies_t now_j = click_jiffies();
    IPRewriterFlow *mf = m->flow();
    if (iph->ip_p == IP_PROTO_TCP) {
	TCPFlow *tcpmf = static_cast<TCPFlow *>(mf);
	tcpmf->apply(p, m->direction(), _annos);
	if (_timeouts[1])
	    tcpmf->change_expiry(_heap, true, now_j + _timeouts[1]);
	else
	    tcpmf->change_expiry(_heap, false, now_j + tcp_flow_timeout(tcpmf));
    } else {
	UDPFlow *udpmf = static_cast<UDPFlow *>(mf);
	udpmf->apply(p, m->direction(), _annos);
	if (_udp_timeouts[1])
	    udpmf->change_expiry(_heap, true, now_j + _udp_timeouts[1]);
	else
	    udpmf->change_expiry(_heap, false, now_j + udp_flow_timeout(udpmf));
    }

    output(m->output()).push(p);
}

String
IPRewriter::udp_mappings_handler(Element *e, void *)
{
    IPRewriter *rw = (IPRewriter *)e;
    click_jiffies_t now = click_jiffies();
    StringAccum sa;

    rw->_udp_map_lock->acquire();
    for (Map::iterator iter = rw->_udp_map.begin(); iter.live(); ++iter) {
	iter->flow()->unparse(sa, iter->direction(), now);
	sa << '\n';
    }
    rw->_udp_map_lock->release();

    return sa.take_string();
}

void
IPRewriter::add_handlers()
{
    add_read_handler("tcp_table", tcp_mappings_handler);
    add_read_handler("udp_table", udp_mappings_handler);
    add_read_handler("tcp_mappings", tcp_mappings_handler, 0, Handler::h_deprecated);
    add_read_handler("udp_mappings", udp_mappings_handler, 0, Handler::h_deprecated);
    set_handler("tcp_lookup", Handler::OP_READ | Handler::READ_PARAM, tcp_lookup_handler, 0);
    add_rewriter_handlers(true);
}

CLICK_ENDDECLS
ELEMENT_REQUIRES(TCPRewriter UDPRewriter)
EXPORT_ELEMENT(IPRewriter)
