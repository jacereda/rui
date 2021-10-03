#include "microui/src/microui.h"
#include "microui/demo/atlas.inl"

#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

enum keys {
#define K(x) K_##x,
#include "glcv/src/cvkeys.h"
};

#include "pkt.h"

#include "marshall.h"
#include "renderwire.h"
static uint8_t	g_buf[RWIRE_SZ];
static unsigned g_curr;

static int handle_events(mu_Context *ctx);

static void
push_quad(mu_Rect d, mu_Rect s, mu_Color c)
{
	RWIRE_RECT(c.r, c.g, c.b, c.a, s.x, s.y, s.w, s.h, d.x, d.y, d.w, d.h);
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
	RWIRE_END();
	DONE;
	assert(g_curr < sizeof(g_buf));
	pkt_send(g_buf, sizeof(g_buf));
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
	RWIRE_FLUSH();
	DONE;
	RWIRE_CLIP(r.x, r.y, umin(0xffff, r.w), umin(0xffff, r.h));
	DONE;
}

int
rui_process(mu_Context *ctx)
{
	mu_Command *cmd = NULL;
	RWIRE_BEGIN();
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
	RWIRE_FLUSH();
	DONE;
	send_packet();
	return handle_events(ctx);
}

void
rui_clear(mu_Color c)
{
	RWIRE_CLEAR(c.r, c.g, c.b, c.a);
	DONE;
}

void
rui_init(mu_Context *ctx)
{
	mu_init(ctx);
	ctx->text_width = get_text_width;
	ctx->text_height = get_text_height;

	pkt_init_client();

	RWIRE_CONFIG(100, 100, 512, 512);
	DONE;
	send_packet();
	handle_events(0);

	RWIRE_ATLAS(atlas_texture, ATLAS_WIDTH, ATLAS_HEIGHT);
	DONE;
	send_packet();
	handle_events(0);
}

#include "unmarshall.h"
#include "eventwire.h"

static int
handle_events(mu_Context *ctx)
{
	static int16_t mx, my;
	uint8_t	       buf[EVWIRE_SZ];
	pkt_recv(buf, sizeof(buf));

	UNMARSHALL_BEGIN(buf);

	EVWIRE_END();
	return 1;
	DONE;

	EVWIRE_MOTION(x, y);
	mu_input_mousemove(ctx, x, y);
	mx = x;
	my = y;
	DONE;

	EVWIRE_DOWN(k);
	switch (k) {
	case K_MOUSELEFT:
		mu_input_mousedown(ctx, mx, my, MU_MOUSE_LEFT);
		break;
	}
	DONE;

	EVWIRE_UP(k);
	switch (k) {
	case K_MOUSELEFT:
		mu_input_mouseup(ctx, mx, my, MU_MOUSE_LEFT);
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
