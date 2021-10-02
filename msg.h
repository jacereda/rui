static inline void
msgsend(const uint8_t *p, size_t s)
{
	size_t sofar = 0;
	while (sofar < s) {
		const size_t mtu = 1500;
		if (sendto(g_sock, p + sofar, mtu, 0,
			(const struct sockaddr *)&g_sin, sizeof(g_sin)) < 0)
			perror("sendto");
		sofar += mtu;
	}
}

static inline void
msgrecv(uint8_t *p, ssize_t s)
{
	size_t sofar = 0;
	while (sofar < s) {
		const size_t mtu = 1500;
		socklen_t    sinlen;
		ssize_t	     sz = recvfrom(g_sock, p + sofar, mtu, MSG_WAITALL,
			 (struct sockaddr *)&g_sin, &sinlen);
		if (sz < 0)
			perror("recvfrom");
		sofar += sz;
	}
}
