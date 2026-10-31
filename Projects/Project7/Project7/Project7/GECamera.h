#pragma once

#include <glm/glm.hpp>

class GECamera {
public:
	GECamera();
	glm::mat4 getViewMatrix();

	void setPosition(glm::vec3 pos);
	void setDirection(glm::vec3 dir, glm::vec3 up);
	void setMoveStep(float step);
	void setTurnStep(float step);

	glm::vec3 getPosition();
	glm::vec3 getDirection();
	glm::vec3 getUpDirection();
	float getMoveStep();
	float getTurnStep();

	void update();

	void setTurnLeft(bool flag);
	void setTurnRight(bool flag);
	void setTurnUp(bool flag);
	void setTurnDown(bool flag);
	void setTurnCW(bool flag);
	void setTurnCCW(bool flag);
	void setMoveLeft(bool flag);
	void setMoveRight(bool flag);
	void setMoveUp(bool flag);
	void setMoveDown(bool flag);

private:
	glm::vec3 Pos;
	glm::vec3 Dir;
	glm::vec3 Up;
	glm::vec3 Right;

	float moveStep;
	float turnStep;
	float cosAngle;
	float sinAngle;

	bool turnLeftPressed;
	bool turnRightPressed;
	bool turnUpPressed;
	bool turnDownPressed;
	bool turnCWPressed;
	bool turnCCWPressed;
	bool moveLeftPressed;
	bool moveRightPressed;
	bool moveUpPressed;
	bool moveDownPressed;

	void turnRight();
	void turnLeft();
	void turnUp();
	void turnDown();
	void turnCW();
	void turnCCW();

	void moveLeft();
	void moveRight();
	void moveUp();
	void moveDown();
	void moveFront();
	void moveBack();
};

