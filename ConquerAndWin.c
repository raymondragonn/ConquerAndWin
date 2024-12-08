#include <GL/glut.h>
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
GLuint textura;

#define GRID_SIZE 300
#define CELL_SIZE (10.0f / GRID_SIZE)
#define MAX_PIXELS 10000
#define MAX_PALMS 1000
#define DIRECTION_NONE -1

float zoomLevel = 1.0f; // Nivel de zoom (1.0 es el predeterminado)
float orthoLeft = -1.0f, orthoRight = 11.0f, orthoBottom = -1.0f, orthoTop = 11.0f;

bool MainGrid = false;  // Controla el dibujo del grid principal
bool SubGrid = false;  // Controla el dibujo del subgrid
bool AxisNumbers = false;

bool palmsGenerated = false;

typedef struct {
    float r, g, b;
    float x1, y1, x2, y2, x3, y3, x4, y4;
} Cell;

Cell grid[GRID_SIZE][GRID_SIZE]; // Matriz para almacenar la información de la cuadrícula

typedef struct {
    float xMin, xMax, yMin, yMax;
} Range;

typedef struct {
    float x, y;
    float r, g, b;
} PalmPixel;

typedef struct {
    PalmPixel pixels[10]; // Arreglo para almacenar la información de la palmera
    int pixelCount;
    float height = 5.0f;
    float width = 0.0f;
    float x;
    float y;
} Palm;

Palm palmPositions[MAX_PALMS];
int palmCount = 0; // Número de palmeras generadas

typedef struct {
    int life;
    float speed;
    int cost;
    int damage;
    int animationState;
    int direction;
    float x;
    float y;
    float height;
    float width;
} Pirate;

Pirate pirate = { 1000, 0.12f, 500, 80, 0, -1, 1.0f, 1.0f, 0.0f, 0.0f };

// Dibujado de objetos
void addPixel(float x, float y, float r, float g, float b);
float randomVariation(float baseValue, float variation);
void createPalm(float x, float y, Palm* palm);
void generatePalms();
void renderPalms();
void drawPirate();
void printPalmPositions();

// Funcionalidades del jugador
bool checkBorderCollision(float newX, float newY);
void adjustCameraToFollowPirate();
void adjustCameraGeneral();

// Mapa
void screenToWorld(int screenX, int screenY, float* worldX, float* worldY);
void drawPixel(const Cell* cell);
void fillGrid();
void initializeSeed(); // Creación del mapa base
void drawBorders(); // Creación de los bordes

// Plano cartesiano
void drawText(const char* text, float x, float y);
void drawSubGrid();
void drawGrid();
void drawAxisNumbers();

// Ventanas
void reshapeWindow(int w, int h);
void createFirstWindow();
void renderWindow1();
void init();
void createSecondaryWindow();
void renderWindow2();
void timer(int value);
void keyboard(unsigned char key, int x, int y);
void display();

// Texturas
GLuint cargarTextura(const char* archivo);

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    createFirstWindow();
    createSecondaryWindow();

    glutTimerFunc(16, timer, 0);
    glutDisplayFunc(display);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textura);
    glutMainLoop();
    return 0;
}

// Ventanas
void display() {
    glutSetWindow(1);
    renderWindow1();

    glutSetWindow(2);
    renderWindow2();
}

void keyboard(unsigned char key, int x, int y) {
    float newX = pirate.x;
    float newY = pirate.y;

    int moved = 0;

    switch (key) {
    case 'w':
    case 'W': // Mover hacia arriba
        newY += pirate.speed;
        pirate.direction = 0;
        moved = 1;
        break;
    case 's':
    case 'S': // Mover hacia abajo
        newY -= pirate.speed;
        pirate.direction = 1;
        moved = 1;
        break;
    case 'a':
    case 'A': // Mover hacia la izquierda
        newX -= pirate.speed;
        pirate.direction = 2;
        moved = 1;
        break;
    case 'd':
    case 'D': // Mover hacia la derecha
        newX += pirate.speed;
        pirate.direction = 3;
        moved = 1;
        break;

    case 'm':
    case 'M':
        adjustCameraGeneral();
        return;

    case 'n':
    case 'N': // Alternar visibilidad de cuadrículas y números de ejes
        MainGrid = !MainGrid;
        SubGrid = !SubGrid;
        AxisNumbers = !AxisNumbers;
        glutPostRedisplay();
        return;

    case 27: // Salir con la tecla Esc
        exit(0);
        break;

    default:
        break;
    }

    // Verificar colisión antes de actualizar la posición del pirata
    if (checkBorderCollision(newX, newY)) {
        pirate.x = newX;
        pirate.y = newY;
    }

    // Solicitar redibujo de la pantalla
    glutPostRedisplay();
}

// Temporizador para movimiento suave
void timer(int value) {
    if (pirate.direction != DIRECTION_NONE) {
        float newX = pirate.x;
        float newY = pirate.y;

        switch (pirate.direction) {
        case 0: // Arriba
            newY += pirate.speed;
            break;
        case 1: // Abajo
            newY -= pirate.speed;
            break;
        case 2: // Izquierda
            newX -= pirate.speed;
            break;
        case 3: // Derecha
            newX += pirate.speed;
            break;
        }

        // Chequea colisiones antes de mover
        if (checkBorderCollision(newX, newY)) {
            pirate.x = newX;
            pirate.y = newY;
        }
    }

    // Ajusta la cámara para seguir al pirata
    adjustCameraToFollowPirate();

    // Redibuja la escena
    glutPostRedisplay();

    if (pirate.direction != DIRECTION_NONE) {
        pirate.direction = DIRECTION_NONE;
    }

    // Llama al temporizador de nuevo después de 16ms (~60 FPS)
    glutTimerFunc(16, timer, 0);
}

void reshapeWindow(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
}

void createFirstWindow() {
    glutInitWindowSize(700, 700);  // Tamaño de la ventana secundaria
    glutInitWindowPosition(230, 50);  // Posición en la pantalla
    int secondaryWindow = glutCreateWindow("Conquer & Win!");
    init(); // Inicializar el juego
    glutKeyboardFunc(keyboard); // Manejar las entradas del teclado

    glutDisplayFunc(renderWindow1);
}

void init() {
    glClearColor(0.549f, 0.835f, 0.953f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 11.0, -1.0, 11.0);

    glMatrixMode(GL_MODELVIEW);

    initializeSeed(); // Inicializa la cuadrícula al inicio
}

void renderWindow1() {
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
    if (AxisNumbers) {
        drawAxisNumbers();
    }
    generatePalms();
    drawBorders();
    renderPalms();
    drawPirate();

    glutSwapBuffers();
}

void createSecondaryWindow() {
    // Establecer el tamaño y otras configuraciones
    glutInitWindowSize(300, 700);  // Tamaño de la ventana secundaria
    glutInitWindowPosition(1000, 50);  // Posición en la pantalla
    int secondaryWindow = glutCreateWindow("Panel Control");

    // Opcional: configurar otras propiedades de la ventana secundaria
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);  // Color de fondo
    glViewport(0, 0, 800, 600);  // Tamaño del viewport

    // Configura las funciones de renderizado para la ventana secundaria
    glutDisplayFunc(renderWindow2);
}

void renderWindow2() {

}

// Mapa

// Función para transformar coordenadas de pantalla a coordenadas de mundo
void screenToWorld(int screenX, int screenY, float* worldX, float* worldY) {
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    *worldX = orthoLeft + (screenX / (float)viewport[2]) * (orthoRight - orthoLeft);
    *worldY = orthoBottom + ((viewport[3] - screenY) / (float)viewport[3]) * (orthoTop - orthoBottom);
}

void drawPixel(const Cell* cell) {
    glColor3f(cell->r, cell->g, cell->b);

    glBegin(GL_QUADS);
    glVertex3f(cell->x1, cell->y1, 0.0f);
    glVertex3f(cell->x2, cell->y2, 0.0f);
    glVertex3f(cell->x3, cell->y3, 0.0f);
    glVertex3f(cell->x4, cell->y4, 0.0f);
    glEnd();
}

void fillGrid() {
    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            drawPixel(&grid[i][j]);
        }
    }
}

void initializeSeed() {
    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            // Define un tono base para un naranja cálido
            float r = 1.0f + (rand() % 10) / 100.0f; // Base roja, variación leve
            float g = 0.8f + (rand() % 10) / 100.0f; // Base verde ajustada a 203/255
            float b = 0.6f + (rand() % 10) / 100.0f; // Base azul ajustada a 150/255

            // Asegura que los valores no excedan 1.0
            r = (r > 1.0f) ? 1.0f : r;
            g = (g > 1.0f) ? 1.0f : g;
            b = (b > 1.0f) ? 1.0f : b;

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

void drawBorders() {
    glColor3f(1.0f, 0.796f, 0.588f);
    glLineWidth(10.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(0, 0);
    glVertex2f(10, 0);
    glVertex2f(10, 10);
    glVertex2f(0, 10);
    glEnd();
}

// Plano cartesiano

void drawText(const char* text, float x, float y) {
    glPushMatrix();
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
    }
    glPopMatrix();
}

void drawSubGrid() {
    glColor3f(0.5f, 0.5f, 0.5f); // gris claro para el subgrid
    glLineWidth(0.2f);

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


// Funcionalidades del jugador
bool checkBorderCollision(float newX, float newY) {
    // Chequear los límites del mapa (asumido de 0 a 10)
    // Teniendo en cuenta el ancho y alto del pirata
    if (!(newX >= 0.0f && newX + pirate.width <= 10.0f &&
        newY >= 0.0f && newY + pirate.height <= 10.0f)) {
        return false; // Colisión con los bordes
    }

    Range ranges[] = {
    {0.0, 1.0, 3.0, 7.0},
    {3.0, 7.0, 6.0, 7.0},
    {3.0, 7.0, 9.0, 10.0},
    {3.8, 6.2, 8.5, 9.0},
    {5.5, 10.0, 0.0, 1.0},
    {6.5, 10.0, 1.0, 2.0},
    {8.0, 10.0, 2.0, 3.0},
    {9.0, 10.0, 3.0, 4.0},
    {9.0, 10.0, 4.0, 5.0},
    {6.0, 7.0, 5.0, 6.0},
    {3.0, 4.5, 4.0, 4.8},
    {4.0, 5.0, 3.0, 4.0}
    };

    // Verifica colisión con cada rango definido en 'ranges'
    int numRanges = sizeof(ranges) / sizeof(Range);
    for (int i = 0; i < numRanges; i++) {
        Range range = ranges[i];
        if (!(newX + pirate.width <= range.xMin || newX >= range.xMax ||
            newY + pirate.height <= range.yMin || newY >= range.yMax)) {
            return false; // Colisión con un rango
        }
    }

    return true; // No hay colisión
}

void adjustCameraGeneral() {
    float viewSize = 7.0f;

    orthoLeft = -0.5f;
    orthoRight = 11.5f;
    orthoBottom = -0.5f;
    orthoTop = 11.5f;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(orthoLeft, orthoRight, orthoBottom, orthoTop);
}

void adjustCameraToFollowPirate() {
    // Área de visión deseada alrededor del pirata
    float viewSize = 7.0f;

    // Determina los límites de la cámara basados en la posición del pirata
    float targetLeft = pirate.x - viewSize / 2;
    float targetRight = pirate.x + viewSize / 2;
    float targetBottom = pirate.y - viewSize / 2;
    float targetTop = pirate.y + viewSize / 2;

    // Interpolación para un movimiento más suave de la cámara
    orthoLeft += (targetLeft - orthoLeft) * 0.1f;
    orthoRight += (targetRight - orthoRight) * 0.1f;
    orthoBottom += (targetBottom - orthoBottom) * 0.1f;
    orthoTop += (targetTop - orthoTop) * 0.1f;

    // Actualiza la proyección
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(orthoLeft, orthoRight, orthoBottom, orthoTop);
}

// Dibujado de objetos

// Función para agregar un píxel (adaptada para dibujar pequeñas figuras)
void addPixel(float x, float y, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x - 0.05f, y - 0.05f);
    glVertex2f(x + 0.05f, y - 0.05f);
    glVertex2f(x + 0.05f, y + 0.05f);
    glVertex2f(x - 0.05f, y + 0.05f);
    glEnd();
}

// Función para generar una variación aleatoria en el color
float randomVariation(float baseValue, float variation) {
    return baseValue + ((rand() % 100) / 100.0f * variation - variation / 2.0f);
}

// Función para crear una palmera con variación de colores
void createPalm(float x, float y, Palm* palm) {
    palm->pixelCount = 0;

    // Tronco (marrón claro) con variación
    float trunk_r = 0.6f, trunk_g = 0.4f, trunk_b = 0.2f;
    for (int i = 0; i < 5; ++i) { // 5 píxeles de alto
        float trunk_r_variation = randomVariation(trunk_r, 0.1f);
        float trunk_g_variation = randomVariation(trunk_g, 0.1f);
        float trunk_b_variation = randomVariation(trunk_b, 0.1f);

        palm->pixels[palm->pixelCount++] = (PalmPixel){
            .x = x,
            .y = y + i * 0.1f,
            .r = trunk_r_variation,
            .g = trunk_g_variation,
            .b = trunk_b_variation
        };
    }

    // Hojas (verde) con variación
    float leaf_r = 0.1f, leaf_g = 0.6f, leaf_b = 0.1f;
    float leaf_offset = 0.08f;

    PalmPixel leaves[] = {
        {x, y + 0.5f, randomVariation(leaf_r, 0.1f), randomVariation(leaf_g, 0.1f), randomVariation(leaf_b, 0.1f)},                  // Hoja central
        {x + leaf_offset, y + 0.5f - leaf_offset, randomVariation(leaf_r, 0.1f), randomVariation(leaf_g, 0.1f), randomVariation(leaf_b, 0.1f)}, // Hoja derecha
        {x - leaf_offset, y + 0.5f - leaf_offset, randomVariation(leaf_r, 0.1f), randomVariation(leaf_g, 0.1f), randomVariation(leaf_b, 0.1f)}, // Hoja izquierda
        {x + 2 * leaf_offset, y + 0.5f - 2 * leaf_offset, randomVariation(leaf_r, 0.1f), randomVariation(leaf_g, 0.1f), randomVariation(leaf_b, 0.1f)}, // Hoja derecha extendida
        {x - 2 * leaf_offset, y + 0.5f - 2 * leaf_offset, randomVariation(leaf_r, 0.1f), randomVariation(leaf_g, 0.1f), randomVariation(leaf_b, 0.1f)}  // Hoja izquierda extendida
    };

    for (int i = 0; i < 5; i++) {
        palm->pixels[palm->pixelCount++] = leaves[i];
    }
}

void generatePalms() {
    if (palmsGenerated) return;
    srand((unsigned int)time(0)); // Inicializa la semilla para números aleatorios

    Range ranges[] = {
        {0.2, 1.0, 3.0, 7.0},
        {3.0, 7.0, 6.0, 7.0},
        {3.0, 7.0, 9.0, 10.0},
        {4.0, 6.0, 8.5, 9.0},
        {5.5, 10.0, 0.0, 1.0},
        {6.5, 10.0, 1.0, 2.0},
        {8.0, 10.0, 2.0, 3.0},
        {9.0, 10.0, 3.0, 4.0},
        {9.0, 10.0, 4.0, 5.0},
        {6.0, 7.0, 5.0, 6.0},
        {3.0, 4.5, 4.0, 4.8},
        {4.0, 5.0, 3.0, 4.0}
    };

    float xStep = 0.35, yStep = 0.3;
    palmCount = 0; // Reiniciar el contador de palmeras

    for (int r = 0; r < sizeof(ranges) / sizeof(ranges[0]); r++) {
        float xMin = ranges[r].xMin, xMax = ranges[r].xMax;
        float yMin = ranges[r].yMin, yMax = ranges[r].yMax;

        for (float i = xMin; i <= xMax; i += xStep) {
            for (float j = yMin; j <= yMax; j += yStep) {
                if (palmCount >= MAX_PALMS) return;

                // Generar un pequeño ajuste aleatorio para las coordenadas
                float xOffset = ((float)rand() / RAND_MAX - 0.5f) * xStep; // Variación de -xStep/2 a +xStep/2
                float yOffset = ((float)rand() / RAND_MAX - 0.5f) * yStep; // Variación de -yStep/2 a +yStep/2

                Palm* palm = &palmPositions[palmCount++];
                createPalm(i + xOffset, j + yOffset, palm);
            }
        }
    }
    palmsGenerated = true;
}

void renderPalms() {
    for (int i = 0; i < palmCount; i++) {
        Palm* palm = &palmPositions[i];
        for (int j = 0; j < palm->pixelCount; j++) {
            PalmPixel* pixel = &palm->pixels[j];
            addPixel(pixel->x, pixel->y, pixel->r, pixel->g, pixel->b);
        }
    }
}

void drawPirate() {
    // *** Botas ***
    glColor3f(0.1f, 0.1f, 0.1f); // Negro oscuro
    glBegin(GL_QUADS);
    glVertex3f(pirate.x, pirate.y - 0.05f, 0.0f);
    glVertex3f(pirate.x + 0.1f, pirate.y - 0.05f, 0.0f);
    glVertex3f(pirate.x + 0.1f, pirate.y, 0.0f);
    glVertex3f(pirate.x, pirate.y, 0.0f);

    glVertex3f(pirate.x + 0.15f, pirate.y - 0.05f, 0.0f);
    glVertex3f(pirate.x + 0.25f, pirate.y - 0.05f, 0.0f);
    glVertex3f(pirate.x + 0.25f, pirate.y, 0.0f);
    glVertex3f(pirate.x + 0.15f, pirate.y, 0.0f);
    glEnd();

    // *** Pantalones ***
    glColor3f(0.1f, 0.2f, 0.5f); // Azul oscuro
    glBegin(GL_QUADS);
    glVertex3f(pirate.x, pirate.y, 0.0f);
    glVertex3f(pirate.x + 0.25f, pirate.y, 0.0f);
    glVertex3f(pirate.x + 0.25f, pirate.y + 0.2f, 0.0f);
    glVertex3f(pirate.x, pirate.y + 0.2f, 0.0f);
    glEnd();

    // *** Cinturón ***
    glColor3f(0.3f, 0.2f, 0.1f); // Marrón
    glBegin(GL_QUADS);
    glVertex3f(pirate.x, pirate.y + 0.2f, 0.0f);
    glVertex3f(pirate.x + 0.25f, pirate.y + 0.2f, 0.0f);
    glVertex3f(pirate.x + 0.25f, pirate.y + 0.25f, 0.0f);
    glVertex3f(pirate.x, pirate.y + 0.25f, 0.0f);
    glEnd();

    // Hebilla dorada
    glColor3f(1.0f, 0.8f, 0.2f);
    glBegin(GL_QUADS);
    glVertex3f(pirate.x + 0.1f, pirate.y + 0.2f, 0.0f);
    glVertex3f(pirate.x + 0.15f, pirate.y + 0.2f, 0.0f);
    glVertex3f(pirate.x + 0.15f, pirate.y + 0.25f, 0.0f);
    glVertex3f(pirate.x + 0.1f, pirate.y + 0.25f, 0.0f);
    glEnd();

    // *** Camisa (Chaleco rojo) ***
    glColor3f(0.8f, 0.0f, 0.0f); // Rojo intenso
    glBegin(GL_QUADS);
    glVertex3f(pirate.x, pirate.y + 0.25f, 0.0f);
    glVertex3f(pirate.x + 0.25f, pirate.y + 0.25f, 0.0f);
    glVertex3f(pirate.x + 0.25f, pirate.y + 0.45f, 0.0f);
    glVertex3f(pirate.x, pirate.y + 0.45f, 0.0f);
    glEnd();

    // *** Cabeza ***
    glColor3f(0.9f, 0.7f, 0.5f); // Piel clara
    glBegin(GL_QUADS);
    glVertex3f(pirate.x + 0.05f, pirate.y + 0.45f, 0.0f);
    glVertex3f(pirate.x + 0.2f, pirate.y + 0.45f, 0.0f);
    glVertex3f(pirate.x + 0.2f, pirate.y + 0.6f, 0.0f);
    glVertex3f(pirate.x + 0.05f, pirate.y + 0.6f, 0.0f);
    glEnd();

    // *** Sombrero ***
    glColor3f(0.1f, 0.1f, 0.1f); // Negro
    glBegin(GL_TRIANGLES);
    glVertex3f(pirate.x, pirate.y + 0.6f, 0.0f);
    glVertex3f(pirate.x + 0.25f, pirate.y + 0.6f, 0.0f);
    glVertex3f(pirate.x + 0.125f, pirate.y + 0.7f, 0.0f);
    glEnd();

    // *** Ojo y Parche ***
    glColor3f(0.0f, 0.0f, 0.0f); // Negro para el parche
    glPointSize(5.0f);
    glBegin(GL_POINTS);
    glVertex3f(pirate.x + 0.08f, pirate.y + 0.55f, 0.0f); // Ojo derecho
    glEnd();

    // Línea del parche
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex3f(pirate.x + 0.05f, pirate.y + 0.58f, 0.0f);
    glVertex3f(pirate.x + 0.2f, pirate.y + 0.52f, 0.0f);
    glEnd();

    // *** Espada (Colgando al costado) ***
    glColor3f(0.5f, 0.5f, 0.5f); // Gris metálico
    glBegin(GL_QUADS);
    glVertex3f(pirate.x + 0.3f, pirate.y + 0.2f, 0.0f);
    glVertex3f(pirate.x + 0.35f, pirate.y + 0.2f, 0.0f);
    glVertex3f(pirate.x + 0.35f, pirate.y + 0.5f, 0.0f);
    glVertex3f(pirate.x + 0.3f, pirate.y + 0.5f, 0.0f);
    glEnd();
}

void printPalmPositions() {
    printf("Palm Positions:\n");
    for (int i = 0; i < palmCount; i++) {
        printf("Palm %d: (%.2f, %.2f) | Width: %.2f | Height: %.2f\n",
            i + 1,
            palmPositions[i].x,
            palmPositions[i].y,
            palmPositions[i].width,
            palmPositions[i].height);
    }
}