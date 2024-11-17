#include <GL/freeglut.h>

// Definir los colores
float white[] = { 1.0f, 1.0f, 1.0f };
float gold[] = { 1.0f, 0.84f, 0.0f };
float brown[] = { 0.6f, 0.3f, 0.1f };
float gray[] = { 0.5f, 0.5f, 0.5f };

void drawPixel(float x, float y, float color[]) {
    glColor3fv(color);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + 0.5f, y);
    glVertex2f(x + 0.5f, y + 0.5f);
    glVertex2f(x, y + 0.5f);
    glEnd();
}

void drawKing() {
    // Cuerpo del rey (caja marrón)
    for (float x = 0; x < 2; x += 0.5f) {
        for (float y = 0; y < 2.5f; y += 0.5f) {
            drawPixel(x, y, brown); // El cuerpo del rey es marrón
        }
    }

    // Cabeza del rey (caja blanca)
    for (float x = 0.5f; x < 1.5f; x += 0.5f) {
        for (float y = 2.5f; y < 3.0f; y += 0.5f) {
            drawPixel(x, y, white); // La cabeza es blanca
        }
    }

    // Ojos del rey
    drawPixel(0.75f, 2.75f, gray); // Ojo izquierdo
    drawPixel(1.25f, 2.75f, gray); // Ojo derecho

    // Corona del rey (parte superior)
    drawPixel(0.5f, 3.0f, gold); // Corona izquierda
    drawPixel(1.0f, 3.0f, gold); // Corona central
    drawPixel(1.5f, 3.0f, gold); // Corona derecha

    // Coronas adicionales (en la parte superior de la central)
    drawPixel(0.5f, 3.5f, gold); // Punta izquierda
    drawPixel(1.0f, 3.5f, gold); // Punta central
    drawPixel(1.5f, 3.5f, gold); // Punta derecha
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    drawKing();  // Dibuja el rey
    glFlush();
}

void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glOrtho(0.0, 2.0, 0.0, 4.0, -1.0, 1.0); // Ajustar la vista
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(400, 600);
    glutCreateWindow("Pixel Art Rey en GLUT");
    init();
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}
