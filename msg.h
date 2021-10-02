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
	size_t sofar = 0;
	while (sofar < s) {
		if (sendto(g_sock, p + sofar, szmin(BATCH, s - sofar), 0,
			(const struct sockaddr *)&g_sin, sizeof(g_sin)) < 0)
			perror("sendto");
		sofar += BATCH;
	}
}

static inline void
msgrecv(uint8_t *p, size_t s)
{
	size_t sofar = 0;
	while (sofar < s) {
		socklen_t sinlen;
		ssize_t	  rs =
		    recvfrom(g_sock, p + sofar, szmin(BATCH, s - sofar),
			MSG_WAITALL, (struct sockaddr *)&g_sin, &sinlen);
		if (rs < 0)
			perror("recvfrom");
		sofar += rs;
	}
}
