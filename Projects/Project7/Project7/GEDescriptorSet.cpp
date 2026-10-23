#include "GEDescriptorSet.h"

#include "GEUniformBuffer.h"
#include <iostream>

//
// FUNCIÓN:GEDescriptorSet::GEDescriptorSet(GEGraphicsContext* gc, std::vector<GEUniformBuffer> ubos)
// 
// PROPÓSITO: Crea los conjuntos de descriptores asociados a los buffers
//
GEDescriptorSet::GEDescriptorSet(GEGraphicsContext* gc, GERenderingContext* rc, std::vector<GEUniformBuffer*> ubos)
{
	uint32_t bufferCount = (uint32_t) ubos.size();
	uint32_t imageCount = rc->imageCount;

	std::vector<VkDescriptorPoolSize> poolSizes(bufferCount);

	for (uint32_t i = 0; i < bufferCount; i++)
	{
		VkDescriptorPoolSize poolSize = {};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = imageCount;
		poolSizes[i] = poolSize;
	}

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = imageCount;

	if (vkCreateDescriptorPool(gc->device, &poolInfo, nullptr, &this->descriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}

	std::vector<VkDescriptorSetLayout> layouts(imageCount, rc->descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = this->descriptorPool;
	allocInfo.descriptorSetCount = imageCount;
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(imageCount);

	if (vkAllocateDescriptorSets(gc->device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < imageCount; i++)
	{
		std::vector<VkDescriptorBufferInfo> buffersInfo;
		buffersInfo.resize(bufferCount);

		for (uint32_t j = 0; j < bufferCount; j++)
		{
			buffersInfo[j] = {};
			buffersInfo[j].buffer = ubos[j]->buffers[i];
			buffersInfo[j].offset = 0;
			buffersInfo[j].range = ubos[j]->bufferSize;
		}

		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = this->descriptorSets[i];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = (uint32_t)buffersInfo.size();
		descriptorWrite.pBufferInfo = buffersInfo.data();

		vkUpdateDescriptorSets(gc->device, 1, &descriptorWrite, 0, nullptr);
	}
}

//
// FUNCIÓN: GEDescriptorSet::destroy(GEGraphicsContext* gc) 
//
// PROPÓSITO: Destruye los conjuntos de descriptores
//
void GEDescriptorSet::destroy(GEGraphicsContext* gc) 
{
	vkFreeDescriptorSets(gc->device, descriptorPool, (uint32_t)descriptorSets.size(), descriptorSets.data());
	vkDestroyDescriptorPool(gc->device, descriptorPool, nullptr);
}
