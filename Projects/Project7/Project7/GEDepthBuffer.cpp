#include "GEDepthBuffer.h"

#include <iostream>

//
// FUNCIÓN: GEDepthBuffer::GEDepthBuffer(GEGraphicsContext* gc, VkExtent2D extent)
//
// PROPÓSITO: Crea un buffer de profundidad (con su imagen, memoria y vista)
//
GEDepthBuffer::GEDepthBuffer(GEGraphicsContext* gc, VkExtent2D extent)
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = extent.width;
	imageInfo.extent.height = extent.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = gc->findDepthFormat(); //  format;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(gc->device, &imageInfo, nullptr, &image) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create depth buffer image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(gc->device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = gc->findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(gc->device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate depth buffer image memory!");
	}

	vkBindImageMemory(gc->device, image, imageMemory, 0);

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = gc->findDepthFormat();
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(gc->device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create depth buffer image view!");
	}

}

//
// FUNCIÓN: GEDepthBuffer::destroy(GEGraphicsContext* gc)
//
// PROPÓSITO: Destruye un buffer de profundidad
//
void GEDepthBuffer::destroy(GEGraphicsContext* gc)
{
	vkDestroyImageView(gc->device, imageView, nullptr);
	vkFreeMemory(gc->device, imageMemory, nullptr);
	vkDestroyImage(gc->device, image, nullptr);
}