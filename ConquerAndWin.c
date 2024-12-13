#include <GL/glut.h>
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define GRID_SIZE 300
#define MAX_PALMS 1000
#define DIRECTION_NONE -1

int selectedOption = 0;  // 0: Nuevo Juego, 1: Cargar Juego, 2: Salir
int selectedOption3 = 0;   // Opción seleccionada en la ventana "Juego Terminado" 
int gameStarted = 0;  // Variable para determinar si el juego ha comenzado
int window0, window1, window2, window3;
bool gameStateChanged = false;

int timerStarted = 0;
int remainingTime = 30;
int lastTimerUpdate = 0;

float zoomLevel = 1.0f; // Nivel de zoom (1.0 es el predeterminado)
float orthoLeft = -1.0f, orthoRight = 11.0f, orthoBottom = -1.0f, orthoTop = 11.0f;

bool MainGrid = false;  // Controla el dibujo del grid principal
bool SubGrid = false;  // Controla el dibujo del subgrid
bool AxisNumbers = false;

bool palmsGenerated = false;

float treasureChestX = -1.0f, treasureChestY = -1.0f; // Posición inicial de carga (Inválida)
bool treasureChestGenerated = false;  // Bandera para creación de Cofre
bool showChest = false; // Bandera para indicar si se muestra el cofre
int totalChestInMission = 10;

float grapeInitX = -1.0f, grapeInitY = -1.0f;
bool grapeGenerated = false;
bool showGrape = false;
int totalGrapeInMission = 4;

float necklaceX = -1.0f, necklaceY = -1.0f;
bool necklaceGenerated = false;
bool showNecklace = false;
int totalNecklaceInMission = 2;

typedef struct {
    float r, g, b;
    float x1, y1, x2, y2, x3, y3, x4, y4;
} Cell;

Cell grid[GRID_SIZE][GRID_SIZE]; // Matriz para almacenar la información de la cuadrícula

typedef struct {
    float xMin, xMax, yMin, yMax;
} Range;

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

typedef enum {
    ITEM_CHEST,
    ITEM_GRAPE,
    ITEM_NECKLACE
} ItemType;
struct elemento
{
    int id;
    int count;
    ItemType type;
};
typedef struct nodo
{
    struct elemento elem;
    struct nodo* sgt;
} Nodo;

Nodo* Inventory = NULL;
int c = 1; // Contador para establecer ID dentro de Lista

// Estructura para representar una misión
typedef struct {
    char nombre[50];
    char descripcion[200];
} Mision;

// Estructura de la pila
typedef struct {
    Mision* misiones;  // Puntero a un arreglo de misiones
    int tope;          // Índice del último elemento en la pila
    int capacidad;     // Capacidad máxima de la pila
} Pila;

Pila pilaMisiones;

// Misiones
void inicializarPila(Pila* pila, int capacidadInicial);
int estaVacia(Pila* pila);
void expandirPila(Pila* pila);
void agregarMision(Pila* pila, Mision mision);
Mision eliminarMision(Pila* pila);
Mision verMisionSuperior(Pila* pila);
void liberarPila(Pila* pila);

// Inventario
Nodo* crearNodo(int count);
void insertarNodo(Nodo** Lista, int id, ItemType type);

// Dibujado de objetos
void generateGoldNecklace(float* necklaceX, float* necklaceY);
void drawGoldNecklace(float necklaceX, float necklaceY);

void generateGrapes(float* grapeX, float* grapeY);
void drawGrapes(float grapeX, float grapeY);

void generateTreasureChest(float* chestX, float* chestY);
void drawTreasureChest(float chestX, float chestY);

void drawPirate();

void addPixel(float x, float y, float r, float g, float b);
float randomVariation(float baseValue, float variation);
void createPalm(float x, float y, Palm* palm);
void generatePalms();
void renderPalms();
void printPalmPositions();


// Colisiones de objetos
bool checkNecklaceCollision(float pirateX, float pirateY);
bool checkGrapeCollision(float pirateX, float pirateY);
bool checkChestCollision(float pirateX, float pirateY);

// Funcionalidades del jugador
int isInRestrictedRange(float x, float y);
bool checkBorderCollision(float newX, float newY);
void adjustCameraToFollowPirate();
void adjustCameraGeneral();

// Mapa
void fillGrid();

// Plano cartesiano
void drawText(const char* text, float x, float y);
void drawSubGrid();
void drawGrid();
void drawAxisNumbers();

// Minimapa, inventario y misiones en Ventana 2
void drawSubGrid2();
void drawGrid2();
void drawMiniMap();
void drawInventory();
void drawMissions();
void drawItemWithCount(float x, float y, const char* itemName, int count);

// Ventanas
void reshapeWindow(int w, int h);

// Menú principal
void drawMenu();
void menuKeyboard(unsigned char key, int x, int y);
void renderText(const char* text, float x, float y, void* font);
void drawTitle(const char* title, float x, float y);
void drawPirateBackground();
void drawSelectionIndicator(float x, float y);
void drawMenu();
void timer0(int value);
void initMenuWindow();
void createMenuWindow();

// Ventana 1 (Juego)
void timer1(int value);
void renderWindow1();
void createWindow1();
void initWindow1();
void keyboard(unsigned char key, int x, int y);

// Ventana 2 (Minimapa y misiones)
void timer2(int value);
void renderWindow2();
void createWindow2();

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    inicializarPila(&pilaMisiones, 3);
    Mision m1 = { "Mision 1", "Agarra 4 uvas" };
    Mision m3 = { "Mision 3", "Consigue 15 cofres" };
    Mision m2 = { "Mision 2", "Encuentra 2 collares" };

    agregarMision(&pilaMisiones, m3);
    agregarMision(&pilaMisiones, m2);
    agregarMision(&pilaMisiones, m1);

    createMenuWindow();  // Crear la ventana del menú
    glutTimerFunc(16, timer0, 0);
    glutMainLoop();  // Bucle principal de GLUT
    return 0;
}

// Función para dibujar la ventana "Juego Terminado"
void drawGameOverWindow() {
    glClearColor(0.63f, 0.32f, 0.18f, 1.0f);  // Color base de madera (#804d26)
    glClear(GL_COLOR_BUFFER_BIT);

    drawPirateBackground();  // Fondo similar al menú principal
    drawTitle("¡Juego Terminado!", -0.3f, 0.5f);

    const char* mainMenuText = "Volver al Menu Principal";
    const char* saveText = "Guardar";
    const char* exitText = "Salir";

    glColor3f(0.8f, 0.7f, 0.5f);

    // Coordenadas centralizadas para las opciones
    float menuCenterX = -0.3f;
    float menuStartY = 0.2f;
    float menuSpacing = -0.15f;

    renderText(mainMenuText, menuCenterX, menuStartY, GLUT_BITMAP_HELVETICA_18);
    renderText(saveText, menuCenterX, menuStartY + menuSpacing, GLUT_BITMAP_HELVETICA_18);
    renderText(exitText, menuCenterX, menuStartY + 2 * menuSpacing, GLUT_BITMAP_HELVETICA_18);

    if (selectedOption3 == 0) {
        drawSelectionIndicator(menuCenterX - 0.1f, menuStartY);
        glColor3f(1.0f, 1.0f, 0.8f);
        renderText(mainMenuText, menuCenterX, menuStartY, GLUT_BITMAP_HELVETICA_18);
    }
    else if (selectedOption3 == 1) {
        drawSelectionIndicator(menuCenterX - 0.1f, menuStartY + menuSpacing);
        glColor3f(1.0f, 1.0f, 0.8f);
        renderText(saveText, menuCenterX, menuStartY + menuSpacing, GLUT_BITMAP_HELVETICA_18);
    }
    else if (selectedOption3 == 2) {
        drawSelectionIndicator(menuCenterX - 0.1f, menuStartY + 2 * menuSpacing);
        glColor3f(1.0f, 1.0f, 0.8f);
        renderText(exitText, menuCenterX, menuStartY + 2 * menuSpacing, GLUT_BITMAP_HELVETICA_18);
    }
    glutSwapBuffers();
}

// Función de teclado para manejar las opciones en la ventana "Juego Terminado"
void gameOverKeyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'w':
    case 'W':
        if (selectedOption3 > 0) {
            selectedOption3--;
        }
        glutPostRedisplay();
        break;

    case 's':
    case 'S':
        if (selectedOption3 < 2) {
            selectedOption3++;
        }
        glutPostRedisplay();
        break;

    case 13:  // Enter
        if (selectedOption3 == 0) {  // Volver al Menú Principal
            glutDestroyWindow(window3);  // Cerrar la ventana de "Juego Terminado"
            createMenuWindow();  // Crear la ventana del menú principal
            timerStarted = 0;
            remainingTime = 30;
            lastTimerUpdate = 0;
        }
        else if (selectedOption3 == 1) {  // Guardar
            printf("Juego Guardado... (Funcionalidad pendiente)\n");
            // Lógica para guardar el estado del juego
        }
        else if (selectedOption3 == 2) {  // Salir
            exit(0);  // Salir del programa
        }
        break;

    default:
        break;
    }
}

// Función para inicializar la ventana "Juego Terminado"
void initGameOverWindow() {
    glClearColor(0.0, 0.0, 0.0, 1.0);  // Fondo negro
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
}

// Función para crear la ventana "Juego Terminado"
void createGameOverWindow() {
    glutInitWindowSize(700, 700);
    glutInitWindowPosition(230, 50);
    window3 = glutCreateWindow("Juego Terminado");
    initGameOverWindow();
    glutKeyboardFunc(gameOverKeyboard);
    glutDisplayFunc(drawGameOverWindow);
}

void timer0(int value) {
    glutPostRedisplay();
    glutTimerFunc(16, timer0, 0);
}

void renderText(const char* text, float x, float y, void* font) {
    glRasterPos2f(x, y);
    for (int i = 0; i < strlen(text); i++) {
        glutBitmapCharacter(font, text[i]);
    }
}

// Dibujar un título
void drawTitle(const char* title, float x, float y) {
    glColor3f(0.2f, 0.1f, 0.0f);  // Sombra más oscura
    renderText(title, x + 0.01f, y - 0.01f, GLUT_BITMAP_TIMES_ROMAN_24);

    glColor3f(0.5f, 0.3f, 0.15f);  // Color de madera envejecida
    renderText(title, x, y, GLUT_BITMAP_TIMES_ROMAN_24);
}

void drawPirateBackground() {
    glBegin(GL_QUADS);
    // Degradado
    glColor3f(0.63f, 0.32f, 0.18f);
    glVertex2f(-1.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);

    glColor3f(0.5f, 0.3f, 0.15f);  // Tono más oscuro
    glVertex2f(1.0f, -1.0f);
    glVertex2f(-1.0f, -1.0f);
    glEnd();
}

// Dibujar indicador de selección
void drawSelectionIndicator(float x, float y) {
    glColor3f(0.8f, 0.4f, 0.1f);

    // Forma de ancla más elaborada
    glBegin(GL_TRIANGLES);
    // Punta del ancla
    glVertex2f(x, y);

    // Base del ancla
    glVertex2f(x - 0.04f, y + 0.05f);
    glVertex2f(x + 0.04f, y + 0.05f);

    // Línea horizontal del ancla
    glVertex2f(x - 0.02f, y + 0.05f);
    glVertex2f(x + 0.02f, y + 0.05f);
    glVertex2f(x, y + 0.07f);
    glEnd();
}

void drawMenu() {
    glClearColor(0.63f, 0.32f, 0.18f, 1.0f);  // Color base de madera (#804d26)
    glClear(GL_COLOR_BUFFER_BIT);

    drawPirateBackground();
    drawTitle("CONQUER & WIN!", -0.3f, 0.5f);

    const char* newGameText = "Nueva Aventura";
    const char* loadGameText = "Cargar Juego";
    const char* exitText = "Abandonar Barco!";

    glColor3f(0.8f, 0.7f, 0.5f);

    // Coordenadas centralizadas para las opciones de menú
    float menuCenterX = -0.2f;
    float menuStartY = 0.2f;
    float menuSpacing = -0.15f;

    renderText(newGameText, menuCenterX, menuStartY, GLUT_BITMAP_HELVETICA_18);
    renderText(loadGameText, menuCenterX, menuStartY + menuSpacing, GLUT_BITMAP_HELVETICA_18);
    renderText(exitText, menuCenterX, menuStartY + 2 * menuSpacing, GLUT_BITMAP_HELVETICA_18);

    if (selectedOption == 0) {
        drawSelectionIndicator(menuCenterX - 0.1f, menuStartY);
        glColor3f(1.0f, 1.0f, 0.8f);
        renderText(newGameText, menuCenterX, menuStartY, GLUT_BITMAP_HELVETICA_18);
    }
    else if (selectedOption == 1) {
        drawSelectionIndicator(menuCenterX - 0.1f, menuStartY + menuSpacing);
        glColor3f(1.0f, 1.0f, 0.8f);
        renderText(loadGameText, menuCenterX, menuStartY + menuSpacing, GLUT_BITMAP_HELVETICA_18);
    }
    else if (selectedOption == 2) {
        drawSelectionIndicator(menuCenterX - 0.1f, menuStartY + 2 * menuSpacing);
        glColor3f(1.0f, 1.0f, 0.8f);
        renderText(exitText, menuCenterX, menuStartY + 2 * menuSpacing, GLUT_BITMAP_HELVETICA_18);
    }
    glutSwapBuffers();
}

void menuKeyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'w':
    case 'W':
        if (selectedOption > 0) {
            selectedOption--;
        }
        glutPostRedisplay();
        break;

    case 's':  // Mover hacia abajo
    case 'S':
        if (selectedOption < 2) {
            selectedOption++;
        }
        glutPostRedisplay();
        break;

    case 13:  // Enter
        if (selectedOption == 0) {  // Nuevo Juego
            gameStarted = 1;
            glutDestroyWindow(window0);  // Cerrar la ventana del menú
            createWindow1();
            createWindow2();
        }
        else if (selectedOption == 1) {  // Cargar Juego
            printf("Cargar juego... (Funcionalidad pendiente)\n");
        }
        else if (selectedOption == 2) {  // Salir
            exit(0);
        }
        break;

    default:
        break;
    }
}

void initMenuWindow() {
    glClearColor(0.0, 0.0, 0.0, 1.0);  // Fondo negro
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
}

void createMenuWindow() {
    glutInitWindowSize(700, 700);
    glutInitWindowPosition(230, 50);
    window0 = glutCreateWindow("Conquer & Win!");
    initMenuWindow();
    glutKeyboardFunc(menuKeyboard);
    glutDisplayFunc(drawMenu);
}

// Inventario 
Nodo* buscarNodo(Nodo* Lista, int id) {
    while (Lista != NULL) {
        if (Lista->elem.id == id) {
            return Lista; // Nodo encontrado
        }
        Lista = Lista->sgt;
    }
    return NULL; // Nodo no encontrado
}

Nodo* crearNodo(int count)
{
    Nodo* nodo = (Nodo*)malloc(sizeof(Nodo));
    if (!nodo)
        return NULL;
    nodo->elem.id = count;
    nodo->elem.count = 1;
    nodo->sgt = NULL;
    return nodo;
}

void insertarNodo(Nodo** Lista, int id, ItemType type) {
    // Verificar si ya existe un nodo con el mismo tipo
    Nodo* current = *Lista;

    while (current) {
        if (current->elem.type == type) {
            // Actualizar el contador y banderas según el tipo
            current->elem.count++;
            if (type == ITEM_CHEST) {
                showChest = true;
            }
            else if (type == ITEM_GRAPE) {
                showGrape = true;
            }
            else if (type == ITEM_NECKLACE) {
                showNecklace = true;
            }
            return; // Salir de la función
        }
        current = current->sgt; // Mover al siguiente nodo
    }

    // Si no existe un nodo con el mismo tipo, crear uno nuevo
    Nodo* newNodo = (Nodo*)malloc(sizeof(Nodo));
    if (!newNodo) {
        fprintf(stderr, "Error: No se pudo asignar memoria para el nodo.\n");
        return; // Manejo de error
    }

    // Inicializar el nuevo nodo
    newNodo->elem.id = id;
    newNodo->elem.count = 1;
    newNodo->elem.type = type;
    newNodo->sgt = *Lista; // Enlazar al inicio de la lista
    *Lista = newNodo;      // Actualizar el puntero de la lista

    // Actualizar banderas según el tipo del nuevo nodo
    if (type == ITEM_CHEST) {
        showChest = true;
    }
    else if (type == ITEM_GRAPE) {
        showGrape = true;
    }
    else if (type == ITEM_NECKLACE) {
        showNecklace = true;
    }
}

// Ventanas
void reshapeWindow(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
}

void keyboard(unsigned char key, int x, int y) {
    if (!timerStarted && (key == 'w' || key == 'W' || key == 's' || key == 'S' ||
        key == 'a' || key == 'A' || key == 'd' || key == 'D')) {
        timerStarted = 1;
        lastTimerUpdate = glutGet(GLUT_ELAPSED_TIME);
    }

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

// Ventana 1
void updateGameTimer(int value) {
    if (timerStarted) {
        int currentTime = glutGet(GLUT_ELAPSED_TIME);
        int elapsedSeconds = (currentTime - lastTimerUpdate) / 1000;

        remainingTime -= elapsedSeconds;
        lastTimerUpdate = currentTime;

        if (remainingTime <= 0) {
            // Time is up, create Game Over window
            glutDestroyWindow(window1);
            createGameOverWindow();
            glutDisplayFunc(drawGameOverWindow);
            timerStarted = 0;
            return;
        }
    }

    // Continue the timer
    glutTimerFunc(1000, updateGameTimer, 0);
}

void timer1(int value) {
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

        // Comprobar colisiones con los bordes
        if (checkBorderCollision(newX, newY)) {
            if (newX != pirate.x || newY != pirate.y) {
                pirate.x = newX;
                pirate.y = newY;
                gameStateChanged = true;
            }
        }

        if (checkChestCollision(pirate.x, pirate.y)) {
            generateTreasureChest(&treasureChestX, &treasureChestY);
            insertarNodo(&Inventory, c, ITEM_CHEST); // Inserta un cofre
            if (totalGrapeInMission == 0 && totalNecklaceInMission == 0) {
                --totalChestInMission;
                if (totalChestInMission == 0) {
                    eliminarMision(&pilaMisiones);
                }
            }
        }

        if (checkGrapeCollision(pirate.x, pirate.y)) {
            generateGrapes(&grapeInitX, &grapeInitY);
            insertarNodo(&Inventory, c, ITEM_GRAPE); // Inserta uvas
            --totalGrapeInMission;
            if (totalGrapeInMission == 0) {
                eliminarMision(&pilaMisiones);
            }
        }

        if (checkNecklaceCollision(pirate.x, pirate.y)) {
            generateGoldNecklace(&necklaceX, &necklaceY);
            insertarNodo(&Inventory, c, ITEM_NECKLACE);
            if (totalGrapeInMission == 0) {
                --totalNecklaceInMission;
                if (totalNecklaceInMission == 0) {
                    eliminarMision(&pilaMisiones);
                }
            }
        }

    }

    if (gameStateChanged) {
        glutSetWindow(window1);
        adjustCameraToFollowPirate();
        glutPostRedisplay();  // Redibujar solo la ventana activa
        gameStateChanged = false;
    }

    if (pirate.direction != DIRECTION_NONE) {
        pirate.direction = DIRECTION_NONE;
    }

    // Llamar al temporizador de nuevo después de 16ms (~60 FPS)
    glutTimerFunc(16, timer1, 0);
}

void renderWindow1() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    fillGrid();
    renderPalms();

    if (SubGrid) {
        drawSubGrid();
    }
    if (MainGrid) {
        drawGrid();
    }
    if (AxisNumbers) {
        drawAxisNumbers();
    }

    drawPirate();

    if (treasureChestGenerated) {
        drawTreasureChest(treasureChestX, treasureChestY);
    }

    drawGrapes(grapeInitX, grapeInitY);
    drawGoldNecklace(necklaceX, necklaceY);

    if (timerStarted) {
        char timeStr[20];
        sprintf(timeStr, "Time: %d:%02d", remainingTime / 60, remainingTime % 60);

        glColor3f(1.0f, 1.0f, 1.0f);  // White color
        glRasterPos2f(-0.5f, 1.0f);  // Position at top left
        for (int i = 0; timeStr[i] != '\0'; i++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, timeStr[i]);
        }
    }

    glutSwapBuffers();
}

void initWindow1() {
    glClearColor(0.549f, 0.835f, 0.953f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 11.0, -1.0, 11.0);

    glMatrixMode(GL_MODELVIEW);

    generatePalms();
    generateGrapes(&grapeInitX, &grapeInitY);
    generateTreasureChest(&treasureChestX, &treasureChestY);
    generateGoldNecklace(&necklaceX, &necklaceY);
    treasureChestGenerated = true;
}

void createWindow1() {
    glutInitWindowSize(700, 700);  // Tamaño de la ventana secundaria
    glutInitWindowPosition(230, 50);  // Posición en la pantalla
    window1 = glutCreateWindow("Conquer & Win!");
    initWindow1(); // Inicializar el juego
    glutKeyboardFunc(keyboard); // Manejar las entradas del teclado

    // Add this line to start the timer update
    glutTimerFunc(1000, updateGameTimer, 0);

    glutTimerFunc(16, timer1, 0);
    glutDisplayFunc(renderWindow1);
}


// Ventana 2
void timer2(int value) {
    static int frameCount = 0;
    frameCount++;

    // Reducir la frecuencia de actualización del minimapa
    if (frameCount % 3 == 0) {  // Actualizar cada 3 frames
        glutSetWindow(window2);
        glutPostRedisplay();  // Redibuja solo la ventana del minimapa
    }

    glutTimerFunc(100, timer2, 0);  // Mantener la llamada recursiva
}


void renderWindow2() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Dibuja los elementos de la ventana
    drawGrid2();      // Dibuja la cuadrícula del minimapa
    drawMiniMap();    // Dibuja el minimapa con el jugador
    drawInventory();  // Dibuja el inventario
    drawMissions();   // Dibuja las misiones

    // Dibuja los elementos específicos si están activos
    if (showChest) {
        // Busca el nodo del cofre en el inventario
        Nodo* chestNode = Inventory;
        while (chestNode && chestNode->elem.type != ITEM_CHEST) {
            chestNode = chestNode->sgt;
        }
        if (chestNode) {
            drawItemWithCount(1.4f, 7.2f, "Chest", chestNode->elem.count);
        }
    }

    if (showGrape) {
        // Busca el nodo de las uvas en el inventario
        Nodo* grapeNode = Inventory;
        while (grapeNode && grapeNode->elem.type != ITEM_GRAPE) {
            grapeNode = grapeNode->sgt;
        }
        if (grapeNode) {
            drawItemWithCount(2.8f, 7.2f, "Grape", grapeNode->elem.count);
        }
    }

    if (showNecklace) {
        Nodo* necklaceNode = Inventory;
        while (necklaceNode && necklaceNode->elem.type != ITEM_NECKLACE) {
            necklaceNode = necklaceNode->sgt;
        }
        if (necklaceNode) {
            drawItemWithCount(4.5f, 7.2f, "Necklace", necklaceNode->elem.count);
        }
    }
    // Intercambia los buffers para mostrar la ventana
    glutSwapBuffers();
}

void initWindow2() {
    glClearColor(0.5f, 0.3f, 0.15f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, 6.0, 0.0, 14.0);
    glMatrixMode(GL_MODELVIEW);

}

void createWindow2() {
    glutInitWindowSize(300, 700);
    glutInitWindowPosition(1000, 50);
    window2 = glutCreateWindow("Map and Quest");

    initWindow2();
    glutTimerFunc(100, timer2, 0);
    glutDisplayFunc(renderWindow2);

}

// Ventana 2 - Minimapa, Inventario, Misiones
void drawItemWithCount(float x, float y, const char* itemName, int count) {
    // Dibuja el objeto en la posición especificada
    if (strcmp(itemName, "Chest") == 0) {
        drawTreasureChest(x, y);
    }
    else if (strcmp(itemName, "Grape") == 0) {
        drawGrapes(x, y);
    }
    else if (strcmp(itemName, "Necklace") == 0) {
        drawGoldNecklace(x, y);
    }

    // Configura el color para el texto
    glColor3f(1.0f, 1.0f, 1.0f);

    // Prepara la posición del texto (ligeramente sobre el objeto)
    glRasterPos2f(x + 0.4f, y + 0.15f);

    // Convierte el conteo a cadena
    char countText[10];  // Buffer para el texto
    sprintf(countText, "%d", count);

    // Renderiza cada carácter de la cadena
    for (char* c = countText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

// Función para dibujar las misiones
void drawMissions(Pila* pila) {
    // Variables de tamaño del minimapa
    float gridCols = 10;  // Número de columnas
    float gridRows = 23.33;  // Número de filas
    float colWidth = 6.0f / gridCols;  // Ancho de cada celda en X
    float rowHeight = 14.0f / gridRows;  // Altura de cada celda en Y

    // Definir los límites del área de las misiones
    float x1Pos = 1 * colWidth;
    float y1Pos = 1 * rowHeight;
    float x2Pos = 9 * colWidth;
    float y2Pos = 1 * rowHeight;
    float x3Pos = 9 * colWidth;
    float y3Pos = 8 * rowHeight;
    float x4Pos = 1 * colWidth;
    float y4Pos = 8 * rowHeight;

    // Dibujar el fondo del área de misiones como un pergamino
    glColor3f(0.9f, 0.8f, 0.6f);  // Color sepia claro
    glBegin(GL_QUADS);
    glVertex2f(x1Pos, y1Pos);
    glVertex2f(x2Pos, y2Pos);
    glVertex2f(x3Pos, y3Pos);
    glVertex2f(x4Pos, y4Pos);
    glEnd();

    // Agregar un borde decorativo oscuro
    glColor3f(0.6f, 0.4f, 0.2f);  // Color marrón oscuro
    glLineWidth(3.0f);  // Grosor de línea
    glBegin(GL_LINE_LOOP);
    glVertex2f(x1Pos, y1Pos);
    glVertex2f(x2Pos, y2Pos);
    glVertex2f(x3Pos, y3Pos);
    glVertex2f(x4Pos, y4Pos);
    glEnd();

    // Dibujar texto "MISSIONS" en la esquina superior izquierda del área
    glColor3f(1.0f, 1.0f, 1.0f);  // Color blanco
    glRasterPos2f(x1Pos, y4Pos + 0.1f);  // Posición del texto
    const char* text = "MISSIONS";
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);  // Renderiza cada caracter
    }

    int numMissions = pila->tope + 1;  // Número de misiones en la pila
    int numColumns = 1;

    float missionWidth = (x3Pos - x1Pos) / numColumns;  // Ancho de cada misión
    float missionHeight = (y3Pos - y1Pos) / (float)(numMissions);  // Altura de cada misión

    // Recorrer la pila de abajo hacia arriba, es decir, desde el tope hacia el fondo
    for (int i = pila->tope; i >= 0; i--) {
        int row = (pila->tope - i) / numColumns;  // Determinar la fila (de abajo hacia arriba)
        int col = (pila->tope - i) % numColumns;  // Determinar la columna (si hay más de una)

        // Calcular la posición de cada misión
        float missionX1 = x1Pos + col * missionWidth;  // Posición X de la misión
        float missionX2 = missionX1 + missionWidth;  // Posición X final de la misión
        float missionY1 = y1Pos + row * missionHeight;  // Posición Y de la misión
        float missionY2 = missionY1 + missionHeight;  // Posición Y final de la misión

        // Dibujar fondo de la misión
        if (row == 0 || row == 1) {
            glColor3f(0.95f, 0.95f, 0.95f);  // Fila 1 y 2: color #F4F4F4
        }
        else if (row == 2) {
            glColor3f(0.98f, 0.98f, 0.98f);  // Fila 3: color #F9F9F9
        }

        glBegin(GL_QUADS);
        glVertex2f(missionX1, missionY1);
        glVertex2f(missionX2, missionY1);
        glVertex2f(missionX2, missionY2);
        glVertex2f(missionX1, missionY2);
        glEnd();

        // Borde de cada misión
        glColor3f(0.6f, 0.4f, 0.2f);  // Color marrón oscuro
        glLineWidth(2.0f);  // Grosor de línea
        glBegin(GL_LINE_LOOP);
        glVertex2f(missionX1, missionY1);
        glVertex2f(missionX2, missionY1);
        glVertex2f(missionX2, missionY2);
        glVertex2f(missionX1, missionY2);
        glEnd();

        // Mostrar la descripción de la misión en la fila correspondiente
        Mision misionActual = pila->misiones[i];

        glColor3f(0.0f, 0.0f, 0.0f);  // Color negro para el texto
        glRasterPos2f(missionX1 + 0.1f, missionY1 + 0.1f);  // Posición de inicio del texto
        for (const char* c = misionActual.descripcion; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);  // Renderiza cada caracter
        }
    }
}

void drawInventory() {
    // Variables de tamaño del inventario
    float gridCols = 10;  // Número de columnas
    float gridRows = 23.33;  // Número de filas
    float colWidth = 6.0f / gridCols;  // Ancho de cada celda en X
    float rowHeight = 14.0f / gridRows;  // Altura de cada celda en Y

    // Definir los límites del inventario (en base a la cuadrícula)
    float x1Pos = 1 * colWidth;
    float y1Pos = 9 * rowHeight;
    float x2Pos = 9 * colWidth;
    float y2Pos = 9 * rowHeight;
    float x3Pos = 9 * colWidth;
    float y3Pos = 13 * rowHeight;
    float x4Pos = 1 * colWidth;
    float y4Pos = 13 * rowHeight;

    // Dibujar el fondo del inventario como un pergamino
    glColor3f(0.9f, 0.8f, 0.6f);  // Color sepia claro
    glBegin(GL_QUADS);
    glVertex2f(x1Pos, y1Pos);
    glVertex2f(x2Pos, y2Pos);
    glVertex2f(x3Pos, y3Pos);
    glVertex2f(x4Pos, y4Pos);
    glEnd();

    // Agregar un borde decorativo oscuro
    glColor3f(0.6f, 0.4f, 0.2f);  // Color marrón oscuro
    glLineWidth(3.0f);  // Grosor de línea
    glBegin(GL_LINE_LOOP);
    glVertex2f(x1Pos, y1Pos);
    glVertex2f(x2Pos, y2Pos);
    glVertex2f(x3Pos, y3Pos);
    glVertex2f(x4Pos, y4Pos);
    glEnd();

    // Dibujar texto "INVENTORY" en la esquina superior izquierda del inventario
    glColor3f(1.0f, 1.0f, 1.0f);  // Color blanco
    glRasterPos2f(x1Pos, y4Pos + 0.1f);  // Posición del texto
    const char* text = "INVENTORY";
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);  // Renderiza cada caracter
    }

    // Crear las 6 casillas dentro del inventario (2 filas y 3 casillas por fila)
    int numItems = 6;  // Número de casillas (6 casillas)
    float itemWidth = (x3Pos - x1Pos) / 3;  // Calcular el ancho de cada casilla (3 casillas por fila)
    float itemHeight = (y3Pos - y1Pos) / 2;  // La altura de cada casilla será la mitad de la altura total del inventario (2 filas)

    // Dibujar las casillas dentro del inventario (2 filas y 3 columnas)
    for (int i = 0; i < numItems; i++) {
        int row = i / 3;  // Calcular en qué fila está la casilla (0 o 1)
        int col = i % 3;  // Calcular en qué columna está la casilla (0, 1 o 2)

        // Calcular la posición de cada casilla
        float itemX1 = x1Pos + col * itemWidth;  // Posición X de la casilla
        float itemX2 = itemX1 + itemWidth;  // Posición X final de la casilla
        float itemY1 = y1Pos + row * itemHeight;  // Posición Y de la casilla
        float itemY2 = itemY1 + itemHeight;  // Posición Y final de la casilla

        // Dibujar fondo de cada casilla
        glColor3f(0.8f, 0.7f, 0.5f);  // Color de fondo de la casilla
        glBegin(GL_QUADS);
        glVertex2f(itemX1, itemY1);
        glVertex2f(itemX2, itemY1);
        glVertex2f(itemX2, itemY2);
        glVertex2f(itemX1, itemY2);
        glEnd();

        // Borde de cada casilla
        glColor3f(0.6f, 0.4f, 0.2f);  // Color marrón oscuro
        glLineWidth(2.0f);  // Grosor de línea
        glBegin(GL_LINE_LOOP);
        glVertex2f(itemX1, itemY1);
        glVertex2f(itemX2, itemY1);
        glVertex2f(itemX2, itemY2);
        glVertex2f(itemX1, itemY2);
        glEnd();
    }
}

// Función para dibujar las misiones
void drawMissions() {
    // Variables de tamaño del minimapa
    float gridCols = 10;  // Número de columnas
    float gridRows = 23.33;  // Número de filas
    float colWidth = 6.0f / gridCols;  // Ancho de cada celda en X
    float rowHeight = 14.0f / gridRows;  // Altura de cada celda en Y

    // Definir los límites del área de las misiones
    float x1Pos = 1 * colWidth;
    float y1Pos = 1 * rowHeight;
    float x2Pos = 9 * colWidth;
    float y2Pos = 1 * rowHeight;
    float x3Pos = 9 * colWidth;
    float y3Pos = 8 * rowHeight;
    float x4Pos = 1 * colWidth;
    float y4Pos = 8 * rowHeight;

    // Dibujar el fondo del área de misiones como un pergamino
    glColor3f(0.9f, 0.8f, 0.6f);  // Color sepia claro
    glBegin(GL_QUADS);
    glVertex2f(x1Pos, y1Pos);
    glVertex2f(x2Pos, y2Pos);
    glVertex2f(x3Pos, y3Pos);
    glVertex2f(x4Pos, y4Pos);
    glEnd();

    // Agregar un borde decorativo oscuro
    glColor3f(0.6f, 0.4f, 0.2f);  // Color marrón oscuro
    glLineWidth(3.0f);  // Grosor de línea
    glBegin(GL_LINE_LOOP);
    glVertex2f(x1Pos, y1Pos);
    glVertex2f(x2Pos, y2Pos);
    glVertex2f(x3Pos, y3Pos);
    glVertex2f(x4Pos, y4Pos);
    glEnd();

    // Dibujar texto "MISSIONS" en la esquina superior izquierda del área
    glColor3f(1.0f, 1.0f, 1.0f);  // Color blanco
    glRasterPos2f(x1Pos, y4Pos + 0.1f);  // Posición del texto
    const char* text = "MISSIONS";
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);  // Renderiza cada caracter
    }

    int numMissions = pilaMisiones.tope + 1;  // Número de misiones en la pila
    int numColumns = 1;

    float missionWidth = (x3Pos - x1Pos) / numColumns;  // Ancho de cada misión
    float missionHeight = (y3Pos - y1Pos) / (float)(numMissions);  // Altura de cada misión

    // Dibujar las misiones con colores personalizados para cada fila
    for (int i = 0; i < numMissions; i++) {
        int row = i / numColumns;  // Determinar la fila (0, 1, 2)
        int col = i % numColumns;  // Determinar la columna (0, 1)

        // Calcular la posición de cada misión (cambiar el cálculo de 'y')
        float missionX1 = x1Pos + col * missionWidth;  // Posición X de la misión
        float missionX2 = missionX1 + missionWidth;  // Posición X final de la misión

        // Aquí es donde calculamos la posición correcta para Y
        float missionY1 = y1Pos + (numMissions - 1 - i) * missionHeight;  // Posición Y de la misión (empieza desde abajo)
        float missionY2 = missionY1 + missionHeight;  // Posición Y final de la misión

        // Dibujar fondo de la misión
        if (row == 0 || row == 1) {
            glColor3f(0.95f, 0.95f, 0.95f);  // Fila 1 y 2: color #F4F4F4
        }
        else if (row == 2) {
            glColor3f(0.98f, 0.98f, 0.98f);  // Fila 3: color #F9F9F9
        }

        glBegin(GL_QUADS);
        glVertex2f(missionX1, missionY1);
        glVertex2f(missionX2, missionY1);
        glVertex2f(missionX2, missionY2);
        glVertex2f(missionX1, missionY2);
        glEnd();

        // Borde de cada misión
        glColor3f(0.6f, 0.4f, 0.2f);  // Color marrón oscuro
        glLineWidth(2.0f);  // Grosor de línea
        glBegin(GL_LINE_LOOP);
        glVertex2f(missionX1, missionY1);
        glVertex2f(missionX2, missionY1);
        glVertex2f(missionX2, missionY2);
        glVertex2f(missionX1, missionY2);
        glEnd();

        // Mostrar la descripción de la misión en la fila correspondiente
        Mision misionActual = pilaMisiones.misiones[i];

        glColor3f(0.0f, 0.0f, 0.0f);  // Color negro para el texto
        glRasterPos2f(missionX1 + 0.1f, missionY1 + 0.1f);  // Posición de inicio del texto
        for (const char* c = misionActual.descripcion; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);  // Renderiza cada caracter
        }
    }
}

void drawMiniMap() {
    // Variables de tamaño del minimapa
    float gridCols = 10;  // Número de columnas
    float gridRows = 23.33;  // Número de filas
    float colWidth = 6.0f / gridCols;  // Ancho de cada celda en X
    float rowHeight = 14.0f / gridRows;  // Altura de cada celda en Y

    // Definir los límites del minimapa
    float x1Pos = 1 * colWidth;
    float y1Pos = 14 * rowHeight;
    float x2Pos = 9 * colWidth;
    float y2Pos = 14 * rowHeight;
    float x3Pos = 9 * colWidth;
    float y3Pos = 22 * rowHeight;
    float x4Pos = 1 * colWidth;
    float y4Pos = 22 * rowHeight;

    // Calcular el ancho y alto del minimapa
    float miniMapWidth = x2Pos - x1Pos - 2;
    float miniMapHeight = y4Pos - y1Pos + 2;

    // Dibujar el fondo del minimapa como un pergamino
    glColor3f(0.9f, 0.8f, 0.6f);  // Color sepia claro
    glBegin(GL_QUADS);
    glVertex2f(x1Pos, y1Pos);
    glVertex2f(x2Pos, y2Pos);
    glVertex2f(x3Pos, y3Pos);
    glVertex2f(x4Pos, y4Pos);
    glEnd();

    // Agregar un borde decorativo oscuro
    glColor3f(0.6f, 0.4f, 0.2f);  // Color marrón oscuro
    glLineWidth(3.0f);  // Grosor de línea
    glBegin(GL_LINE_LOOP);
    glVertex2f(x1Pos, y1Pos);
    glVertex2f(x2Pos, y2Pos);
    glVertex2f(x3Pos, y3Pos);
    glVertex2f(x4Pos, y4Pos);
    glEnd();

    // Dibujar un punto rojo para el jugador en el minimapa
    float playerXInMiniMap = x1Pos + (pirate.x / 6.0f) * miniMapWidth;
    float playerYInMiniMap = y1Pos + (pirate.y / 14.0f) * miniMapHeight;

    // Verificar que el punto del jugador esté dentro del cuadro
    if (playerXInMiniMap < x1Pos) playerXInMiniMap = x1Pos;
    if (playerXInMiniMap > x2Pos) playerXInMiniMap = x2Pos;
    if (playerYInMiniMap < y1Pos) playerYInMiniMap = y1Pos;
    if (playerYInMiniMap > y3Pos) playerYInMiniMap = y3Pos;

    glColor3f(1.0f, 0.0f, 0.0f);  // Color rojo para el jugador
    glPointSize(8.0f);
    glBegin(GL_POINTS);
    glVertex2f(playerXInMiniMap, playerYInMiniMap);
    glEnd();

    if (treasureChestGenerated) {
        float chestXInMiniMap = x1Pos + (treasureChestX / 6.0f) * miniMapWidth;
        float chestYInMiniMap = y1Pos + (treasureChestY / 14.0f) * miniMapHeight;

        // Verificar que el punto del cofre esté dentro del cuadro
        if (chestXInMiniMap < x1Pos) chestXInMiniMap = x1Pos;
        if (chestXInMiniMap > x2Pos) chestXInMiniMap = x2Pos;
        if (chestYInMiniMap < y1Pos) chestYInMiniMap = y1Pos;
        if (chestYInMiniMap > y3Pos) chestYInMiniMap = y3Pos;

        glColor3f(1.0f, 1.0f, 0.0f);  // Color amarillo para el cofre

        // Dibujar la "X" con dos líneas cruzadas
        float size = 0.1f;  // Tamaño de la X en el minimapa

        // Línea diagonal 1 (de arriba izquierda a abajo derecha)
        glBegin(GL_LINES);
        glVertex2f(chestXInMiniMap - size, chestYInMiniMap + size);
        glVertex2f(chestXInMiniMap + size, chestYInMiniMap - size);
        glEnd();

        // Línea diagonal 2 (de arriba derecha a abajo izquierda)
        glBegin(GL_LINES);
        glVertex2f(chestXInMiniMap + size, chestYInMiniMap + size);
        glVertex2f(chestXInMiniMap - size, chestYInMiniMap - size);
        glEnd();
    }

    // Dibujar texto "MAP" en la esquina superior izquierda del minimapa
    glColor3f(1.0f, 1.0f, 1.0f);  // Color blanco
    glRasterPos2f(x1Pos, y4Pos + 0.1f);  // Posición del texto en coordenadas del minimapa
    const char* text = "MAP";
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);  // Renderiza cada caracter
    }
}

void drawSubGrid2() {
    glColor3f(1.0f, 1.0f, 1.0f); // Color blanco para el subgrid
    glBegin(GL_LINES);

    // Ajustar el tamaño de las celdas en el grid global
    float gridCols = 10;  // Número de columnas en la cuadrícula principal
    float gridRows = 23.33;  // Número de filas en la cuadrícula principal
    float colWidth = 6.0f / gridCols;  // Ancho de cada celda en X
    float rowHeight = 14.0f / gridRows;  // Altura de cada celda en Y

    // Coordenadas para el subgrid
    int subgridStartX = 1;  // Comienza en la columna 1
    int subgridStartY = 1;  // Comienza en la fila 1
    int subgridEndX = 8;   // Termina en la columna 23
    int subgridEndY = 22;    // Termina en la fila 9

    // Líneas verticales del subgrid (de 1 a 23 en X)
    for (int i = subgridStartX; i <= subgridEndX; i++) {
        float x = i * colWidth;
        glVertex2f(x, subgridStartY * rowHeight); // Punto inferior
        glVertex2f(x, (subgridEndY + 1) * rowHeight); // Punto superior
    }

    // Líneas horizontales del subgrid (de 1 a 9 en Y)
    for (int j = subgridStartY; j <= subgridEndY; j++) {
        float y = j * rowHeight;
        glVertex2f(subgridStartX * colWidth, y); // Punto izquierdo
        glVertex2f((subgridEndX + 1) * colWidth, y); // Punto derecho
    }

    // Línea blanca adicional en la parte superior del subgrid
    float topY = (subgridEndY + 1) * rowHeight;
    glVertex2f(subgridStartX * colWidth, topY); // Punto izquierdo en la parte superior
    glVertex2f((subgridEndX + 1) * colWidth, topY); // Punto derecho en la parte superior

    // Línea blanca adicional en la parte derecha del subgrid
    float rightX = (subgridEndX + 1) * colWidth;
    glVertex2f(rightX, subgridStartY * rowHeight); // Punto inferior derecho
    glVertex2f(rightX, (subgridEndY + 1) * rowHeight); // Punto superior derecho

    glEnd();
}

void drawGrid2() {
    glColor3f(0.0f, 0.0f, 0.0f); // Color de las líneas (negro)
    glBegin(GL_LINES);

    // Ajustar líneas verticales y horizontales a las proporciones
    float gridCols = 10; // Número de columnas
    float gridRows = 23.33; // Número de filas
    float colWidth = 6.0f / gridCols; // Ancho de cada celda en X
    float rowHeight = 14.0f / gridRows; // Altura de cada celda en Y

    // Líneas verticales
    for (int i = 0; i <= gridCols; i++) {
        float x = i * colWidth;
        glVertex2f(x, 0.0f); // Punto inferior
        glVertex2f(x, 14.0f); // Punto superior
    }

    // Líneas horizontales
    for (int j = 0; j <= gridRows; j++) {
        float y = j * rowHeight;
        glVertex2f(0.0f, y); // Punto izquierdo
        glVertex2f(6.0f, y); // Punto derecho
    }
    glEnd();
}

// Mapa
void fillGrid() {
    glColor3f(255.0f / 255.0f, 226.0f / 255.0f, 164.0f / 255.0f);
    // Dibuja el cuadrado
    glBegin(GL_QUADS);
    glVertex3f(0.0f, 0.0f, 0.0f);  // Esquina inferior izquierda
    glVertex3f(10.0f, 0.0f, 0.0f); // Esquina inferior derecha
    glVertex3f(10.0f, 10.0f, 0.0f); // Esquina superior derecha
    glVertex3f(0.0f, 10.0f, 0.0f);  // Esquina superior izquierda
    glEnd();

    // Establecer el color del borde (puedes cambiar este color)
    glColor3f(0.0f, 0.0f, 0.0f); // Negro para el borde

    // Establecer el grosor de la línea del borde
    glLineWidth(2.0f); // Grosor del borde

    // Dibuja el borde (cuadrado con líneas)
    glBegin(GL_LINE_LOOP);
    glVertex3f(0.0f, 0.0f, 0.0f);  // Esquina inferior izquierda
    glVertex3f(10.0f, 0.0f, 0.0f); // Esquina inferior derecha
    glVertex3f(10.0f, 10.0f, 0.0f); // Esquina superior derecha
    glVertex3f(0.0f, 10.0f, 0.0f);  // Esquina superior izquierda
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

int isInRestrictedRange(float x, float y) {
    for (int i = 0; i < sizeof(ranges) / sizeof(ranges[0]); i++) {
        if (x >= ranges[i].xMin && x <= ranges[i].xMax && y >= ranges[i].yMin && y <= ranges[i].yMax) {
            return 1; // Está dentro del rango restringido
        }
    }
    return 0; // Está fuera del rango restringido
}

// Funcionalidades del jugador
bool checkBorderCollision(float newX, float newY) {
    // Chequear los límites del mapa (asumido de 0 a 10)
    // Teniendo en cuenta el ancho y alto del pirata
    if (!(newX >= 0.0f && newX + pirate.width <= 10.0f &&
        newY >= 0.0f && newY + pirate.height <= 10.0f)) {
        return false; // Colisión con los bordes
    }

    // Verifica si las nuevas coordenadas están en un rango restringido
    if (isInRestrictedRange(newX, newY)) {
        return false; // Colisión con un rango restringido
    }

    return true; // No hay colisión
}

void adjustCameraGeneral() {
    float viewSize = 7.0f;

    orthoLeft = -1.0f;
    orthoRight = 11.0f;
    orthoBottom = -1.0f;
    orthoTop = 11.0f;

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

    // Aumentar el factor de interpolación de 0.1 a 0.5 para un movimiento más rápido
    float interpolationFactor = 0.5f;

    // Interpolación para un movimiento más suave de la cámara
    orthoLeft += (targetLeft - orthoLeft) * interpolationFactor;
    orthoRight += (targetRight - orthoRight) * interpolationFactor;
    orthoBottom += (targetBottom - orthoBottom) * interpolationFactor;
    orthoTop += (targetTop - orthoTop) * interpolationFactor;

    // Actualiza la proyección
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(orthoLeft, orthoRight, orthoBottom, orthoTop);
}

// Dibujado de objetos
void generateGoldNecklace(float* necklaceX, float* necklaceY) {
    float x, y;
    do {
        // Generar una posición aleatoria entre 0 y 10 para X e Y
        x = ((float)rand() / RAND_MAX) * 10.0f; // Generar una posición aleatoria en el rango [0, 10] para X
        y = ((float)rand() / RAND_MAX) * 10.0f; // Generar una posición aleatoria en el rango [0, 10] para Y
    } while (isInRestrictedRange(x, y));  // Verificar que la posición no esté dentro de un área restringida

    // Asignar las coordenadas válidas al collar
    *necklaceX = x;  // Asignar la posición de X al collar
    *necklaceY = y;  // Asignar la posición de Y al collar
}

void drawGoldNecklace(float necklaceX, float necklaceY) {
    // Color dorado para el collar
    glColor3f(0.85f, 0.65f, 0.12f);

    // Parámetros para un collar ovalado más pequeño
    float radiusX = 0.2f;  // Radio horizontal aún más corto
    float radiusY = 0.1f;  // Radio vertical aún más corto

    // Dibujar el contorno ovalado del collar
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 360; i += 5) {
        float angle = i * M_PI / 180.0f;
        float x = radiusX * cos(angle);
        float y = radiusY * sin(angle);
        glVertex3f(necklaceX + x, necklaceY + y, 0.0f);
    }
    glEnd();

    // Dibujar la cruz colgando del collar (ajustada proporcionalmente)
    glLineWidth(1.5f);

    // Punto de unión de la cruz (colgando del collar)
    float crossAttachX = necklaceX;
    float crossAttachY = necklaceY - radiusY;

    // Línea vertical de la cruz (más corta)
    glBegin(GL_LINES);
    glVertex3f(crossAttachX, crossAttachY, 0.0f);
    glVertex3f(crossAttachX, crossAttachY - 0.1f, 0.0f);
    glEnd();

    // Línea horizontal de la cruz (más estrecha)
    glBegin(GL_LINES);
    glVertex3f(crossAttachX - 0.05f, crossAttachY - 0.05f, 0.0f);
    glVertex3f(crossAttachX + 0.05f, crossAttachY - 0.05f, 0.0f);
    glEnd();
}

void generateGrapes(float* grapeX, float* grapeY) {
    float x, y;
    do {
        // Generar una posición aleatoria entre 0 y 10 para X e Y
        x = ((float)rand() / RAND_MAX) * 10.0f;
        y = ((float)rand() / RAND_MAX) * 10.0f;
    } while (isInRestrictedRange(x, y));  // Verificar que la posición no esté dentro de un área restringida

    // Asignar las coordenadas válidas al cofre
    *grapeX = x;
    *grapeY = y;
}

// Dibujado de objetos
void drawGrapes(float grapeX, float grapeY) {
    // *** Uva 1 ***
    glColor3f(0.6f, 0.2f, 0.6f); // Color púrpura para la uva
    glBegin(GL_POLYGON); // Dibuja un círculo para la uva
    for (int i = 0; i < 360; i++) {
        float angle = i * 3.14159f / 180.0f;
        float x = 0.05f * cos(angle); // Radio de la uva
        float y = 0.05f * sin(angle); // Radio de la uva
        glVertex3f(grapeX + x, grapeY + y, 0.0f); // Posición de la uva
    }
    glEnd();

    // *** Uva 2 ***
    glBegin(GL_POLYGON); // Dibuja otra uva
    for (int i = 0; i < 360; i++) {
        float angle = i * 3.14159f / 180.0f;
        float x = 0.05f * cos(angle); // Radio de la uva
        float y = 0.05f * sin(angle); // Radio de la uva
        glVertex3f(grapeX + 0.1f + x, grapeY + y, 0.0f); // Posición de la uva
    }
    glEnd();

    // *** Uva 3 ***
    glBegin(GL_POLYGON); // Dibuja otra uva
    for (int i = 0; i < 360; i++) {
        float angle = i * 3.14159f / 180.0f;
        float x = 0.05f * cos(angle); // Radio de la uva
        float y = 0.05f * sin(angle); // Radio de la uva
        glVertex3f(grapeX + 0.2f + x, grapeY + 0.05f + y, 0.0f); // Posición de la uva
    }
    glEnd();

    // *** Uva 4 ***
    glBegin(GL_POLYGON); // Dibuja otra uva
    for (int i = 0; i < 360; i++) {
        float angle = i * 3.14159f / 180.0f;
        float x = 0.05f * cos(angle); // Radio de la uva
        float y = 0.05f * sin(angle); // Radio de la uva
        glVertex3f(grapeX + 0.1f + x, grapeY + 0.1f + y, 0.0f); // Posición de la uva
    }
    glEnd();

    // *** Uva 5 ***
    glBegin(GL_POLYGON); // Dibuja otra uva
    for (int i = 0; i < 360; i++) {
        float angle = i * 3.14159f / 180.0f;
        float x = 0.05f * cos(angle); // Radio de la uva
        float y = 0.05f * sin(angle); // Radio de la uva
        glVertex3f(grapeX + 0.15f + x, grapeY + 0.15f + y, 0.0f); // Posición de la uva
    }
    glEnd();

    // *** Tallo (con un pequeño rectángulo) ***
    glColor3f(0.3f, 0.2f, 0.1f); // Color marrón para el tallo
    glBegin(GL_QUADS); // Dibuja el tallo
    glVertex3f(grapeX + 0.1f, grapeY + 0.2f, 0.0f); // Esquina inferior izquierda
    glVertex3f(grapeX + 0.12f, grapeY + 0.2f, 0.0f); // Esquina inferior derecha
    glVertex3f(grapeX + 0.12f, grapeY + 0.25f, 0.0f); // Esquina superior derecha
    glVertex3f(grapeX + 0.1f, grapeY + 0.25f, 0.0f); // Esquina superior izquierda
    glEnd();
}

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
        {5.5, 10.0, 0.2, 1.0},
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

void drawTreasureChest(float chestX, float chestY) {
    // *** Base del cofre ***
    glColor3f(0.6f, 0.4f, 0.2f); // Marrón madera
    glBegin(GL_QUADS);
    glVertex3f(chestX - 0.3f, chestY - 0.3f, 0.0f); // Incrementar dimensiones
    glVertex3f(chestX + 0.3f, chestY - 0.3f, 0.0f);
    glVertex3f(chestX + 0.3f, chestY, 0.0f);
    glVertex3f(chestX - 0.3f, chestY, 0.0f);
    glEnd();

    // *** Tapa del cofre ***
    glColor3f(0.5f, 0.3f, 0.1f); // Marrón más oscuro
    glBegin(GL_QUADS);
    glVertex3f(chestX - 0.3f, chestY, 0.0f);
    glVertex3f(chestX + 0.3f, chestY, 0.0f);
    glVertex3f(chestX + 0.225f, chestY + 0.15f, 0.0f); // Incrementar altura y ancho
    glVertex3f(chestX - 0.225f, chestY + 0.15f, 0.0f);
    glEnd();

    // *** Borde metálico inferior ***
    glColor3f(0.8f, 0.8f, 0.8f); // Gris claro
    glBegin(GL_QUADS);
    glVertex3f(chestX - 0.3f, chestY - 0.33f, 0.0f); // Ajustar borde inferior
    glVertex3f(chestX + 0.3f, chestY - 0.33f, 0.0f);
    glVertex3f(chestX + 0.3f, chestY - 0.3f, 0.0f);
    glVertex3f(chestX - 0.3f, chestY - 0.3f, 0.0f);
    glEnd();

    // *** Cerradura ***
    glColor3f(0.0f, 0.0f, 0.0f); // Negro
    glBegin(GL_QUADS);
    glVertex3f(chestX - 0.045f, chestY - 0.075f, 0.0f); // Ajustar cerradura
    glVertex3f(chestX + 0.045f, chestY - 0.075f, 0.0f);
    glVertex3f(chestX + 0.045f, chestY, 0.0f);
    glVertex3f(chestX - 0.045f, chestY, 0.0f);
    glEnd();

    // *** Detalle de la cerradura (círculo pequeño) ***
    glColor3f(0.9f, 0.7f, 0.0f); // Dorado
    glPointSize(7.5f); // Incrementar tamaño del punto
    glBegin(GL_POINTS);
    glVertex3f(chestX, chestY - 0.03f, 0.0f); // Ajustar posición del círculo
    glEnd();
}

void generateTreasureChest(float* chestX, float* chestY) {
    float x, y;
    do {
        // Generar una posición aleatoria entre 0 y 10 para X e Y
        x = ((float)rand() / RAND_MAX) * 10.0f;
        y = ((float)rand() / RAND_MAX) * 10.0f;
    } while (isInRestrictedRange(x, y));  // Verificar que la posición no esté dentro de un área restringida

    // Asignar las coordenadas válidas al cofre
    *chestX = x;
    *chestY = y;
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

// Colisiones
// Función para verificar si hay colisión entre el pirata y el cofre
bool checkChestCollision(float pirateX, float pirateY) {
    float chestRadius = 0.7f;  // Radio del cofre (la mitad del tamaño de la base)
    float distance = sqrt(pow(pirateX - treasureChestX, 2) + pow(pirateY - treasureChestY, 2));

    return distance < chestRadius;  // Retorna verdadero si el pirata está cerca del cofre
}

bool checkGrapeCollision(float pirateX, float pirateY) {
    float grapeRadius = 0.4f;  // Radio del cofre (la mitad del tamaño de la base)
    float distance = sqrt(pow(pirateX - grapeInitX, 2) + pow(pirateY - grapeInitY, 2));

    return distance < grapeRadius;  // Retorna verdadero si el pirata está cerca del cofre
}


bool checkNecklaceCollision(float pirateX, float pirateY) {
    float necklaceRadius = 0.5f;  // Radio del cofre (la mitad del tamaño de la base)
    float distance = sqrt(pow(pirateX - necklaceX, 2) + pow(pirateY - necklaceY, 2));

    return distance < necklaceRadius;  // Retorna verdadero si el pirata está cerca del cofre
}

// Misiones
// Función para inicializar la pila
void inicializarPila(Pila* pila, int capacidadInicial) {
    pila->misiones = (Mision*)malloc(sizeof(Mision) * capacidadInicial);
    if (pila->misiones == NULL) {
        exit(1);
    }
    pila->tope = -1;
    pila->capacidad = capacidadInicial;
}

// Función para verificar si la pila está vacía
int estaVacia(Pila* pila) {
    return pila->tope == -1;
}

// Función para expandir la pila cuando se alcanza la capacidad máxima
void expandirPila(Pila* pila) {
    pila->capacidad *= 2;  // Doblamos la capacidad
    pila->misiones = (Mision*)realloc(pila->misiones, sizeof(Mision) * pila->capacidad);
    if (pila->misiones == NULL) {
        exit(1);
    }
}

// Función para agregar una misión a la pila
void agregarMision(Pila* pila, Mision mision) {
    // Si la pila está llena, expandimos su capacidad
    if (pila->tope == pila->capacidad - 1) {
        expandirPila(pila);
    }

    pila->tope++;
    pila->misiones[pila->tope] = mision;
}

// Función para eliminar una misión de la pila
Mision eliminarMision(Pila* pila) {
    if (estaVacia(pila)) {
        exit(1);
    }
    return pila->misiones[pila->tope--];
}

// Función para ver la misión en la parte superior de la pila sin eliminarla
Mision verMisionSuperior(Pila* pila) {
    if (estaVacia(pila)) {
        exit(1);
    }
    return pila->misiones[pila->tope];
}

// Función para liberar la memoria de la pila
void liberarPila(Pila* pila) {
    free(pila->misiones);
}