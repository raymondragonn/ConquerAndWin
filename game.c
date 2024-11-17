#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>

// Definición de colores para los terrenos
const float GRASS_COLOR[3] = { 0.2f, 0.8f, 0.2f };     // Verde para pasto
const float STONE_COLOR[3] = { 0.6f, 0.6f, 0.6f };     // Gris para piedra
const float WATER_COLOR[3] = { 0.0f, 0.4f, 0.8f };     // Azul para agua
const float HERO_BASE[3] = { 0.3f, 0.9f, 0.3f };       // Verde más claro para base de héroes
const float ENEMY_BASE[3] = { 0.15f, 0.6f, 0.15f };    // Verde más oscuro para base enemiga

// Matriz que define el mapa (10x10)
// 0: Pasto, 1: Piedra, 2: Agua, 3: Base Héroes, 4: Base Enemiga
int mapa[10][10] = {
    {3, 0, 0, 2, 2, 1, 0, 1, 1, 0},
    {0, 0, 2, 2, 1, 1, 0, 0, 1, 0},
    {0, 2, 2, 1, 1, 0, 0, 1, 0, 0},
    {2, 2, 1, 1, 0, 0, 1, 1, 0, 1},
    {2, 1, 1, 0, 0, 1, 1, 0, 0, 1},
    {1, 1, 0, 0, 1, 1, 0, 0, 1, 1},
    {0, 0, 0, 1, 1, 0, 0, 1, 1, 2},
    {0, 1, 1, 1, 0, 0, 1, 1, 2, 2},
    {1, 1, 0, 0, 0, 1, 1, 2, 2, 4},
    {1, 0, 0, 1, 1, 1, 2, 2, 4, 4}
};


void drawPoint(float x, float y) {
    glColor3f(1.0f, 0.0f, 0.0f); // color rojo para el punto
    glPointSize(5.0f); // tamaño del punto

    glBegin(GL_POINTS);
    glVertex2f(x, y);
    glEnd();
}

void drawPixel(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
    glColor3f(0.5f, 0.5f, 0.5f);

    glBegin(GL_QUADS);
    glVertex3f(x1, y1, 0.0f);
    glVertex3f(x2, y2, 0.0f);
    glVertex3f(x3, y3, 0.0f);
    glVertex3f(x4, y4, 0.50f);
    glEnd();
}

void drawText(const char* text, float x, float y) {
    glPushMatrix();
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
    }
    glPopMatrix();
}

void drawSubGrid() {
    glColor3f(0.9f, 0.9f, 0.9f); // gris claro para el subgrid
    glLineWidth(0.5f);

    glBegin(GL_LINES);

    for (float i = 0; i <= 10; i += 0.25) {
        // líneas verticales
        glVertex2f(i, 0);
        glVertex2f(i, 10);

        // líneas horizontales
        glVertex2f(0, i);
        glVertex2f(10, i);
    }
    glEnd();
}

void drawGrid() {
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    for (int i = 0; i <= 10; i++) {
        // líneas verticales
        glVertex2f(i, 0);
        glVertex2f(i, 10);

        // líneas horizontales
        glVertex2f(0, i);
        glVertex2f(10, i);
    }
    glEnd();
}

void drawAxisNumbers() {
    glColor3f(0.0f, 0.0f, 0.0f);

    // eje X
    for (int i = 0; i <= 10; i++) {
        char buffer[4];
        snprintf(buffer, sizeof(buffer), "%d", i);
        drawText(buffer, i - 0.2f, -0.5f);
    }

    // eje Y
    for (int i = 0; i <= 10; i++) {
        char buffer[4];
        snprintf(buffer, sizeof(buffer), "%d", i);
        drawText(buffer, -0.8f, i - 0.2f);
    }
}

void drawBorders() {
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(0, 0);
    glVertex2f(10, 0);
    glVertex2f(10, 10);
    glVertex2f(0, 10);
    glEnd();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawSubGrid();
    drawGrid();
    drawBorders();
    drawAxisNumbers();

    drawPixel(1.0f, 1.0f, 1.5f, 1.0f, 1.5f, 1.5f, 1.0f, 1.5f);
    drawPoint(1.0f, 1.0f);

    glutSwapBuffers();
}

void idle() {
    glutPostRedisplay();
}

void init() {
    glClearColor(0.15686f, 0.59608f, 0.15686f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 11.0, -1.0, 11.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(700, 700);
    glutInitWindowPosition(410, 50);
    glutCreateWindow("Conquer & Win!");

    init();

    glutDisplayFunc(display);
    glutIdleFunc(idle);

    glutMainLoop();

    return 0;
}