#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "GEWindowPosition.h"
#include "GEGraphicsContext.h"
#include "GEDrawingContext.h"
#include "GECommandContext.h"
#include "GEScene.h"

const int WIDTH = 800;
const int HEIGHT = 600;

//
// CLASE: GEApplication
//
// DESCRIPCIÓN: Clase que crea y lanza la aplicación gráfica.
//
class GEApplication
{
public:
	void run();

private:
	GLFWwindow* window;
	GEWindowPosition windowPos;
	GEGraphicsContext* gc;
	GEDrawingContext* dc;
	GECommandContext* cc;
	GEScene* scene;

	// Métodos principales
	GLFWwindow* initWindow();
	GEWindowPosition initWindowPos();
	void mainLoop();
	void draw();
	void cleanup();
	void swapFullScreen();
	void resize();

	// Respuesta a eventos
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};

