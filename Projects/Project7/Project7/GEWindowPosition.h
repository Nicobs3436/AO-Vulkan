#pragma once

//
// ESTRUCTURA: GEWindowPosition
//
// DESCRIPCIÓN: Estructura que almacena la posición y el tamaño de la ventana gráfica
//
typedef struct {
	int width;
	int height;
	int Xpos;
	int Ypos;
	int screenWidth;
	int screenHeight;
	bool fullScreen;
} GEWindowPosition;
