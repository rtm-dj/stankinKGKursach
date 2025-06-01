#include <Windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>


void drawCarSedan() {
	glBegin(GL_QUADS);

	glColor3d(0.5, 0.7, 0.5);

	glVertex3s(1, 0, 0);
	glVertex3s(3, 5, 0);
	glVertex3s(0, 1, 0);
	glVertex3s(-1, -2, 0);
	glEnd();
}
