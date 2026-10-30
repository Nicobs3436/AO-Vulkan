#include "GEVertexBuffer.h"

#include <iostream>

//
// FUNCIÓN: GEVertexBuffer::GEVertexBuffer(GEGraphicsContext* gc, size_t size, const void* data)
//
// PROPÓSITO: Crea un Vertex Buffer
//
GEVertexBuffer::GEVertexBuffer(GEGraphicsContext* gc, size_t size, const void* data)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(gc->device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(gc->device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = gc->findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (vkAllocateMemory(gc->device, &allocInfo, nullptr, &memory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(gc->device, buffer, memory, 0);

	void* gpudata;
	vkMapMemory(gc->device, memory, 0, size, 0, &gpudata);
	memcpy(gpudata, data, size);
	vkUnmapMemory(gc->device, memory);
}

//
// FUNCIÓN: GEVertexBuffer::destroy(GEGraphicsContext* gc)
//
// PROPÓSITO: Destruye los campos de un Vertex Buffer
//
void GEVertexBuffer::destroy(GEGraphicsContext* gc)
{
	vkDestroyBuffer(gc->device, buffer, nullptr);
	vkFreeMemory(gc->device, memory, nullptr);
}
