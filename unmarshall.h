#define UNMARSHALL_BEGIN \
	while (1)        \
		switch (*p++) {
#define UNMARSHALL_END                           \
	default:                                 \
		printf("unhandled %d\n", p[-1]); \
		assert(0);                       \
		}
#undef U8
#define U8(x) uint8_t x = *p++;
#undef U16
#define U16(x)             \
	uint16_t x = *p++; \
	x <<= 8;           \
	x += *p++;
#undef I8
#define I8(x) int8_t x = *p++;
#undef I16
#define I16(x)            \
	int16_t x = *p++; \
	x <<= 8;          \
	x += *p++;
#undef WIRE
#define WIRE(x) case x: {
#undef BYTES
#define BYTES(s, n)     \
	uint8_t *s = p; \
	p += (n);
#undef DONE
#define DONE \
	}    \
	break
