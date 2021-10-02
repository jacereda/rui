#undef U8
#define U8(x) *p++=(x);
#undef U16
#define U16(x) U8((x)>>8); U8(x);
#undef I8
#define I8 U8
#undef I16
#define I16 U16
#undef WIRE
#define WIRE(x) do {uint8_t*p0=g_buf+g_curr;uint8_t*p=p0;U8(x);
#undef BYTES
#define BYTES(s,n) for (unsigned i = 0; i <(n); i++) p[i]=(s)[i]; p+=(n);
#undef DONE
#define DONE g_curr += p-p0;} while(0)
  
