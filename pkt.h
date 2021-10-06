#if defined _WIN32
#define _WINSOCKAPI_
#include <winsock2.h>
#include <Ws2tcpip.h>
typedef intptr_t ssize_t;
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

static int	   g_sock;
struct sockaddr_in g_sin;

#define MTU 1500
#define BATCH (MTU * 6)
static size_t
szmin(size_t a, size_t b)
{
	return a < b ? a : b;
}

static inline int
pkt_init_common(struct sockaddr_in *sin)
{
#if defined _WIN32
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
		perror("socket");
	memset(sin, 0, sizeof(*sin));
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = INADDR_ANY;
	sin->sin_port = htons(50042);

#if defined _WIN32
	unsigned long cmd = 0;
	return 0 == ioctlsocket(sock, FIONBIO, &cmd);
#endif
	return sock;
}

static inline void
pkt_init_server()
{
	struct sockaddr_in sin;
	g_sock = pkt_init_common(&sin);
	bind(g_sock, (const struct sockaddr *)&sin, sizeof(sin));
}

static inline void
pkt_init_client()
{
	const char *	hn = getenv("RUIHOST");
	struct hostent *he = gethostbyname(hn ? hn : "localhost");
	g_sock = pkt_init_common(&g_sin);
	memcpy(&g_sin.sin_addr.s_addr, he->h_addr, he->h_length);
	//	g_sin.sin_addr.s_addr = *(uint32_t *)he->h_addr_list[0];
}

static void
pkt_send(const uint8_t *p, size_t s)
{
	size_t sofar = 0;
	while (sofar < s) {
		size_t	bs = szmin(BATCH, s - sofar);
		ssize_t ss = sendto(g_sock, p + sofar, bs, 0,
		    (const struct sockaddr *)&g_sin, sizeof(g_sin));
		if (ss < 0)
			perror("sendto");
		sofar += ss;
	}
}

static void
pkt_recv(uint8_t *p, size_t s)
{
	socklen_t sinlen = sizeof(g_sin);
	size_t	  sofar = 0;
	while (sofar < s) {
		size_t	bs = szmin(BATCH, s - sofar);
		ssize_t rs = recvfrom(g_sock, p + sofar, bs, MSG_WAITALL,
		    (struct sockaddr *)&g_sin, &sinlen);
		if (rs < 0)
			perror("recvfrom");
		sofar += rs;
	}
}
