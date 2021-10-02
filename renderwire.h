enum cmd {
	end,
	begin,
	flush,
	rect,
	tex,
	clear,
	clip,
};

#define RWIRE_BEGIN() WIRE(begin)
#define RWIRE_ATLAS(a, w, h) WIRE(tex) U16(w) U16(h) BYTES(a, (w) * (h))
#define RWIRE_CLEAR(r, g, b, a) WIRE(clear) U8(r) U8(g) U8(b) U8(a)
#define RWIRE_CLIP(x, y, w, h) WIRE(clip) I16(x) I16(y) U16(w) U16(h)
#define RWIRE_RECT(r, g, b, a, sx, sy, sw, sh, dx, dy, dw, dh)              \
	WIRE(rect)                                                          \
	U8(r)                                                               \
	U8(g)                                                               \
	U8(b) U8(a) I16(sx) I16(sy) U16(sw) U16(sh) I16(dx) I16(dy) U16(dw) \
	    U16(dh)
#define RWIRE_FLUSH() WIRE(flush)
#define RWIRE_END() WIRE(end)

#define RWIRE_SZ 40 * MTU
