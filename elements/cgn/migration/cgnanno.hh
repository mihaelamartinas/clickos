#ifndef CLICK_CGN_ANNO_HH
#define CLICK_CGN_ANNO_HH

#include <click/config.h>
#include <click/packet_anno.hh>
#include <click/glue.hh>
#include <click/packet.hh>
#include <click/args.hh>

CLICK_DECLS

/* easy peasy to change annotation type this way */

namespace CGN
{

typedef uint8_t Annotation;

const Annotation ANNOTATION_NONE = 0;

const Annotation ANNOTATION_MIN  = 0;
const Annotation ANNOTATION_MAX  = 0xff;

#define CLICK_CGN_ANNOTATION_PARSER BoundedIntArg(CGN::ANNOTATION_NONE, CGN::ANNOTATION_MAX)

static inline void setAnnotation(Packet *p, Annotation anno)
{
	SET_PAINT_ANNO(p, anno);
}

static inline Annotation getAnnotation(Packet *p)
{
	return PAINT_ANNO(p);
}

static inline Annotation daisyChainAnnotation(uint8_t computer, bool outbound)
{
	return computer + (outbound << 7);
}

static inline Annotation forwardAnnotation(bool outbound)
{
	return (0xff >> 1) + (outbound << 7);
}

static inline bool annotationIsOutbound(Annotation anno)
{
	return (bool)(anno >> 7);
}

}

CLICK_ENDDECLS

#endif /* CLICK_CGN_ANNO_HH */
