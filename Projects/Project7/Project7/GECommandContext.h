#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "GEGraphicsContext.h"

//
// CLASE: GECommandContext
//
// DESCRIPCIÓN: Clase que almacena los buffers de comandos vinculados a cada imagen de la swapchain
//
class GECommandContext
{
public:
	std::vector<VkCommandBuffer> commandBuffers;

	GECommandContext(GEGraphicsContext* gc, uint32_t imageCount);
	void destroy(GEGraphicsContext* gc);

private:
	VkCommandPool commandPool;
	
	// Métodos de inicialización 
	void createCommandPool(GEGraphicsContext* gc);
	void createCommandBuffers(GEGraphicsContext* gc, uint32_t imageCount);
};

