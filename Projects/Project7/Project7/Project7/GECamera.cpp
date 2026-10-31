#include "GECamera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//
// FUNCIÓN: GECamera::GECamera()
//
// PROPÓSITO: Construye una cámara
//
// COMENTARIOS: 
//     La posición inicial es (0,0,0).
//     La orientación incial es el sistema de coordenadas del modelo
//     El tamaño del paso inicial es 0.1
//     El támaño del giro inicial es 1.0 grados
//
GECamera::GECamera()
{
	Pos = glm::vec3(0.0f, 0.0f, 0.0f);
	Dir = glm::vec3(0.0f, 0.0f, 1.0f);
	Up = glm::vec3(0.0f, 1.0f, 0.0f);
	Right = glm::vec3(1.0f, 0.0f, 0.0f);

	moveStep = 0.1f;
	turnStep = 1.0f;
	cosAngle = (float)cos(glm::radians(turnStep));
	sinAngle = (float)sin(glm::radians(turnStep));

	turnLeftPressed = false;
	turnRightPressed = false;
	turnUpPressed = false;
	turnDownPressed = false;
	turnCWPressed = false;
	turnCCWPressed = false;
	moveLeftPressed = false;
	moveRightPressed = false;
	moveUpPressed = false;
	moveDownPressed = false;
}

//
// FUNCIÓN: GECamera::getViewMatrix()
//
// PROPÓSITO: Obtiene la matriz View para situar la cámara.
//
glm::mat4 GECamera::getViewMatrix()
{
	return glm::lookAt(Pos, Pos - Dir, Up);
}

//
// FUNCIÓN: GECamera::setPosition(glm::vec3 pos)
//
// PROPÓSITO: Asigna la posición de la cámara con respecto al sistema de coordenadas del modelo.
//
void GECamera::setPosition(glm::vec3 pos)
{
	Pos = glm::vec3(pos);
}

//
// FUNCIÓN: GECamera::setDirection(glm::vec3 dir, glm::vec3 up)
//
// PROPÓSITO: Asigna la orientación de la cámara.
//
void GECamera::setDirection(glm::vec3 dir, glm::vec3 up)
{
	Dir = glm::vec3(dir);
	Up = glm::vec3(up);
	Right = glm::cross(Up, Dir);
}

//
// FUNCIÓN: GECamera::setMoveStep(float step)
//
// PROPÓSITO: Asigna el avance en cada paso.
//
void GECamera::setMoveStep(float step)
{
	moveStep = step;
}

//
// FUNCIÓN: GECamera::setTurnStep(float step)
//
// PROPÓSITO: Asigna el ángulo de giro en cada paso.
//
void GECamera::setTurnStep(float step)
{
	turnStep = step;
	cosAngle = (float)cos(glm::radians(turnStep));
	sinAngle = (float)sin(glm::radians(turnStep));
}

//
// FUNCIÓN: GECamera::getPosition()
//
// PROPÓSITO: Obtiene la posición de la cámara.
//
glm::vec3 GECamera::getPosition()
{
	return Pos;
}

//
// FUNCIÓN: GECamera::getDirection()
//
// PROPÓSITO: Obtiene la orientación de la cámara (eje Z).
//
glm::vec3 GECamera::getDirection()
{
	return Dir;
}

//
// FUNCIÓN: GECamera::getUpDirection()
//
// PROPÓSITO: Obtiene la orientación cenital de la cámara (eje Y).
//
glm::vec3 GECamera::getUpDirection()
{
	return Up;
}

//
// FUNCIÓN: GECamera::getMoveStep()
//
// PROPÓSITO: Obtiene el avance en cada paso.
//
float GECamera::getMoveStep()
{
	return moveStep;
}

//
// FUNCIÓN: GECamera::getTurnStep()
//
// PROPÓSITO: Obtiene el ángulo de giro en cada paso.
//
float GECamera::getTurnStep()
{
	return turnStep;
}

//
// FUNCIÓN: GECamera::update()
//
// PROPÓSITO: Actualiza la posición y orientación de la cámara 
//
void GECamera::update()
{
	if (turnLeftPressed && !turnRightPressed) turnLeft();
	if (!turnLeftPressed && turnRightPressed) turnRight();
	if (turnUpPressed && !turnDownPressed) turnUp();
	if (!turnUpPressed && turnDownPressed) turnDown();
	if (turnCWPressed && !turnCCWPressed) turnCW();
	if (!turnCWPressed && turnCCWPressed) turnCCW();
	if (moveLeftPressed && !moveRightPressed) moveLeft();
	if (!moveLeftPressed && moveRightPressed) moveRight();
	if (moveUpPressed && !moveDownPressed) moveUp();
	if (!moveUpPressed && moveDownPressed) moveDown();

	moveFront();
}

//
// FUNCIÓN: GECamera::moveFront()
//
// PROPÓSITO: Mueve el observador un paso (moveStep) en la dirección -Dir 
//
void GECamera::moveFront()
{
	Pos -= moveStep * Dir;
}

//
// FUNCIÓN: GECamera::moveBack()
//
// PROPÓSITO: Mueve el observador un paso (moveStep) hacia atrás en la dirección Dir 
//
void GECamera::moveBack()
{
	Pos += moveStep * Dir;
}

//
// FUNCIÓN: GECamera::moveLeft()
//
// PROPÓSITO: Mueve el observador un paso (moveStep) hacia la izquierda. 
//
void GECamera::moveLeft()
{
//	Pos -= moveStep * Right;
	Pos -= 0.1f * Right;
}

//
// FUNCIÓN: GECamera::moveRight()
//
// PROPÓSITO: Mueve el observador un paso (moveStep) hacia la derecha. 
//
void GECamera::moveRight()
{
//	Pos += moveStep * Right;
	Pos += 0.1f * Right;
}

//
// FUNCIÓN: GECamera::moveUp()
//
// PROPÓSITO: Mueve el observador un paso (moveStep) hacia arriba. 
//
void GECamera::moveUp()
{
//	Pos += moveStep * Up;
	Pos += 0.1f * Up;
}

//
// FUNCIÓN: GECamera::moveDown()
//
// PROPÓSITO: Mueve el observador un paso (moveStep) hacia abajo. 
//
void GECamera::moveDown()
{
//	Pos -= moveStep * Up;
	Pos -= 0.1f * Up;
}

//
// FUNCIÓN: GECamera::turnRight()
//
// PROPÓSITO: Rota el observador un paso (angleStep) hacia su derecha.
//
void GECamera::turnRight()
{
	Dir.x = cosAngle * Dir.x - sinAngle * Right.x;
	Dir.y = cosAngle * Dir.y - sinAngle * Right.y;
	Dir.z = cosAngle * Dir.z - sinAngle * Right.z;

	// Right = Up x Dir
	Right = glm::cross(Up, Dir);
}

//
// FUNCIÓN: CACamera::turnLeft()
//
// PROPÓSITO: Rota el observador un paso (angleStep) hacia su izquierda.
//
void GECamera::turnLeft()
{
	Dir.x = cosAngle * Dir.x + sinAngle * Right.x;
	Dir.y = cosAngle * Dir.y + sinAngle * Right.y;
	Dir.z = cosAngle * Dir.z + sinAngle * Right.z;

	// Right = Up x Dir
	Right = glm::cross(Up, Dir);
}

//
// FUNCIÓN: GECamera::turnUp()
//
// PROPÓSITO: Rota el observador un paso (angleStep) hacia arriba.
//
void GECamera::turnUp()
{
	Dir.x = cosAngle * Dir.x - sinAngle * Up.x;
	Dir.y = cosAngle * Dir.y - sinAngle * Up.y;
	Dir.z = cosAngle * Dir.z - sinAngle * Up.z;

	// Up = Dir x Right
	Up = glm::cross(Dir, Right);
}

//
// FUNCIÓN: GECamera::turnDown()
//
// PROPÓSITO: Rota el observador un paso (angleStep) hacia abajo.
//
void GECamera::turnDown()
{
	Dir.x = cosAngle * Dir.x + sinAngle * Up.x;
	Dir.y = cosAngle * Dir.y + sinAngle * Up.y;
	Dir.z = cosAngle * Dir.z + sinAngle * Up.z;

	// Up = Dir x Right
	Up = glm::cross(Dir, Right);
}

//
// FUNCIÓN: GECamera::turnCW()
//
// PROPÓSITO: Rota el observador un paso (angleStep) en sentido del reloj.
//
void GECamera::turnCW()
{
	Up.x = cosAngle * Up.x + sinAngle * Right.x;
	Up.y = cosAngle * Up.y + sinAngle * Right.y;
	Up.z = cosAngle * Up.z + sinAngle * Right.z;

	// Right = Up x Dir
	Right = glm::cross(Up, Dir);
}

//
// FUNCIÓN: GECamera::turnCCW()
//
// PROPÓSITO: Rota el observador un paso (angleStep) en sentido contrario al reloj.
//
void GECamera::turnCCW()
{
	Up.x = cosAngle * Up.x - sinAngle * Right.x;
	Up.y = cosAngle * Up.y - sinAngle * Right.y;
	Up.z = cosAngle * Up.z - sinAngle * Right.z;

	// Right = Up x Dir
	Right = glm::cross(Up, Dir);
}

//
// FUNCIÓN: GECamera::setTurnLeft(bool flag)
//
// PROPÓSITO: Activa o desactiva el giro a la izquierda
//
void GECamera::setTurnLeft(bool flag)
{
	turnLeftPressed = flag;
}

//
// FUNCIÓN: GECamera::setTurnRight(bool flag)
//
// PROPÓSITO: Activa o desactiva el giro a la derecha
//
void GECamera::setTurnRight(bool flag)
{
	turnRightPressed = flag;
}

//
// FUNCIÓN: GECamera::setTurnUp(bool flag)
//
// PROPÓSITO: Activa o desactiva el giro hacia arriba
//
void GECamera::setTurnUp(bool flag)
{
	turnUpPressed = flag;
}

//
// FUNCIÓN: GECamera::setTurnDown(bool flag)
//
// PROPÓSITO: Activa o desactiva el giro hacia abajo
//
void GECamera::setTurnDown(bool flag)
{
	turnDownPressed = flag;
}

//
// FUNCIÓN: GECamera::setTurnCW(bool flag)
//
// PROPÓSITO: Activa o desactiva el giro horario
//
void GECamera::setTurnCW(bool flag)
{
	turnCWPressed = flag;
}

//
// FUNCIÓN: GECamera::setTurnCCW(bool flag)
//
// PROPÓSITO: Activa o desactiva el giro antihorario
//
void GECamera::setTurnCCW(bool flag)
{
	turnCCWPressed = flag;
}

//
// FUNCIÓN: GECamera::setMoveLeft(bool flag)
//
// PROPÓSITO: Activa o desactiva el desplazamiento a la izquierda
//
void GECamera::setMoveLeft(bool flag)
{
	moveLeftPressed = flag;
}

//
// FUNCIÓN: GECamera::setMoveRight(bool flag)
//
// PROPÓSITO: Activa o desactiva el desplazamiento a la derecha
//
void GECamera::setMoveRight(bool flag)
{
	moveRightPressed = flag;
}

//
// FUNCIÓN: GECamera::setMoveUp(bool flag)
//
// PROPÓSITO: Activa o desactiva el desplazamiento hacia arriba
//
void GECamera::setMoveUp(bool flag)
{
	moveUpPressed = flag;
}

//
// FUNCIÓN: GECamera::setMoveDown(bool flag)
//
// PROPÓSITO: Activa o desactiva el desplazamiento hacia abajo
//
void GECamera::setMoveDown(bool flag)
{
	moveDownPressed = flag;
}