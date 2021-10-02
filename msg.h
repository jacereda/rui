
#define MTU 1500
#define BATCH (MTU * 6)
static inline size_t
szmin(size_t a, size_t b)
{
	return a < b ? a : b;
}

static inline void
msgsend(const uint8_t *p, size_t s)
{
#if defined __linux__
	sendto(g_sock, p, s, 0, (const struct sockaddr *)&g_sin, sizeof(g_sin));
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
		printf("s %zu/%zu\n", sofar, s);
	}
}

static inline void
msgrecv(uint8_t *p, size_t s)
{
	socklen_t sinlen;
#if defined __linux__
	recvfrom(g_sock, p, s, MSG_WAITALL, (struct sockaddr *)&g_sin, &sinlen);
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
		printf("r %zu/%zu\n", sofar, s);
	}
}
