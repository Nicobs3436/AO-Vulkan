#pragma once

#include "GEGraphicsContext.h"
#include "GEDrawingContext.h"
#include "GECommandContext.h"
#include "GERenderingContext.h"

#include "GEFigure.h"
#include "GEBalljoint.h"
#include "GECamera.h"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "GESkeleton.h"
#include "GEAnimation.h"
//
// CLASE: GEScene
//
// DESCRIPCIÓN: Clase que describe una escena
//
class GEScene
{
private:
	GERenderingContext* rc;
	GEFigure* ground;
	GEFigure* column;
	GEFigure* cross;
	GEBalljoint* armL;
	GEBalljoint* armR;
	GESkeleton* testSkeleton = nullptr;// Esqueleto temporal para comprobar la jerarquia
	GEAnimation* animation = nullptr;

	// Instante en el que comienza la animacion
	double animationStartTime = 0.0;
	float angle;
	GECamera* camera;
	glm::mat4 projection;

public:
	GEScene(GEGraphicsContext* gc, GEDrawingContext* dc, GECommandContext* cc);
	void destroy(GEGraphicsContext* gc);
	void recreate(GEGraphicsContext* gc, GEDrawingContext* dc, GECommandContext* cc);
	void update(GEGraphicsContext* gc, uint32_t index);
	void key_action(int key, bool pressed);
	void aspect_ratio(double aspect);

private:
	GEPipelineConfig* createPipelineConfig(VkExtent2D extent);
	void fillCommandBuffers(std::vector<VkCommandBuffer> commandBuffers);
};

