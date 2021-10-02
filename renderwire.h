enum cmd {
	rw_end = 1,
	rw_config,
	rw_begin,
	rw_flush,
	rw_rect,
	rw_tex,
	rw_clear,
	rw_clip,
};

#define RWIRE_BEGIN() WIRE(rw_begin)
#define RWIRE_CONFIG(x, y, w, h) WIRE(rw_config) I16(x) I16(y) U16(w) U16(h)
#define RWIRE_ATLAS(a, w, h) WIRE(rw_tex) U16(w) U16(h) BYTES(a, (w) * (h))
#define RWIRE_CLEAR(r, g, b, a) WIRE(rw_clear) U8(r) U8(g) U8(b) U8(a)
#define RWIRE_CLIP(x, y, w, h) WIRE(rw_clip) I16(x) I16(y) U16(w) U16(h)
#define RWIRE_RECT(r, g, b, a, sx, sy, sw, sh, dx, dy, dw, dh) \
	WIRE(rw_rect)                                          \
	U8(r)                                                  \
	U8(g)                                                  \
	U8(b)                                                  \
	U8(a) I16(sx) I16(sy) U16(sw) U16(sh) I16(dx) I16(dy) U16(dw) U16(dh)
#define RWIRE_FLUSH() WIRE(rw_flush)
#define RWIRE_END() WIRE(rw_end)

#define RWIRE_SZ (40 * MTU)
