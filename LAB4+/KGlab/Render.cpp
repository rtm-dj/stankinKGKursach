#include "Render.h"
#include <Windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "GUItextRectangle.h"
#include "MyShaders.h"
#include "Texture.h"
#include <vector>
#include <chrono>
#include <random>
#include <map>

#include "ObjLoader.h"


#include "debout.h"

//---------------------Models----------------------
float wheelRotationAngle = 0;

double easeBounceOut(double t) {
	if (t < 1 / 2.75) {
		return 7.5625 * t * t;
	}
	else if (t < 2 / 2.75) {
		t -= 1.5 / 2.75;
		return 7.5625 * t * t + 0.75;
	}
	else if (t < 2.5 / 2.75) {
		t -= 2.25 / 2.75;
		return 7.5625 * t * t + 0.9375;
	}
	else {
		t -= 2.625 / 2.75;
		return 7.5625 * t * t + 0.984375;
	}
}

class roadSign {
public:
	int type;
	Texture signTex;
	unsigned int textureId = 0;
	double spawnTime = 0;
	bool spawnFinished = false;
	float x, y, z;

	roadSign(int _type, int _x, int _y, int _z) : type(_type), x(_x), y(_y), z(_z) {}

	void spawn() {
		double duration = 0.5;
		double t = spawnTime / duration;

		if (t >= 1) {
			t = 1;
			spawnFinished = true;
		}

		double offset = easeBounceOut(t);
		double currentZ = -10 + offset * 10;

		glPushMatrix();
		glTranslated(0, 0, currentZ);
		drawSign();
		glPopMatrix();

		if (!spawnFinished)
			spawnTime += 0.016;
	}

	void resetSpawnAnimation() {
		spawnTime = 0;
		spawnFinished = false;
	}

	void drawSign() {
		glTranslated(x, y, z);

		glBegin(GL_QUADS);
		glColor3d(0.5, 0.5, 0.5);
		glVertex3f(-0.2, -0.2, 0);
		glVertex3f(0.2, -0.2, 0);
		glVertex3f(0.2, -0.2, 5);
		glVertex3f(-0.2, -0.2, 5);

		glVertex3f(-0.2, 0.2, 0);
		glVertex3f(0.2, 0.2, 0);
		glVertex3f(0.2, 0.2, 5);
		glVertex3f(-0.2, 0.2, 5);

		glColor3d(0.4, 0.4, 0.4);
		glVertex3f(-0.2, -0.2, 0);
		glVertex3f(-0.2, 0.2, 0);
		glVertex3f(-0.2, 0.2, 5);
		glVertex3f(-0.2, -0.2, 5);

		glVertex3f(0.2, -0.2, 0);
		glVertex3f(0.2, 0.2, 0);
		glVertex3f(0.2, 0.2, 5);
		glVertex3f(0.2, -0.2, 5);
		glEnd();

		if (y < 0) {
			glRotated(180, 0, 0, 1);
		}

		glEnable(GL_TEXTURE_2D);

		switch (type) {
		case 0:
			signTex.LoadTexture("textures/signArrow.jpg"); textureId = signTex.GetID(); break;
		case 1:
			signTex.LoadTexture("textures/signPedestrians.jpg"); textureId = signTex.GetID(); break;
		case 2:
			signTex.LoadTexture("textures/signSpeed.jpg"); textureId = signTex.GetID(); break;
		}
		glBindTexture(GL_TEXTURE_2D, textureId);

		glPushMatrix();
		glRotated(90, 0, 0, 1);

		glBegin(GL_QUADS);
		glColor3d(1, 1, 1);
		glTexCoord2f(0, 0); glVertex3f(-1, -0.1, 5);
		glTexCoord2f(1, 0); glVertex3f(1, -0.1, 5);
		glTexCoord2f(1, 1); glVertex3f(1, -0.1, 7);
		glTexCoord2f(0, 1); glVertex3f(-1, -0.1, 7);
		glEnd();
		glDisable(GL_TEXTURE_2D);

		glBegin(GL_QUADS);
		glColor3d(0.3, 0.3, 0.3);
		glVertex3f(-1, 0.1, 5);
		glVertex3f(1, 0.1, 5);
		glVertex3f(1, 0.1, 7);
		glVertex3f(-1, 0.1, 7);

		glVertex3f(-1, -0.1, 5);
		glVertex3f(-1, 0.1, 5);
		glVertex3f(-1, 0.1, 7);
		glVertex3f(-1, -0.1, 7);

		glVertex3f(1, -0.1, 5);
		glVertex3f(1, 0.1, 5);
		glVertex3f(1, 0.1, 7);
		glVertex3f(1, -0.1, 7);

		glVertex3f(-1, -0.1, 7);
		glVertex3f(1, -0.1, 7);
		glVertex3f(1, 0.1, 7);
		glVertex3f(-1, 0.1, 7);

		glVertex3f(-1, -0.1, 5);
		glVertex3f(1, -0.1, 5);
		glVertex3f(1, 0.1, 5);
		glVertex3f(-1, 0.1, 5);
		glEnd();

		glPopMatrix();
	}
};

void drawWheel(float x, float y, float z, bool isRotating = true) {
	const float wheelRadius = 0.7;
	const float wheelWidth = 0.4;
	const int segments = 10;

	if (isRotating) {
		wheelRotationAngle += 2;
		if (wheelRotationAngle > 360) {
			wheelRotationAngle -= 360;
		}
	}

	glPushMatrix();
	glTranslatef(x, y, z);
	glRotatef(-270, 1, 0, 0);
	glRotatef(wheelRotationAngle, 0, 0, 1);

	glColor3f(0.1, 0.1, 0.1);
	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= segments; ++i) {
		float angle = 2 * 3.14 * i / segments;
		float cosA = cos(angle);
		float sinA = sin(angle);
		glVertex3f(cosA * wheelRadius, sinA * wheelRadius, wheelWidth / 2);
		glVertex3f(cosA * wheelRadius, sinA * wheelRadius, -wheelWidth / 2);
	}
	glEnd();

	glColor3f(0.5, 0.5, 0.5);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0, 0, wheelWidth / 2);
	for (int i = 0; i <= segments; ++i) {
		float angle = 2 * 3.14 * i / segments;
		glVertex3f(cos(angle) * wheelRadius, sin(angle) * wheelRadius, wheelWidth / 2);
	}
	glEnd();

	glColor3f(0.3, 0.3, 0.3);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0, 0, -wheelWidth / 2);
	for (int i = segments; i >= 0; --i) {
		float angle = 2 * 3.14 * i / segments;
		glVertex3f(cos(angle) * wheelRadius, sin(angle) * wheelRadius, -wheelWidth / 2);
	}
	glEnd();

	glColor3f(0.2, 0.2, 0.2);
	glBegin(GL_LINES);
	for (int i = 0; i < segments; ++i) {
		float angle = 2 * 3.14 * i / segments;
		glVertex3f(0, 0, 0);
		glVertex3f(cos(angle) * wheelRadius * 0.7, sin(angle) * wheelRadius * 0.7, 0);
	}
	glEnd();

	glPopMatrix();
}

void drawLights(float x, float y, float z, float width, float height, float depth, int type) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (type == 0)
		glColor4f(1, 1, 0, 0.5);
	else
		glColor4f(1, 0, 0, 0.5);

	float x1 = x;
	float y1 = y;
	float z1 = z;
	float x2 = x + width;
	float y2 = y + height;
	float z2 = z + depth;

	glBegin(GL_QUADS);
	glVertex3f(x1, y1, z1);
	glVertex3f(x2, y1, z1);
	glVertex3f(x2, y2, z1);
	glVertex3f(x1, y2, z1);

	glVertex3f(x1, y1, z2);
	glVertex3f(x2, y1, z2);
	glVertex3f(x2, y2, z2);
	glVertex3f(x1, y2, z2);

	glVertex3f(x1, y1, z1);
	glVertex3f(x1, y1, z2);
	glVertex3f(x1, y2, z2);
	glVertex3f(x1, y2, z1);

	glVertex3f(x2, y1, z1);
	glVertex3f(x2, y1, z2);
	glVertex3f(x2, y2, z2);
	glVertex3f(x2, y2, z1);

	glVertex3f(x1, y2, z1);
	glVertex3f(x2, y2, z1);
	glVertex3f(x2, y2, z2);
	glVertex3f(x1, y2, z2);

	glVertex3f(x1, y1, z1);
	glVertex3f(x2, y1, z1);
	glVertex3f(x2, y1, z2);
	glVertex3f(x1, y1, z2);
	glEnd();

	glDisable(GL_BLEND);
}

void drawVihlop(float x, float y, float z) {
	const float wheelRadius = 0.2;
	const float wheelWidth = 1;
	const int segments = 6;

	glPushMatrix();
	glTranslatef(x, y, z);
	glRotatef(90, 0, 1, 0);
	glColor3f(0.1, 0.1, 0.1);
	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= segments; ++i) {
		float angle = 2 * 3.14 * i / segments;
		float cosA = cos(angle);
		float sinA = sin(angle);
		glVertex3f(cosA * wheelRadius, sinA * wheelRadius, wheelWidth / 2);
		glVertex3f(cosA * wheelRadius, sinA * wheelRadius, -wheelWidth / 2);
	}
	glEnd();

	glColor3f(0.5, 0.5, 0.5);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0, 0, wheelWidth / 2);
	for (int i = 0; i <= segments; ++i) {
		float angle = 2 * 3.14 * i / segments;
		glVertex3f(cos(angle) * wheelRadius, sin(angle) * wheelRadius, wheelWidth / 2);
	}
	glEnd();

	glColor3f(0.3, 0.3, 0.3);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0, 0, -wheelWidth / 2);
	for (int i = segments; i >= 0; --i) {
		float angle = 2 * 3.14 * i / segments;
		glVertex3f(cos(angle) * wheelRadius, sin(angle) * wheelRadius, -wheelWidth / 2);
	}
	glEnd();

	glColor3f(0.2, 0.2, 0.2);
	glBegin(GL_LINES);
	for (int i = 0; i < segments; ++i) {
		float angle = 2 * 3.14 * i / segments;
		glVertex3f(0, 0, 0);
		glVertex3f(cos(angle) * wheelRadius * 0.7, sin(angle) * wheelRadius * 0.7, 0);
	}
	glEnd();
	glPopMatrix();
}


class Car {
public:
	int id;	
	float colorRed;
	float colorGreen;
	float colorBlue;
	float positionX = 0;
	bool isSuv;

	void draw() {
		if (isSuv) {
			drawCarSuv();
		}
		else {
			drawCarSedan();
		}
	}

	Car() {
		static int nextId = 0;
		id = nextId++;
		colorRed = 0.2 + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 0.6));
		colorGreen = 0.2 + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 0.6));
		colorBlue = 0.2 + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 0.6));
		isSuv = (std::rand() % 2 == 1);
	}

	void drawCarSuv() {
		//левая грань
		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(-3, -2, 0);
		glVertex3f(-3, -2, 2.5);
		glVertex3f(8, -2, 2.5);
		glVertex3f(8, -2, 0);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(8, -2, 0);
		glVertex3f(8, -2, 2.5);
		glVertex3f(8.3, -2, 2.5);
		glVertex3f(8.3, -2, 1);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(-8, -2, 1);
		glVertex3f(-8, -2, 2);
		glVertex3f(-3, -2, 2.5);
		glVertex3f(-3, -2, 0);
		glEnd();


		//правая грань
		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(-3, 2, 0);
		glVertex3f(-3, 2, 2.5);
		glVertex3f(8, 2, 2.5);
		glVertex3f(8, 2, 0);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(8, 2, 0);
		glVertex3f(8, 2, 2.5);
		glVertex3f(8.3, 2, 2.5);
		glVertex3f(8.3, 2, 1);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(-8, 2, 1);
		glVertex3f(-8, 2, 2);
		glVertex3f(-3, 2, 2.5);
		glVertex3f(-3, 2, 0);
		glEnd();


		//перед
		glBegin(GL_QUADS);
		glColor3d(colorRed + 0.1, colorGreen + 0.1, colorBlue + 0.1);
		glVertex3f(-3, -2, 2.5);
		glVertex3f(-8, -2, 2);
		glVertex3f(-8, 2, 2);
		glVertex3f(-3, 2, 2.5);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed + 0.1, colorGreen + 0.1, colorBlue + 0.1);
		glVertex3f(-8, -2, 2);
		glVertex3f(-8, -2, 1);
		glVertex3f(-8, 2, 1);
		glVertex3f(-8, 2, 2);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed - 0.1, colorGreen - 0.1, colorBlue - 0.1);
		glVertex3f(-8, 2, 1);
		glVertex3f(-8, -2, 1);
		glVertex3f(-3, -2, 0);
		glVertex3f(-3, 2, 0);
		glEnd();

		//зад
		glBegin(GL_QUADS);
		glColor3d(colorRed + 0.1, colorGreen + 0.1, colorBlue + 0.1);
		glVertex3f(8, -2, 2.5);
		glVertex3f(8, 2, 2.5);
		glVertex3f(8, 2, 2.5);
		glVertex3f(8, -2, 2.5);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed + 0.1, colorGreen + 0.1, colorBlue + 0.1);
		glVertex3f(8, 2, 2.5);
		glVertex3f(8, -2, 2.5);
		glVertex3f(8.3, -2, 1);
		glVertex3f(8.3, 2, 1);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed - 0.1, colorGreen - 0.1, colorBlue - 0.1);
		glVertex3f(8.3, 2, 1);
		glVertex3f(8.3, -2, 1);
		glVertex3f(8, -2, 0);
		glVertex3f(8, 2, 0);
		glEnd();


		//дно
		glBegin(GL_QUADS);
		glColor3d(colorRed - 0.2, colorGreen - 0.2, colorBlue - 0.2);
		glVertex3f(-3, -2, 0);
		glVertex3f(-3, 2, 0);
		glVertex3f(8, 2, 0);
		glVertex3f(8, -2, 0);
		glEnd();


		//стойки
		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(-3, -2, 2.5);
		glVertex3f(-1.1, -1.5, 7);
		glVertex3f(-1, -1.5, 7);
		glVertex3f(-2.7, -2, 2.5);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(1.5, -2, 2.5);
		glVertex3f(1.5, -1.5, 7);
		glVertex3f(1.6, -1.5, 7);
		glVertex3f(1.8, -2, 2.5);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(7.7, -2, 2.5);
		glVertex3f(7.6, -1.5, 7);
		glVertex3f(7.7, -1.5, 7);
		glVertex3f(8, -2, 2.5);
		glEnd();


		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(-3, 2, 2.5);
		glVertex3f(-1.1, 1.5, 7);
		glVertex3f(-1, 1.5, 7);
		glVertex3f(-2.7, 2, 2.5);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(1.5, 2, 2.5);
		glVertex3f(1.5, 1.5, 7);
		glVertex3f(1.6, 1.5, 7);
		glVertex3f(1.8, 2, 2.5);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(7.7, 2, 2.5);
		glVertex3f(7.6, 1.5, 7);
		glVertex3f(7.7, 1.5, 7);
		glVertex3f(8, 2, 2.5);
		glEnd();


		glBegin(GL_QUADS);
		glColor3d(colorRed + 0.1, colorGreen + 0.1, colorBlue + 0.1);
		glVertex3f(-3, -2, 2.5);
		glVertex3f(-3, -1.7, 2.5);
		glVertex3f(-1.1, -1.4, 7);
		glVertex3f(-1.1, -1.5, 7);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed + 0.1, colorGreen + 0.1, colorBlue + 0.1);
		glVertex3f(-3, 2, 2.5);
		glVertex3f(-3, 1.7, 2.5);
		glVertex3f(-1.1, 1.4, 7);
		glVertex3f(-1.1, 1.5, 7);
		glEnd();


		glBegin(GL_QUADS);
		glColor3d(colorRed + 0.1, colorGreen + 0.1, colorBlue + 0.1);
		glVertex3f(8, -2, 2.5);
		glVertex3f(7.7, -1.5, 7);
		glVertex3f(7.7, -1.4, 7);
		glVertex3f(8, -1.7, 2.5);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed + 0.1, colorGreen + 0.1, colorBlue + 0.1);
		glVertex3f(8, 1.7, 2.5);
		glVertex3f(7.7, 1.4, 7);
		glVertex3f(7.7, 1.5, 7);
		glVertex3f(8, 2, 2.5);
		glEnd();



		//крыша
		glBegin(GL_QUADS);
		glColor3d(colorRed + 0.2, colorGreen + 0.2, colorBlue + 0.2);
		glVertex3f(-1.1, -1.5, 7);
		glVertex3f(-1.1, 1.5, 7);
		glVertex3f(7.7, 1.5, 7);
		glVertex3f(7.7, -1.5, 7);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(-1.1, -1.5, 6.9);
		glVertex3f(-1.1, 1.5, 6.9);
		glVertex3f(7.7, 1.5, 6.9);
		glVertex3f(7.7, -1.5, 6.9);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed + 0.1, colorGreen + 0.1, colorBlue + 0.1);
		glVertex3f(-1.1, -1.5, 7);
		glVertex3f(-1.1, 1.5, 7);
		glVertex3f(-1.1, 1.5, 6.9);
		glVertex3f(-1.1, -1.5, 6.9);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed + 0.1, colorGreen + 0.1, colorBlue + 0.1);
		glVertex3f(7.7, -1.5, 7);
		glVertex3f(7.7, 1.5, 7);
		glVertex3f(7.7, 1.5, 6.9);
		glVertex3f(7.7, -1.5, 6.9);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(-1.1, 1.5, 7);
		glVertex3f(7.7, 1.5, 7);
		glVertex3f(7.7, 1.5, 6.9);
		glVertex3f(-1.1, 1.5, 6.9);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(-1.1, -1.5, 7);
		glVertex3f(7.7, -1.5, 7);
		glVertex3f(7.7, -1.5, 6.9);
		glVertex3f(-1.1, -1.5, 6.9);
		glEnd();

		drawWheel(-2.5, -2.3, 0.5);
		drawWheel(-2.5, 2.3, 0.5);
		drawWheel(3.3, -2.3, 0.5);
		drawWheel(3.3, 2.3, 0.5);
		drawVihlop(8, 1, 1);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glBegin(GL_QUADS);
		glColor4f(0.6, 0.6, 0.6, 0.3);
		glVertex3f(-1.0, -1.5, 7.0);
		glVertex3f(1.5, -1.5, 7.0);
		glVertex3f(1.5, -2.0, 2.5);
		glVertex3f(-2.7, -2.0, 2.5);
		glEnd();

		glBegin(GL_QUADS);
		glColor4f(0.6, 0.6, 0.6, 0.3);
		glVertex3f(1.6, -1.5, 7.0);
		glVertex3f(7.6, -1.5, 7.0);
		glVertex3f(7.7, -2.0, 2.5);
		glVertex3f(1.8, -2.0, 2.5);
		glEnd();

		glBegin(GL_QUADS);
		glColor4f(0.6, 0.6, 0.6, 0.3);
		glVertex3f(-1.0, 1.5, 7.0);
		glVertex3f(1.5, 1.5, 7.0);
		glVertex3f(1.5, 2.0, 2.5);
		glVertex3f(-2.7, 2.0, 2.5);
		glEnd();

		glBegin(GL_QUADS);
		glColor4f(0.6, 0.6, 0.6, 0.3);
		glVertex3f(1.6, 1.5, 7.0);
		glVertex3f(7.6, 1.5, 7.0);
		glVertex3f(7.7, 2.0, 2.5);
		glVertex3f(1.8, 2.0, 2.5);
		glEnd();

		glBegin(GL_QUADS);
		glColor4f(0.6, 0.6, 0.6, 0.3);
		glVertex3f(-3.0, -1.7, 2.5);
		glVertex3f(-3.0, 1.7, 2.5);
		glVertex3f(-1.1, 1.4, 7.0);
		glVertex3f(-1.1, -1.4, 7.0);
		glEnd();

		glBegin(GL_QUADS);
		glColor4f(0.6, 0.6, 0.6, 0.3);
		glVertex3f(8.0, -1.7, 2.5);
		glVertex3f(8.0, 1.7, 2.5);
		glVertex3f(7.7, 1.4, 7.0);
		glVertex3f(7.7, -1.4, 7.0);
		glEnd();

		glDisable(GL_BLEND);

		drawLights(-8.01, -1.7, 1.5, -0.1, 1, 0.5, 0);
		drawLights(-8.01, 0.7, 1.5, -0.1, 1, 0.5, 0);

		drawLights(8.3, -1.7, 1.5, -0.1, 1, 0.5, 1);
		drawLights(8.3, 0.7, 1.5, -0.1, 1, 0.5, 1);
	}

	void drawCarSedan() {
		drawWheel(-2, -2.3, 0.5);
		drawWheel(-2, 2.3, 0.5);
		drawWheel(3, -2.3, 0.5);
		drawWheel(3, 2.3, 0.5);

		drawVihlop(5.5, 1, 1);
		
		

		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(-3, -2, 0);
		glVertex3f(-3, -2, 2.5);
		glVertex3f(4, -2, 2.5);
		glVertex3f(4, -2, 0);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(4, -2, 0);
		glVertex3f(4, -2, 2.5);
		glVertex3f(5, -2, 2.5);
		glVertex3f(5.5, -2, 1);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(-5, -2, 1);
		glVertex3f(-5, -2, 2);
		glVertex3f(-3, -2, 2.5);
		glVertex3f(-3, -2, 0);
		glEnd();


		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(-3, 2, 0);
		glVertex3f(-3, 2, 2.5);
		glVertex3f(4, 2, 2.5);
		glVertex3f(4, 2, 0);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(4, 2, 0);
		glVertex3f(4, 2, 2.5);
		glVertex3f(5, 2, 2.5);
		glVertex3f(5.5, 2, 1);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(-5, 2, 1);
		glVertex3f(-5, 2, 2);
		glVertex3f(-3, 2, 2.5);
		glVertex3f(-3, 2, 0);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed+0.1, colorGreen+0.1, colorBlue+0.1);
		glVertex3f(-3, -2, 2.5);
		glVertex3f(-5, -2, 2);
		glVertex3f(-5, 2, 2);
		glVertex3f(-3, 2, 2.5);
		glEnd();
		

		glBegin(GL_QUADS);
		glColor3d(colorRed+0.1, colorGreen+0.1, colorBlue+0.1);
		glVertex3f(-5, -2, 2);
		glVertex3f(-5, -2, 1);
		glVertex3f(-5, 2, 1);
		glVertex3f(-5, 2, 2);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed-0.1, colorGreen-0.1, colorBlue-0.1);
		glVertex3f(-5, 2, 1);
		glVertex3f(-5, -2, 1);
		glVertex3f(-3, -2, 0);
		glVertex3f(-3, 2, 0);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed+0.1, colorGreen+0.1, colorBlue+0.1);
		glVertex3f(4, -2, 2.5);
		glVertex3f(4, 2, 2.5);
		glVertex3f(5, 2, 2.5);
		glVertex3f(5, -2, 2.5);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed+0.1, colorGreen+0.1, colorBlue+0.1);
		glVertex3f(5, 2, 2.5);
		glVertex3f(5, -2, 2.5);
		glVertex3f(5.5, -2, 1);
		glVertex3f(5.5, 2, 1);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed-0.1, colorGreen-0.1, colorBlue-0.1);
		glVertex3f(5.5, 2, 1);
		glVertex3f(5.5, -2, 1);
		glVertex3f(4, -2, 0);
		glVertex3f(4, 2, 0);
		glEnd();


		glBegin(GL_QUADS);
		glColor3d(colorRed-0.2, colorGreen-0.2, colorBlue-0.2);
		glVertex3f(-3, -2, 0);
		glVertex3f(-3, 2, 0);
		glVertex3f(4, 2, 0);
		glVertex3f(4, -2, 0);
		glEnd();



		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(-3, -2, 2.5);
		glVertex3f(-1.1, -1.5, 7);
		glVertex3f(-1, -1.5, 7);
		glVertex3f(-2.7, -2, 2.5);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(1.5, -2, 2.5);
		glVertex3f(1.5, -1.5, 7);
		glVertex3f(1.6, -1.5, 7);
		glVertex3f(1.8, -2, 2.5);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(3.7, -2, 2.5);
		glVertex3f(2.5, -1.5, 7);
		glVertex3f(2.6, -1.5, 7);
		glVertex3f(4, -2, 2.5);
		glEnd();




		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(-3, 2, 2.5);
		glVertex3f(-1.1, 1.5, 7);
		glVertex3f(-1, 1.5, 7);
		glVertex3f(-2.7, 2, 2.5);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(1.5, 2, 2.5);
		glVertex3f(1.5, 1.5, 7);
		glVertex3f(1.6, 1.5, 7);
		glVertex3f(1.8, 2, 2.5);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(3.7, 2, 2.5);
		glVertex3f(2.5, 1.5, 7);
		glVertex3f(2.6, 1.5, 7);
		glVertex3f(4, 2, 2.5);
		glEnd();


		glBegin(GL_QUADS);
		glColor3d(colorRed+0.1, colorGreen+0.1, colorBlue+0.1);
		glVertex3f(-3, -2, 2.5);
		glVertex3f(-3, -1.7, 2.5);
		glVertex3f(-1.1, -1.4, 7);
		glVertex3f(-1.1, -1.5, 7);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed+0.1, colorGreen+0.1, colorBlue+0.1);
		glVertex3f(-3, 2, 2.5);
		glVertex3f(-3, 1.7, 2.5);
		glVertex3f(-1.1, 1.4, 7);
		glVertex3f(-1.1, 1.5, 7);
		glEnd();


		glBegin(GL_QUADS);
		glColor3d(colorRed+0.1, colorGreen+0.1, colorBlue+0.1);
		glVertex3f(4, -2, 2.5);
		glVertex3f(2.6, -1.5, 7);
		glVertex3f(2.6, -1.4, 7);
		glVertex3f(4, -1.7, 2.5);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed+0.1, colorGreen+0.1, colorBlue+0.1);
		glVertex3f(4, 1.7, 2.5);
		glVertex3f(2.6, 1.4, 7);
		glVertex3f(2.6, 1.5, 7);
		glVertex3f(4, 2, 2.5);
		glEnd();


		glBegin(GL_QUADS);
		glColor3d(colorRed+0.2, colorGreen+0.2, colorBlue+0.2);
		glVertex3f(-1.1, -1.5, 7);
		glVertex3f(-1.1, 1.5, 7);
		glVertex3f(2.6, 1.5, 7);
		glVertex3f(2.6, -1.5, 7);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(-1.1, -1.5, 6.9);
		glVertex3f(-1.1, 1.5, 6.9);
		glVertex3f(2.6, 1.5, 6.9);
		glVertex3f(2.6, -1.5, 6.9);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed+0.1, colorGreen+0.1, colorBlue+0.1);
		glVertex3f(-1.1, -1.5, 7);
		glVertex3f(-1.1, 1.5, 7);
		glVertex3f(-1.1, 1.5, 6.9);
		glVertex3f(-1.1, -1.5, 6.9);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed+0.1, colorGreen+0.1, colorBlue+0.1);
		glVertex3f(2.6, -1.5, 7);
		glVertex3f(2.6, 1.5, 7);
		glVertex3f(2.6, 1.5, 6.9);
		glVertex3f(2.6, -1.5, 6.9);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(-1.1, 1.5, 7);
		glVertex3f(2.6, 1.5, 7);
		glVertex3f(2.6, 1.5, 6.9);
		glVertex3f(-1.1, 1.5, 6.9);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(colorRed, colorGreen, colorBlue);
		glVertex3f(-1.1, -1.5, 7);
		glVertex3f(2.6, -1.5, 7);
		glVertex3f(2.6, -1.5, 6.9);
		glVertex3f(-1.1, -1.5, 6.9);
		glEnd();


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glBegin(GL_QUADS);
		glColor4f(0.6, 0.6, 0.6, 0.3);
		glVertex3f(-1.0, -1.5, 7.0);
		glVertex3f(1.5, -1.5, 7.0);
		glVertex3f(1.5, -2.0, 2.5);
		glVertex3f(-2.7, -2.0, 2.5);
		glEnd();

		glBegin(GL_QUADS);
		glColor4f(0.6, 0.6, 0.6, 0.3); 
		glVertex3f(1.6, -1.5, 7.0);
		glVertex3f(2.5, -1.5, 7.0);
		glVertex3f(3.7, -2.0, 2.5);
		glVertex3f(1.8, -2.0, 2.5);
		glEnd();

		glBegin(GL_QUADS);
		glColor4f(0.6, 0.6, 0.6, 0.3);
		glVertex3f(-1.0, 1.5, 7.0);
		glVertex3f(1.5, 1.5, 7.0);
		glVertex3f(1.5, 2.0, 2.5);
		glVertex3f(-2.7, 2.0, 2.5);
		glEnd();

		glBegin(GL_QUADS);
		glColor4f(0.6, 0.6, 0.6, 0.3);
		glVertex3f(1.6, 1.5, 7.0);
		glVertex3f(2.5, 1.5, 7.0);
		glVertex3f(3.7, 2.0, 2.5);
		glVertex3f(1.8, 2.0, 2.5);
		glEnd();

		glBegin(GL_QUADS);
		glColor4f(0.6, 0.6, 0.6, 0.3); 
		glVertex3f(-3.0, -1.7, 2.5);
		glVertex3f(-3.0, 1.7, 2.5);
		glVertex3f(-1.1, 1.4, 7.0);
		glVertex3f(-1.1, -1.4, 7.0);
		glEnd();

		glBegin(GL_QUADS);
		glColor4f(0.6, 0.6, 0.6, 0.3);
		glVertex3f(4.0, -1.7, 2.5);
		glVertex3f(4.0, 1.7, 2.5);
		glVertex3f(2.6, 1.4, 7.0);
		glVertex3f(2.6, -1.4, 7.0);
		glEnd();


		glDisable(GL_BLEND);

		drawLights(-5.01, -1.7, 1.5, -0.1, 1, 0.5, 0);
		drawLights(-5.01, 0.7, 1.5, -0.1, 1, 0.5, 0);

		drawLights(5.3, -1.7, 1.5, -0.1, 1, 0.5, 1);
		drawLights(5.3, 0.7, 1.5, -0.1, 1, 0.5, 1);
	}
};



std::vector<roadSign> signs;
std::vector<Car> cars;
std::vector<Car> oncomingCars;


bool trafficStopped = false;
float savedSpeed = 20;

void SetTrafficState() {
	trafficStopped = !trafficStopped;
}

void UpdateCars(double delta_time) {
	float spawnInterval = 2;
	const float leftBound = 40;
	const float rightBound = -40;

	static float nextCarTime = 0;
	static float nextOncomingCarTime = 0;

	nextCarTime += delta_time;
	nextOncomingCarTime += delta_time;

	if (nextCarTime >= spawnInterval && !trafficStopped) {
		cars.emplace_back();
		cars.back().positionX = leftBound;
		nextCarTime = 0;
	}

	if (nextOncomingCarTime >= spawnInterval * 2 && !trafficStopped) {
		oncomingCars.emplace_back();
		oncomingCars.back().positionX = rightBound;
		nextOncomingCarTime = 0;
	}

	float currentSpeed = trafficStopped ? 0 : savedSpeed;

	for (auto it = cars.begin(); it != cars.end(); ) {
		it->positionX += currentSpeed * -delta_time;
		if (it->positionX < rightBound) {
			it = cars.erase(it);
		}
		else {
			++it;
		}
	}

	for (auto it = oncomingCars.begin(); it != oncomingCars.end(); ) {
		it->positionX += currentSpeed * delta_time;
		if (it->positionX > leftBound) {
			it = oncomingCars.erase(it);
		}
		else {
			++it;
		}
	}
}

void DrawCars() {
	for (auto& car : cars) {
		glPushMatrix();
		glTranslatef(car.positionX, 4, 0);
		car.draw();
		glPopMatrix();
	}

	for (auto& car : oncomingCars) {
		glPushMatrix();
		glTranslatef(car.positionX, -4, 0);
		glScalef(-1, 1, 1);
		car.draw();
		glPopMatrix();
	}
}

void createRandomSigns(int count) {
	for (auto& sign : signs) {
		sign.resetSpawnAnimation();
	}

	signs.clear();

	srand(static_cast<unsigned int>(time(nullptr)));

	for (int i = 0; i < count; ++i) {
		int type = rand() % 3;
		int x = -60 + rand() % 120;
		int y = (rand() % 2) ? 9.95 : -9.95;
		int z = 0;

		signs.emplace_back(type, x, y, z);
	}
}

#include "MyOGL.h"
extern OpenGL gl;
#include "Light.h"
Light light;
#include "Camera.h"
Camera camera;

bool simulation = true;

void switchModes(OpenGL* sender, KeyEventArg arg) {
	auto key = LOWORD(MapVirtualKeyA(arg.key, MAPVK_VK_TO_CHAR));

	switch (key) {
	case 'S':
		createRandomSigns(4);
		break;
	case ' ':
		SetTrafficState();
		simulation = !simulation;
		break;
	}
}

template<typename T, int M1, int N1, int M2, int N2>
void MatrixMultiply(const T* a, const T* b, T* c) {
	for (int i = 0; i < M1; ++i) {
		for (int j = 0; j < N2; ++j) {
			c[i * N2 + j] = T(0);
			for (int k = 0; k < N1; ++k) {
				c[i * N2 + j] += a[i * N1 + k] * b[k * N2 + j];
			}
		}
	}
}

GuiTextRectangle text;

ObjModel f;

Shader cassini_sh;
Shader phong_sh;
Shader vb_sh;
Shader simple_texture_sh;

Texture road_texture;

void initRender() {
	srand(static_cast<unsigned int>(time(nullptr)));

	cassini_sh.VshaderFileName = "shaders/v.vert";
	cassini_sh.FshaderFileName = "shaders/cassini.frag";
	cassini_sh.LoadShaderFromFile();
	cassini_sh.Compile();

	phong_sh.VshaderFileName = "shaders/v.vert";
	phong_sh.FshaderFileName = "shaders/light.frag";
	phong_sh.LoadShaderFromFile();
	phong_sh.Compile();

	vb_sh.VshaderFileName = "shaders/v.vert";
	vb_sh.FshaderFileName = "shaders/vb.frag";
	vb_sh.LoadShaderFromFile();
	vb_sh.Compile();

	simple_texture_sh.VshaderFileName = "shaders/v.vert";
	simple_texture_sh.FshaderFileName = "shaders/textureShader.frag";
	simple_texture_sh.LoadShaderFromFile();
	simple_texture_sh.Compile();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	camera.caclulateCameraPos();

	gl.WheelEvent.reaction(&camera, &Camera::Zoom);
	gl.MouseMovieEvent.reaction(&camera, &Camera::MouseMovie);
	gl.MouseLeaveEvent.reaction(&camera, &Camera::MouseLeave);
	gl.MouseLdownEvent.reaction(&camera, &Camera::MouseStartDrag);
	gl.MouseLupEvent.reaction(&camera, &Camera::MouseStopDrag);

	gl.MouseMovieEvent.reaction(&light, &Light::MoveLight);
	gl.KeyDownEvent.reaction(&light, &Light::StartDrug);
	gl.KeyUpEvent.reaction(&light, &Light::StopDrug);

	gl.KeyDownEvent.reaction(switchModes);
	text.setSize(512, 180);

	camera.setPosition(2, 1.5, 1.5);
}

float view_matrix[16];
double full_time = 0;
int location = 0;

void Render(double delta_time) {
	full_time += delta_time;

	if (gl.isKeyPressed('F')) {
		light.SetPosition(camera.x(), camera.y(), camera.z());
	}
	camera.SetUpCamera();
	glGetFloatv(GL_MODELVIEW_MATRIX, view_matrix);

	light.SetUpLight();

	gl.DrawAxes();

	glEnable(GL_NORMALIZE);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	UpdateCars(delta_time);
	DrawCars();

	for (auto& sign : signs) {
		sign.spawn();
	}

	road_texture.LoadTexture("textures/road.jpg");
	unsigned int texId = road_texture.GetID();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texId);

	glBegin(GL_QUADS);
	glColor3f(1, 1, 1);
	glTexCoord2f(2, 1); glVertex3f(60, 10, -0.2);
	glTexCoord2f(2, 0); glVertex3f(60, -10, -0.2);
	glTexCoord2f(0, 0); glVertex3f(-60, -10, -0.2);
	glTexCoord2f(0, 1); glVertex3f(-60, 10, -0.2);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glLoadIdentity();
	camera.SetUpCamera();
	Shader::DontUseShaders();
	light.DrawLightGizmo();

	glActiveTexture(GL_TEXTURE0);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glOrtho(0, gl.getWidth() - 1, 0, gl.getHeight() - 1, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	std::wstringstream ss;
	ss << std::fixed << std::setprecision(3);
	ss << "Space - " << (simulation ? L"[вкл]выкл  " : L" вкл[выкл] ") << L"симуляции" << std::endl;
	ss << "S - Spawn road signs" << std::endl;
	ss << L"delta_time: " << std::setprecision(5) << delta_time << std::endl;
	ss << L"full_time: " << std::setprecision(2) << full_time << std::endl;

	text.setPosition(10, gl.getHeight() - 10 - 180);
	text.setText(ss.str().c_str());

	text.Draw();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}