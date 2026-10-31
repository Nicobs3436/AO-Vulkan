#pragma once

#include <vulkan/vulkan.h>
#include "GEGraphicsContext.h"

//
// CLASE: GEDepthBuffer
//
// DESCRIPCIÓN: Clase que describe un buffer de profundidad
//
class GEDepthBuffer
{
public:
	VkImage image;
	VkDeviceMemory imageMemory;
	VkImageView imageView;

	GEDepthBuffer(GEGraphicsContext* gc, VkExtent2D extent);
	void destroy(GEGraphicsContext* gc);
};

