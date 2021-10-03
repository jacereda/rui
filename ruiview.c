#include "glcv/src/cv.h"
#include "glcv/src/cvgl.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static GLuint	texbo;
static GLuint	vertbo;
static GLuint	colbo;
static GLuint	indbo;
static GLuint	trans;
static GLuint	g_sca;
static unsigned g_curr;
static int	g_x;
static int	g_y;
static int	g_w;
static int	g_h;

#include "pkt.h"

static void config();
static void send_events();

static void
report(const char *name, const char *s)
{
	fprintf(stderr, "%s:%s", name, s);
}

static void
init()
{
	pkt_init_server();
	config();
	send_events();
}

static void
glinit()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);

	GLuint t;
	glGenTextures(1, &t);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, t);
	glTexImage2D(
	    GL_TEXTURE_2D, 0, GL_RED, 128, 128, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	GLint st;

	const GLchar *const vss =
	    "#version 100\n"
	    "uniform mat4 trans;\n"
	    "uniform vec2 sca;\n"
	    "attribute vec2 axy;\n"
	    "attribute vec2 auv;\n"
	    "attribute vec4 argba;\n"
	    "varying vec2 vuv;\n"
	    "varying vec4 vrgba;\n"
	    "void main()\n"
	    "{\n"
	    "    gl_Position = trans * vec4(axy.x, axy.y, 0.0, 1.0);\n"
	    "    vuv = auv*sca;\n"
	    "    vrgba = argba;\n"
	    "}\n";
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vss, 0);
	glCompileShader(vs);
	glGetShaderiv(vs, GL_COMPILE_STATUS, &st);
	if (!st) {
		char err[1024];
		glGetShaderInfoLog(vs, sizeof(err), 0, err);
		printf("%s\n", err);
	}

	const GLchar *const fss =
	    "#version 100\n"
	    "precision mediump float;\n"
	    "uniform sampler2D stex;\n"
	    "varying vec2 vuv;\n"
	    "varying vec4 vrgba;\n"
	    "void main()\n"
	    "{\n"
	    "    gl_FragColor = vrgba * texture2D(stex,vuv).r;\n"
	    "}\n";
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fss, 0);
	glCompileShader(fs);
	glGetShaderiv(fs, GL_COMPILE_STATUS, &st);
	if (!st) {
		char err[1024];
		glGetShaderInfoLog(fs, sizeof(err), 0, err);
		printf("%s\n", err);
	}
	GLuint sh = glCreateProgram();
	glAttachShader(sh, vs);
	glAttachShader(sh, fs);
	glLinkProgram(sh);

	glGetProgramiv(sh, GL_LINK_STATUS, &st);
	if (!st) {
		char err[1024];
		glGetProgramInfoLog(sh, sizeof(err), 0, err);
		printf("%s\n", err);
	}
	glUseProgram(sh);

	GLuint stex = glGetUniformLocation(sh, "stex");
	glUniform1i(stex, 0);

	g_sca = glGetUniformLocation(sh, "sca");

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLint xy = glGetAttribLocation(sh, "axy");
	assert(xy >= 0);
	GLint uv = glGetAttribLocation(sh, "auv");
	assert(uv >= 0);
	GLint rgba = glGetAttribLocation(sh, "argba");
	assert(rgba >= 0);

	glGenBuffers(1, &vertbo);
	glBindBuffer(GL_ARRAY_BUFFER, vertbo);
	glVertexAttribPointer(xy, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(xy);

	glGenBuffers(1, &texbo);
	glBindBuffer(GL_ARRAY_BUFFER, texbo);
	glVertexAttribPointer(uv, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(uv);

	glGenBuffers(1, &colbo);
	glBindBuffer(GL_ARRAY_BUFFER, colbo);
	glVertexAttribPointer(rgba, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
	glEnableVertexAttribArray(rgba);

	glGenBuffers(1, &indbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indbo);

	trans = glGetUniformLocation(sh, "trans");
	assert(glGetError() == 0);

	/* cvReport("%d %d", cvWidth(), cvHeight()); */
	/* glViewport(0, 0, cvWidth(), cvHeight()); */
	/* glClear(GL_COLOR_BUFFER_BIT); */
}

#include "marshall.h"
#include "eventwire.h"
static uint8_t g_buf[EVWIRE_SZ];

static void
up(cvkey k)
{
	EVWIRE_UP(k);
	DONE;
}

static void
down(cvkey k)
{
	EVWIRE_DOWN(k);
	DONE;
}

static void
unicode(uint32_t c)
{
	EVWIRE_UNICODE(c);
	DONE;
}

static void
motion(int x, int y)
{
	EVWIRE_MOTION(x, y);
	DONE;
}

static void
close()
{
	cvQuit();
}

static void
quit()
{
	EVWIRE_QUIT();
	DONE;
	// rev_quit();
}

static void
evend()
{
	EVWIRE_END();
	DONE;
}

#include "unmarshall.h"
#include "renderwire.h"

static void
config()
{
	uint8_t buf[RWIRE_SZ];
	pkt_recv(buf, sizeof(buf));

	UNMARSHALL_BEGIN(buf);

	RWIRE_CONFIG(x, y, w, h);
	g_x = x;
	g_y = y;
	g_w = w;
	g_h = h;
	DONE;

	RWIRE_END();
	return;
	DONE;

	UNMARSHALL_END;
}

static void
update()
{
	uint8_t buf[RWIRE_SZ];
	pkt_recv(buf, sizeof(buf));
#define BUFFER_SIZE 16384
	GLfloat	 uv[BUFFER_SIZE * 8];
	GLfloat	 xy[BUFFER_SIZE * 8];
	GLubyte	 rgba[BUFFER_SIZE * 16];
	GLuint	 ind[BUFFER_SIZE * 6];
	unsigned bi = 0;
	UNMARSHALL_BEGIN(buf);

	RWIRE_BEGIN();
	glViewport(0, 0, cvWidth(), cvHeight());
	glScissor(0, 0, 0xffff, 0xffff);
	DONE;

	RWIRE_FLUSH();
	// clang-format off
	GLfloat t[] = {
	  2. / cvWidth(), 0, 0, 0,
	  0, -2. / cvHeight(), 0, 0,
	  0, 0, -1, 0,
	  -1, 1, 1, 1,
	};
	// clang-format on
	glUniformMatrix4fv(trans, 1, GL_FALSE, t);
	glBindBuffer(GL_ARRAY_BUFFER, vertbo);
	glBufferData(
	    GL_ARRAY_BUFFER, bi * 8 * sizeof(xy[0]), xy, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, texbo);
	glBufferData(
	    GL_ARRAY_BUFFER, bi * 8 * sizeof(uv[0]), uv, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, colbo);
	glBufferData(
	    GL_ARRAY_BUFFER, bi * 16 * sizeof(rgba[0]), rgba, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, bi * 6 * sizeof(ind[0]), ind,
	    GL_DYNAMIC_DRAW);
	glDrawElements(GL_TRIANGLES, bi * 6, GL_UNSIGNED_INT, 0);
	bi = 0;
	assert(glGetError() == 0);
	DONE;

	RWIRE_ATLAS(bytes, sw, sh);
	glUniform2f(g_sca, 1.f / sw, 1.f / sh);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, sw, sh, 0, GL_RED,
	    GL_UNSIGNED_BYTE, bytes);
	DONE;

	RWIRE_RECT(r, g, b, a, sx, sy, sw, sh, dx, dy, dw, dh);
	unsigned vi = bi * 8;
	unsigned ci = bi * 16;
	unsigned ei = bi * 4;
	unsigned ii = bi * 6;
	bi++;
	ind[ii + 0] = ei + 0;
	ind[ii + 1] = ei + 1;
	ind[ii + 2] = ei + 2;
	ind[ii + 3] = ei + 2;
	ind[ii + 4] = ei + 3;
	ind[ii + 5] = ei + 1;
	uv[vi + 0] = sx;
	uv[vi + 1] = sy;
	uv[vi + 2] = sx + sw;
	uv[vi + 3] = sy;
	uv[vi + 4] = sx;
	uv[vi + 5] = sy + sh;
	uv[vi + 6] = sx + sw;
	uv[vi + 7] = sy + sh;
	xy[vi + 0] = dx;
	xy[vi + 1] = dy;
	xy[vi + 2] = dx + dw;
	xy[vi + 3] = dy;
	xy[vi + 4] = dx;
	xy[vi + 5] = dy + dh;
	xy[vi + 6] = dx + dw;
	xy[vi + 7] = dy + dh;
	for (unsigned i = 0; i < 4; i++) {
		rgba[ci + 4 * i + 0] = r;
		rgba[ci + 4 * i + 1] = g;
		rgba[ci + 4 * i + 2] = b;
		rgba[ci + 4 * i + 3] = a;
	}
	DONE;

	RWIRE_CLEAR(r, g, b, a);
	glClearColor(r / 255., g / 255., b / 255., a / 255.);
	glClear(GL_COLOR_BUFFER_BIT);
	DONE;

	RWIRE_CLIP(sx, sy, sw, sh);
	glScissor(sx, cvHeight() - (sy + sh), sw, sh);
	DONE;

	RWIRE_END();
	return;
	DONE;

	UNMARSHALL_END;
}

static void
send_events()
{
	evend();
	pkt_send(g_buf, sizeof(g_buf));
	g_curr = 0;
}

intptr_t
event(const ev *e)
{
	intptr_t    ret = 1;
	cveventtype t = evType(e);
	//	cvReport("%s %zu %zu", evName(e), evArg0(e), evArg1(e));
	switch (t) {
	case CVQ_LOGGER:
		ret = (intptr_t)report;
		break;
	case CVQ_NAME:
		ret = (intptr_t) "ruiview";
		break;
	case CVQ_XPOS:
		ret = g_x;
		break;
	case CVQ_YPOS:
		ret = g_y;
		break;
	case CVQ_WIDTH:
		ret = g_w;
		break;
	case CVQ_HEIGHT:
		ret = g_h;
		break;
	case CVE_QUIT:
		quit();
		send_events();
		ret = 0;
		break;
	case CVE_INIT:
		init();
		ret = 0;
		break;
	case CVE_GLINIT:
		glinit();
		ret = 0;
		break;
	case CVE_DOWN:
		down(evWhich(e));
		ret = 0;
		break;
	case CVE_UP:
		up(evWhich(e));
		ret = 0;
		break;
	case CVE_UNICODE:
		unicode(evUnicode(e));
		ret = 0;
		break;
	case CVE_MOTION:
		motion(evX(e), evY(e));
		ret = 0;
		break;
	case CVE_CLOSE:
		close();
		ret = 0;
		break;
	case CVE_UPDATE:
		update();
		send_events();
		ret = 0;
		break;
	default:
		ret = 0;
		break;
	}
	return ret;
}
