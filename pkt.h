#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

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
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
		perror("socket");
	memset(sin, 0, sizeof(*sin));
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = INADDR_ANY;
	sin->sin_port = htons(50042);
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
	g_sin.sin_addr.s_addr = *(in_addr_t *)he->h_addr_list[0];
}

static void
pkt_send(const uint8_t *p, size_t s)
{
#if 0 // defined __linux__
	ssize_t ss = sendto(
	    g_sock, p, s, 0, (const struct sockaddr *)&g_sin, sizeof(g_sin));
	if (ss < 0)
		perror("sendto");
	return;
#endif
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
#if 0 // defined __linux__
	ssize_t rs = recvfrom(
	    g_sock, p, s, 0, (struct sockaddr *)&g_sin, &sinlen);
	if (rs < 0)
		perror("recvfrom");
	//	printf("r %zu from %x\n", rs, g_sin.sin_addr.s_addr);
	return;
#endif
	size_t sofar = 0;
	while (sofar < s) {
		size_t	bs = szmin(BATCH, s - sofar);
		ssize_t rs = recvfrom(g_sock, p + sofar, bs, MSG_WAITALL,
		    (struct sockaddr *)&g_sin, &sinlen);
		if (rs < 0)
			perror("recvfrom");
		sofar += rs;
	}
}
