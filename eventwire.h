enum rev {
	ev_end,
	ev_motion,
	ev_up,
	ev_down,
	ev_unicode,
	ev_quit,
};

#define EVWIRE_END() WIRE(ev_end)
#define EVWIRE_UP(k) WIRE(ev_up) U8(k)
#define EVWIRE_DOWN(k) WIRE(ev_down) U8(k)
#define EVWIRE_MOTION(x, y) WIRE(ev_motion) I16(x) I16(y)
#define EVWIRE_UNICODE(u) WIRE(ev_unicode) U8(u)
#define EVWIRE_QUIT() WIRE(ev_quit)

#define EVWIRE_SZ MTU
