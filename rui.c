#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include "microui/src/microui.h"
#include "microui/demo/atlas.inl"

static uint8_t	g_buf[65507];
static unsigned g_curr;

enum keys {
#define K(x) K_##x,
#include "glcv/src/cvkeys.h"
};

static int	   g_sock;
struct sockaddr_in g_sin;

#include "marshall.h"
#include "renderwire.h"

static uint8_t *handle_events(mu_Context *ctx, uint8_t *p);

static void
push_quad(mu_Rect d, mu_Rect s, mu_Color c)
{
	WIRE_RECT(c.r, c.g, c.b, c.a, s.x, s.y, s.w, s.h, d.x, d.y, d.w, d.h);
	DONE;
}

static void
draw_rect(mu_Rect r, mu_Color c)
{
	push_quad(r, atlas[ATLAS_WHITE], c);
}

static void
draw_text(const char *text, mu_Vec2 pos, mu_Color c)
{
	mu_Rect d = { pos.x, pos.y, 0, 0 };
	for (const char *p = text; *p; p++) {
		if ((*p & 0xc0) == 0x80) {
			continue;
		}
		int	chr = mu_min((unsigned char)*p, 127);
		mu_Rect s = atlas[ATLAS_FONT + chr];
		d.w = s.w;
		d.h = s.h;
		push_quad(d, s, c);
		d.x += d.w;
	}
}

static void
draw_icon(int id, mu_Rect rect, mu_Color color)
{
	mu_Rect s = atlas[id];
	int	x = rect.x + (rect.w - s.w) / 2;
	int	y = rect.y + (rect.h - s.h) / 2;
	push_quad(mu_rect(x, y, s.w, s.h), s, color);
}

static int
get_text_width(mu_Font font, const char *text, int len)
{
	int res = 0;
	(void)font;
	if (len == -1) {
		len = strlen(text);
	}
	for (const char *p = text; *p && len--; p++) {
		if ((*p & 0xc0) == 0x80) {
			continue;
		}
		int chr = mu_min((unsigned char)*p, 127);
		res += atlas[ATLAS_FONT + chr].w;
	}
	return res;
}

static int
get_text_height(mu_Font font)
{
	(void)font;
	return 18;
}

static void
send_packet()
{
	WIRE_END();
	DONE;
	assert(g_curr < sizeof(g_buf));
	if (sendto(g_sock, g_buf, g_curr, 0, (const struct sockaddr *)&g_sin,
		sizeof(g_sin)) < 0)
		perror("sendto");
	g_curr = 0;
}

static unsigned
umin(unsigned a, unsigned b)
{
	return a < b ? a : b;
}

static void
set_clip_rect(mu_Rect r)
{
	WIRE_FLUSH();
	DONE;
	WIRE_CLIP(r.x, r.y, umin(0xffff, r.w), umin(0xffff, r.h));
	DONE;
}

int
rui_process(mu_Context *ctx)
{
	mu_Command *cmd = NULL;
	WIRE_BEGIN();
	DONE;
	while (mu_next_command(ctx, &cmd)) {
		switch (cmd->type) {
		case MU_COMMAND_TEXT:
			draw_text(
			    cmd->text.str, cmd->text.pos, cmd->text.color);
			break;
		case MU_COMMAND_RECT:
			draw_rect(cmd->rect.rect, cmd->rect.color);
			break;
		case MU_COMMAND_ICON:
			draw_icon(
			    cmd->icon.id, cmd->icon.rect, cmd->icon.color);
			break;
		case MU_COMMAND_CLIP:
			set_clip_rect(cmd->clip.rect);
			break;
		}
	}
	WIRE_FLUSH();
	DONE;
	send_packet();

	socklen_t sinlen;
	ssize_t	  sz = recvfrom(g_sock, g_buf, sizeof(g_buf), MSG_WAITALL,
	      (struct sockaddr *)&g_sin, &sinlen);
	if (sz < 0)
		perror("recvfrom");
	uint8_t *pend = handle_events(ctx, g_buf);
	assert(!pend || pend == g_buf + sz);
	return pend != 0;
}

void
rui_clear(mu_Color c)
{
	WIRE_CLEAR(c.r, c.g, c.b, c.a);
	DONE;
}

void
rui_init(mu_Context *ctx)
{
	const char *	hn = getenv("RUIHOST");
	struct hostent *he = gethostbyname(hn ? hn : "localhost");

	if ((g_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		perror("sock");

	memset(&g_sin, 0, sizeof(g_sin));
	g_sin.sin_family = AF_INET;
	g_sin.sin_addr.s_addr = *(in_addr_t *)he->h_addr_list[0];
	g_sin.sin_port = htons(50042);

	mu_init(ctx);
	ctx->text_width = get_text_width;
	ctx->text_height = get_text_height;

	WIRE_ATLAS(atlas_texture, ATLAS_WIDTH, ATLAS_HEIGHT);
	DONE;
	send_packet();
}

#include "unmarshall.h"
#include "eventwire.h"

static uint8_t *
handle_events(mu_Context *ctx, uint8_t *p)
{
	static int16_t x, y;

	UNMARSHALL_BEGIN;

	EVWIRE_END();
	return p;
	DONE;

	EVWIRE_MOTION(x, y);
	mu_input_mousemove(ctx, x, y);
	DONE;

	EVWIRE_DOWN(k);
	switch (k) {
	case K_MOUSELEFT:
		mu_input_mousedown(ctx, x, y, MU_MOUSE_LEFT);
		break;
	}
	DONE;

	EVWIRE_UP(k);
	switch (k) {
	case K_MOUSELEFT:
		mu_input_mouseup(ctx, x, y, MU_MOUSE_LEFT);
		break;
	}
	DONE;

	EVWIRE_UNICODE(u);
	mu_input_text(ctx, (const char *)&u);
	DONE;

	EVWIRE_QUIT();
	return 0;
	DONE;
	UNMARSHALL_END;
}
