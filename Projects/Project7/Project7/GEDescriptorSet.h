#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "GEGraphicsContext.h"
#include "GERenderingContext.h"
#include "GEUniformBuffer.h"

//
// CLASE: GEDescriptorSet
//
// DESCRIPCIÓN: Clase que describe un conjunto de descriptores 
//
class GEDescriptorSet
{
private:
	VkDescriptorPool descriptorPool;

public:
	std::vector<VkDescriptorSet> descriptorSets;

public:
	GEDescriptorSet(GEGraphicsContext* gc, GERenderingContext* rc, std::vector<GEUniformBuffer*> ubos);
	void destroy(GEGraphicsContext* gc);
};

