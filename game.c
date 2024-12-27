#include <math.h>
#include <stdio.h>
#include <GL/glut.h>

#include "textures/crate.h"
#include "textures/water.h"
#include "textures/grass.h"
#include "textures/grass_top.h"
#include "textures/cobblestone.h"

#define res 1
#define SW 260 * res
#define SH 150 * res
#define SW2 (SW / 2)
#define SH2 (SH / 2)
#define pixelScale 4/res
#define GLSW (SW * pixelScale)
#define GLSH (SH * pixelScale)
#define numSect 4
#define numWall 24

typedef struct {
	int fr1, fr2;
} time;
time T;

typedef struct {
	int w, a, s, d;
	int sl, sr;
	int m;
} keys;
keys K;

typedef struct {
	float cos[360];
	float sin[360];
} math;
math M;

typedef struct {
	int x, y, z;
	int a;
	int l;
} player;
player P;

typedef struct
{
	int x1, y1;
	int x2, y2;
	int wt, shade;
} walls;
walls W[30];

typedef struct
{
	int ws, we;
	int z1, z2;
	int d;
	int s;
	int surf[SW];
	int surface;
} sectors;

sectors S[30]; 

typedef struct
{
	int w, h;
	const unsigned char *name;
} TextureMaps;

TextureMaps Textures[32];

int dist(int x1, int y1, int x2, int y2) {
	return sqrt( (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) );
}


void drawPixel(int x, int y, int r, int g, int b) {
	glColor3ub(r, g, b);
	glBegin(GL_POINTS);
	glVertex2i(x*pixelScale+2, y*pixelScale+2);
	glEnd();
}

void movePlayer() {
	if (K.sl == 1 && K.m == 0) {
		P.a -= 4;
		if (P.a < 0)
			P.a += 360;
	}
	if (K.sr == 1 && K.m == 0) {
		P.a += 4;
		if (P.a > 359)
			P.a -= 360;
	}
	int dx = M.sin[P.a] * 10.0;
	int dy = M.cos[P.a] * 10.0;
	if (K.w == 1 && K.m == 0) {
		P.x += dx;
		P.y += dy;
	}
	if (K.s == 1 && K.m == 0) {
		P.x -= dx;
		P.y -= dy;
	}
	if (K.d == 1 && K.m == 0) {
		P.x += dy;
		P.y -= dx;
	}
	if (K.a == 1 && K.m == 0) {
		P.x -= dy;
		P.y += dx;
	}

	if(K.a == 1 && K.m == 1)
		P.l -= 1;
	if(K.d == 1 && K.m == 1)
		P.l += 1;
	if(K.w == 1 && K.m == 1)
		P.z += 4;
	if(K.s == 1 && K.m == 1)
		P.z -= 4;
}

void clearBackground() {
	int x, y;
	for (y = 0; y < SH; y++) {
		for (x = 0; x < SW; x++)
			drawPixel(x, y, 0, 60, 130);
	}
	
}

void clipBehindPlayer(int *x1, int *y1, int *z1, int x2, int y2, int z2) {
	float da=*y1;
	float db = y2;
	float d = da - db;
	if (d==0)
		d = 1;
	float s = da / (da-db);
	*x1 = *x1 + s*(x2-(*x1));
	*y1 = *y1 + s*(y2-(*y1));
	if (*y1 == 0)
		*y1 = 1;
	*z1 = *z1 + s*(z2-(*z1));
}

void drawWall (int x1, int x2, int b1, int b2, int t1, int t2, int s, int w, int loop) {
	int x, y;
	int wt = W[w].wt;
	float ht = 0, ht_step = (float) dist(W[w].x1, W[w].y1, W[w].x2, W[w].y2) / (float) (x2 - x1);
	int dyb = b2 - b1;
	int dyt = t2 - t1;
	int dx = x2 - x1;
	if (dx == 0)
		dx = 1;
	int xs = x1;
	if (x1 < 0) {
		ht -= ht_step * x1;
		x1 = 0;
	}
	if (x2 < 0)
		x2 = 0;
	if (x1 > SW)
		x1 = SW;
	if (x2 > SW)
		x2 = SW;

	for (x = x1; x < x2; x++) {
		int y1 = dyb * (x - xs + 0.5) / dx + b1;
		int y2 = dyt * (x - xs + 0.5) / dx + t1;
		float vt = 0, vt_step = (float) dist(W[w].x1, W[w].y1, W[w].x2, W[w].y2) / (float) (y2 - y1);
		if (y1 < 0) {
			vt -= vt_step * y1;
			y1 = 0;
		}
		if (y2 < 0)
			y2 = 0;
		if (y1 > SH)
			y1 = SH;
		if (y2 > SH)
			y2 = SH;
		
		if (loop == 0) {
			if (S[s].surface == 1)
				S[s].surf[x] = y1;
			if (S[s].surface == 2)
				S[s].surf[x] = y2;
			for(y = y1; y < y2; y++) {
				int pixel = (int) (Textures[wt].h - ((int) vt % Textures[wt].h) - 1)*3*Textures[wt].w + ((int) ht % Textures[wt].w) * 3;
				int r = Textures[wt].name[pixel+0] - W[w].shade/2;
				int g = Textures[wt].name[pixel+1] - W[w].shade/2;
				int b = Textures[wt].name[pixel+2] - W[w].shade/2;
				if (r < 0)
					r = 0;
				if (g < 0)
					g = 0;
				if (b < 0)
					b = 0;
				drawPixel(x, y, r, g, b);
				vt += vt_step;
			}
			ht += ht_step;
		}
		if (loop == 1) {
			int xo = SW / 2;
			int yo = SH / 2;
			int x2 = x - xo;
			int wo;

			if (S[s].surface == 1) {
				y2 = S[s].surf[x];
				wo = S[s].z1;
			}
			if (S[s].surface == 2) {
				y1 = S[s].surf[x];
				wo = S[s].z2;
			}
			if (1) {
				float lookUpDown = -P.l * 2 * M_PI; if(lookUpDown > SH) {lookUpDown = SH;}
				float moveUpDown = (float) (P.z - wo) / (float) yo; if(moveUpDown == 0) {moveUpDown = 0.001;}
				int ys = y1-yo, ye = y2-yo;
				float tile = 70;

				for (y = ys; y < ye; y++) {
					float z = y + lookUpDown; if (z == 0) {z = 0.0001;}
					float fx = x2 / z * moveUpDown * tile;
					float fy = 200.0 / z * moveUpDown * tile;
					float rx = fx * M.sin[P.a] - fy * M.cos[P.a] + (P.y / 60.0 * tile); if (rx < 0) {rx = -rx + 1;}
					float ry = fx * M.cos[P.a] + fy * M.sin[P.a] - (P.x / 60.0 * tile); if (ry < 0) {ry = -ry + 1;}

					int pixel = (int) (Textures[S[s].s].h - ((int) ry % Textures[S[s].s].h) - 1)*3*Textures[S[s].s].w + ((int) rx % Textures[S[s].s].w) * 3;
					int r = Textures[S[s].s].name[pixel+0];
					int g = Textures[S[s].s].name[pixel+1];
					int b = Textures[S[s].s].name[pixel+2];
					drawPixel(x2 + xo, y + yo, r, g, b);
				}
			}
		}
	}
}

void draw3D() {
	int x, s, w, loop, cycles, wx[4], wy[4], wz[4];
	float CS = M.cos[P.a], SN = M.sin[P.a];

	for (s = 0; s < numSect - 1; s++) {
		for (w = 0; w < numSect - s - 1; w++) {
			if (S[w].d < S[w+1].d) {
				sectors st = S[w];
				S[w] = S[w+1];
				S[w+1] = st;
			}
		}
	}
	
	for (s = 0; s < numSect; s++) {
		S[s].d = 0;
		if (P.z < S[s].z1) {
			S[s].surface = 1;
			cycles = 2;
			for (x = 0; x < SW; x++)
				S[s].surf[x] = SH;
		} else if (P.z > S[s].z2) {
			S[s].surface = 2;
			cycles = 2;
			for (x = 0; x < SW; x++)
				S[s].surf[x] = 0;
		} else {
			S[s].surface = 0;
			cycles = 1;
		}
		for (loop=0; loop < cycles; loop++) {
			S[s].d = 0;
			for(w=S[s].ws; w < S[s].we; w++) {

				int x1 = W[w].x1 - P.x, y1 = W[w].y1 - P.y;
				int x2 = W[w].x2 - P.x, y2 = W[w].y2 - P.y;
				if (loop == 1) {
					int swp = x1;
					x1 = x2;
					x2 = swp;
					swp = y1;
					y1 = y2;
					y2 = swp;
				}

				wx[0] = x1 * CS - y1 * SN;
				wx[1] = x2 * CS - y2 * SN;
				wx[2] = wx[0];
				wx[3] = wx[1];
				wy[0] = y1 * CS + x1 * SN;
				wy[1] = y2 * CS + x2 * SN;
				wy[2] = wy[0];
				wy[3] = wy[1];
				S[s].d += dist(0, 0, (wx[0]+wx[1])/2, (wy[0]+wy[1])/2);
				wz[0] = S[s].z1 - P.z + ((P.l * wy[0])/32.0);
				wz[1] = S[s].z1 - P.z + ((P.l * wy[1])/32.0);
				wz[2] = S[s].z2 - P.z + ((P.l * wy[0])/32.0);
				wz[3] = S[s].z2 - P.z + ((P.l * wy[1])/32.0);
				if (wy[0] < 1 && wy[1] < 1)
					continue;
				if (wy[0] < 1) {
					clipBehindPlayer(&wx[0], &wy[0], &wz[0], wx[1], wy[1], wz[1]);
					clipBehindPlayer(&wx[2], &wy[2], &wz[2], wx[3], wy[3], wz[3]);
				}
				if (wy[1] < 1) {
					clipBehindPlayer(&wx[1], &wy[1], &wz[1], wx[0], wy[0], wz[0]);
					clipBehindPlayer(&wx[3], &wy[3], &wz[3], wx[2], wy[2], wz[2]);
				}
				
				wx[0] = wx[0] * 200 / wy[0] + SW2;
				wy[0] = wz[0] * 200 / wy[0] + SH2;
				wx[1] = wx[1] * 200 / wy[1] + SW2;
				wy[1] = wz[1] * 200 / wy[1] + SH2;
				wx[2] = wx[2] * 200 / wy[2] + SW2;
				wy[2] = wz[2] * 200 / wy[2] + SH2;
				wx[3] = wx[3] * 200 / wy[3] + SW2;
				wy[3] = wz[3] * 200 / wy[3] + SH2;

				drawWall(wx[0], wx[1], wy[0], wy[1], wy[2], wy[3], s, w, loop);
			}
			S[s].d /= (S[s].we - S[s].ws);
		}
	}
}

void display() {
	if(T.fr1-T.fr2>=50) {
		clearBackground();
		movePlayer();
		draw3D();
		T.fr2 = T.fr1;
		glutSwapBuffers();
		glutReshapeWindow(GLSW, GLSH);
	}
	T.fr1 = glutGet(GLUT_ELAPSED_TIME);
	glutPostRedisplay();
}

void KeysDown(unsigned char key, int x, int y) {
	if (key == 'w')
		K.w = 1;
	if (key == 'a')
		K.a = 1;
	if (key == 's')
		K.s = 1;
	if (key == 'd')
		K.d = 1;
	if (key == 'm')
		K.m = 1;
	if (key == ',')
		K.sl = 1;
	if (key == '.')
		K.sr = 1;
}

void KeysUp(unsigned char key, int x, int y) {
	if (key == 'w')
		K.w = 0;
	if (key == 'a')
		K.a = 0;
	if (key == 's')
		K.s = 0;
	if (key == 'd')
		K.d = 0;
	if (key == 'm')
		K.m = 0;
	if (key == ',')
		K.sl = 0;
	if (key == '.')
		K.sr = 0;
}

int loadSectors[]= {
	0, 4, 0, 16, 0,
	4, 8, 0, 16, 1,
	8, 12,-20, -10, 3,
	12,24,0, 60, 4
};

int loadWalls[]= {
	100, 0, 116, 0, 0,
	116,0, 116,16, 0,
	116,16, 100,16, 0,
	100,16, 100, 0, 0,

	100, 100, 150, 100, 1,
	150,100, 250,250, 1,
	250,250, 100, 150, 1,
	100, 150, 100, 100, 1,

	-640, -640, 960, -640, 0,
	960,-640, 960,960, 1,
	960,960, -640,960, 2,
	-640,960, -640, -640, 1,

	-40, 0, -20, 0, 4,
	-20, 0, -20, -30, 4,
	-20,-30, -10, -40, 4,
	-10, -40, 10, -40, 4,
	10, -40, 20, -30, 4,
	20, -30, 20, 0, 4,
	20, 0, 40, 0, 4,
	40, 0, 40, 40, 4,
	40, 40, 20, 60, 4,
	20, 60, -20, 60, 4,
	-20, 60, -40, 40, 4,
	-40, 40, -40, 0, 4
};

void init () {
	int x;
	for (x = 0; x < 360; x++) {
		M.cos[x] = cos(x/180.0 * M_PI);
		M.sin[x] = sin(x/180.0 * M_PI);
	}
	
	P.x = 70;
	P.y = -110;
	P.z = 20;
	P.a = 0;
	P.l = 0;
	
	Textures[0].name = crate; Textures[0].h = CRATE_HEIGHT; Textures[0].w = CRATE_WIDTH;
	Textures[1].name = water; Textures[1].h = WATER_HEIGHT; Textures[1].w = WATER_WIDTH;
	Textures[2].name = grass; Textures[2].h = GRASS_HEIGHT; Textures[2].w = GRASS_WIDTH;
	Textures[3].name = grass_top; Textures[3].h = GRASS_TOP_HEIGHT; Textures[3].w = GRASS_TOP_WIDTH;
	Textures[4].name = cobblestone; Textures[4].h = COBBLESTONE_HEIGHT; Textures[4].w = COBBLESTONE_WIDTH;

	int s, w, v1=0, v2=0;
	for (s=0; s < numSect; s++) {
		S[s].ws = loadSectors[v1 + 0];
		S[s].we = loadSectors[v1 + 1];
		S[s].z1 = loadSectors[v1 + 2];
		S[s].z2 = loadSectors[v1 + 3] - loadSectors[v1 + 2];
		S[s].s = loadSectors[v1 + 4];
		v1 += 5;
		for (w=S[s].ws;w<S[s].we;w++) {
			W[w].x1 = loadWalls[v2+0];
			W[w].y1 = loadWalls[v2+1];
			W[w].x2 = loadWalls[v2+2];
			W[w].y2 = loadWalls[v2+3];
			W[w].wt = loadWalls[v2+4];
			float ang = atan2f(W[w].y2 - W[w].y1, W[w].x2 - W[w].x1);
			ang *= 180/M_PI;
			if (ang < 0)
				ang += 360;
			if (ang > 180)
				ang = 360 - ang;
			if (ang > 90)
				ang = 180 - ang;
			W[w].shade = ang;
			v2 += 5;
		}
	}
}

int main (int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowPosition(GLSW / 2, GLSH / 2);
	glutInitWindowSize(GLSW, GLSH);
	glutCreateWindow("");
	glPointSize(pixelScale);
	gluOrtho2D(0, GLSW, 0, GLSH);
	init();
	glutDisplayFunc(display);
	glutKeyboardFunc(KeysDown);
	glutKeyboardUpFunc(KeysUp);
	glutMainLoop();
	return 0;
}
