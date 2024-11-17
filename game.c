#include <GL/glut.h>
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define GRID_SIZE 1000
#define CELL_SIZE 0.010f

// Variables globales para el zoom
float zoomLevel = 1.0f; // Nivel de zoom (1.0 es el predeterminado)
float orthoLeft = -1.0f, orthoRight = 11.0f, orthoBottom = -1.0f, orthoTop = 11.0f;

bool MainGrid = true;  // Controla el dibujo del grid principal
bool SubGrid = true;  // Controla el dibujo del subgrid

int isKingSelected = 0;

typedef struct {
    float r, g, b; // Color de la celda
    float x1, y1, x2, y2, x3, y3, x4, y4; // Coordenadas de los vértices
} Cell;

Cell grid[GRID_SIZE][GRID_SIZE]; // Matriz para almacenar la información de la cuadrícula

typedef struct {
    int life = 1000;
    float speed = 0.4;
    int cost = 500;
    int damage = 80;
    float x = 1.0;
    float y = 1.0;
} King;

King king;
typedef struct {
    int life = 150;
    float speed = 0.4;
    int cost = 50;
    int damage = 0;
} Constructor;

typedef struct {
    int life = 350;
    float speed = 0.4;
    int cost = 100;
    int damage = 50;
} Guard;

// FUNCIONES PARA PERSONAJES
void drawKing() {
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex3f(king.x, king.y, 0.0f);
    glVertex3f(king.x + 0.25f, king.y, 0.0f);
    glVertex3f(king.x + 0.25f, king.y + 0.5f, 0.0f);
    glVertex3f(king.x, king.y + 0.5f, 0.0f);
    glEnd();
}

void drawPoint(float x, float y) {
    glColor3f(1.0f, 0.0f, 0.0f); // color rojo para el punto
    glPointSize(5.0f); // tamaño del punto

    glBegin(GL_POINTS);
    glVertex2f(x, y);
    glEnd();
}

// Dibuja un marco alrededor del King si está seleccionado
void drawSelection() {
    if (isKingSelected) {
        glColor3f(1.0f, 0.0f, 1.0f); // Color del marco
        glLineWidth(3.0f);
        glBegin(GL_LINE_LOOP);
        glVertex3f(king.x, king.y, 0.0f);
        glVertex3f(king.x + 0.25f, king.y, 0.0f);
        glVertex3f(king.x + 0.25f, king.y + 0.5f, 0.0f);
        glVertex3f(king.x, king.y + 0.5f, 0.0f);
        glEnd();
    }
}

void selectionPerson(int x, int y) {
    // Convertir las coordenadas de pantalla a coordenadas del mundo
    float winX = (float)x / 350.0f - 1.0f;  // Asumiendo una ventana de 700x700
    float winY = 10.0f - (float)y / 350.0f;  // Invertir Y para la coordenada de OpenGL

    // Verificar si el clic está dentro del área del King
    if (winX >= king.x && winX <= king.x + 0.25f && winY >= king.y && winY <= king.y + 0.5f) {
        drawSelection();
    }
}

// FUNCIONES PARA EL MAPA
void drawPixel(const Cell* cell) {
    glColor3f(cell->r, cell->g, cell->b);

    glBegin(GL_QUADS);
    glVertex3f(cell->x1, cell->y1, 0.0f);
    glVertex3f(cell->x2, cell->y2, 0.0f);
    glVertex3f(cell->x3, cell->y3, 0.0f);
    glVertex3f(cell->x4, cell->y4, 0.0f);
    glEnd();
}

void initializeSeed() {
    for (int i = 10; i < GRID_SIZE - 10; ++i) {
        for (int j = 10; j < GRID_SIZE - 10; ++j) {
            // Define un tono base de verde y pequeñas variaciones
            float g = 0.5f + (rand() % 20) / 100.0f; // Base para verde
            float r = g * 0.7f + (rand() % 10) / 100.0f; // Rojo menos dominante
            float b = g * 0.4f + (rand() % 10) / 100.0f; // Azul menos dominante

            // Calcula las coordenadas de la celda
            float x1 = i * CELL_SIZE;
            float y1 = j * CELL_SIZE;
            float x2 = (i + 1) * CELL_SIZE;
            float y2 = j * CELL_SIZE;
            float x3 = (i + 1) * CELL_SIZE;
            float y3 = (j + 1) * CELL_SIZE;
            float x4 = i * CELL_SIZE;
            float y4 = (j + 1) * CELL_SIZE;

            // Almacena la información en la cuadrícula
            grid[i][j] = (Cell){ r, g, b, x1, y1, x2, y2, x3, y3, x4, y4 };
        }
    }
}

void fillGrid() {
    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            drawPixel(&grid[i][j]);
        }
    }
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
    glColor3f(1.0f, 0.796f, 0.588f);
    glLineWidth(20.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(0, 0);
    glVertex2f(10, 0);
    glVertex2f(10, 10);
    glVertex2f(0, 10);
    glEnd();
}

// FUNCIONES DE VENTANA
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    fillGrid();

    if (SubGrid) {
        drawSubGrid();
    }
    if (MainGrid) {
        drawGrid();
    }

    drawBorders();
    drawAxisNumbers();
    drawKing();

    glutSwapBuffers();
}


void init() {
    glClearColor(0.549f, 0.835f, 0.953f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 11.0, -1.0, 11.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    initializeSeed(); // Inicializa la cuadrícula al inicio
}

void mouseWheel(int button, int dir, int x, int y) {
    float zoomFactor = 0.05f; // Ajuste más fino para un zoom más fluido

    if (dir > 0) {  // Rueda hacia adelante (zoom in)
        zoomLevel *= (1.0f - zoomFactor); // Reducir el nivel de zoom suavemente
    }
    else if (dir < 0) {  // Rueda hacia atrás (zoom out)
        zoomLevel *= (1.0f + zoomFactor); // Aumentar el nivel de zoom suavemente
    }

    // Limitar el rango de zoom para evitar valores extremos
    if (zoomLevel < 0.1f) zoomLevel = 0.1f;   // Límite mínimo
    if (zoomLevel > 10.0f) zoomLevel = 10.0f; // Límite máximo

    // Ajustar los límites ortográficos según el nivel de zoom
    float centerX = (orthoLeft + orthoRight) / 2.0f;
    float centerY = (orthoBottom + orthoTop) / 2.0f;
    float width = (orthoRight - orthoLeft) * zoomLevel;
    float height = (orthoTop - orthoBottom) * zoomLevel;

    orthoLeft = centerX - width / 2.0f;
    orthoRight = centerX + width / 2.0f;
    orthoBottom = centerY - height / 2.0f;
    orthoTop = centerY + height / 2.0f;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(orthoLeft, orthoRight, orthoBottom, orthoTop);

    glutPostRedisplay();  // Redibujar la ventana
}

// Función para transformar coordenadas de pantalla a coordenadas de mundo
void screenToWorld(int screenX, int screenY, float* worldX, float* worldY) {
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    *worldX = orthoLeft + (screenX / (float)viewport[2]) * (orthoRight - orthoLeft);
    *worldY = orthoBottom + ((viewport[3] - screenY) / (float)viewport[3]) * (orthoTop - orthoBottom);
}

// Función para manejar clic derecho (selección)
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        float worldX, worldY;
        screenToWorld(x, y, &worldX, &worldY);

        // Verificar si el clic está dentro del área del King
        if (worldX >= king.x && worldX <= king.x + 0.25f &&
            worldY >= king.y && worldY <= king.y + 0.5f) {
            isKingSelected = 1; // Seleccionar el King
        }
        else {
            isKingSelected = 0; // Deseleccionar si se hace clic fuera
        }

        glutPostRedisplay(); // Redibujar la ventana
    }
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 'm' || key == 'M') { // Reiniciar el zoom
        zoomLevel = 1.0f;
        orthoLeft = -1.0f;
        orthoRight = 11.0f;
        orthoBottom = -1.0f;
        orthoTop = 11.0f;

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(orthoLeft, orthoRight, orthoBottom, orthoTop);

        glutPostRedisplay();
    }
    else if (key == 'n' || key == 'N') { // Alternar visibilidad de las cuadrículas
        MainGrid = !MainGrid;
        SubGrid = !SubGrid;
        glutPostRedisplay(); // Redibujar la ventana
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(700, 700);
    glutInitWindowPosition(410, 50);
    glutCreateWindow("Conquer & Win!");

    init();

    glutDisplayFunc(display);
    glutMouseWheelFunc(mouseWheel);
    glutKeyboardFunc(keyboard);

    glutMainLoop();

    return 0;
}