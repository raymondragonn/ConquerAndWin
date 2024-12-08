//Librerias para cargar la textura
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" //Debe estar dentro de la misma carpeta del proyecto
#include <stdbool.h>

//Inicializacion de la textura en glut
GLuint textura; //Esto va justo abajo de las librerias (justo donde se declaran las variables globales)


//push();
//Habilitar la textura dentro del push donde dibujes y antes de empezar a dibujar
glEnable(GL_TEXTURE_2D);   //Habilitamos las texturas para cada disco 
glBindTexture(GL_TEXTURE_2D, textura);

//dibujo...

//Deshabilitar la textura antes del pop, despues del dibujo
glDisable(GL_TEXTURE_2D);
//pop();


//Funcion para cargar la textura
GLuint cargarTextura(const char* archivo)
{
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* imagen = stbi_load(archivo, &width, &height, &channels, 0);

    if (!imagen) {
        printf("Error cargando la imagen: %s\n", archivo);
        return 0;
    }

    GLuint texturaID;
    glGenTextures(1, &texturaID);
    glBindTexture(GL_TEXTURE_2D, texturaID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum formato;
    if (channels == 1) formato = GL_RED;
    else if (channels == 3) formato = GL_RGB;
    else if (channels == 4) formato = GL_RGBA;

    glTexImage2D(GL_TEXTURE_2D, 0, formato, width, height, 0, formato, GL_UNSIGNED_BYTE, imagen);
    gluBuild2DMipmaps(GL_TEXTURE_2D, formato, width, height, formato, GL_UNSIGNED_BYTE, imagen);

    stbi_image_free(imagen);

    return texturaID;
}


//Dentro del main cargas la textura ( antes del GlutMainLoop)
textura = cargarTextura("moneda.jpg"); //Nombre de la imagen a utilizar para la textura, debeee estar dentro de la misma carpeta que el programa 
    if (textura == 0) {
        printf("Error al cargar la textura\n");
        return -1;
    }

//Habilitas latextura nuevamente, igual antes del Loop
glEnable(GL_TEXTURE_2D);
glBindTexture(GL_TEXTURE_2D, textura);

//GlutMainLoop();
//Return 0;