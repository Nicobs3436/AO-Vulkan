#include "GEDrawingContext.h"
#include "GEGraphicsContext.h"
#include <glm/common.hpp>
#include <iostream>
#include <vector>

///////////////////////////////////////////////////////////////////////////////////////////
/////                                                                                 /////
/////                               Métodos públicos                                  /////
/////                                                                                 /////
///////////////////////////////////////////////////////////////////////////////////////////

//
// FUNCIÓN: GEDrawingContext::GEDrawingContext(GEGraphicsContext* gc)
//
// PROPÓSITO: Crea el contexto de dibujo (imágenes y cadena de intercambio)
//
GEDrawingContext::GEDrawingContext(GEGraphicsContext* gc, GEWindowPosition wpos)
{
	createSwapChain(gc, wpos);
	createImageViews(gc->device);
	createSyncObjects(gc->device);
	createQueues(gc);
}

//
// FUNCIÓN: GEDrawingContext::destroy(GEGraphicsContext* gc)
//
// PROPÓSITO: Destruye los componentes del contexto de dibujo
//
void GEDrawingContext::destroy(GEGraphicsContext* gc)
{
	for (size_t i = 0; i < frameCount; i++)
	{
		vkDestroySemaphore(gc->device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(gc->device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(gc->device, inFlightFences[i], nullptr);
	}

	for (uint32_t i = 0; i < imageCount; i++)
	{
		vkDestroyImageView(gc->device, imageViews[i], nullptr);
	}
	vkDestroySwapchainKHR(gc->device, swapChain, nullptr);
}

//
// FUNCIÓN: GEDrawingContext::recreate(GEGraphicsContext* gc,  GEWindowPosition wpos)
//
// PROPÓSITO: Reconstruye los componentes del contexto de dibujo
//
void GEDrawingContext::recreate(GEGraphicsContext* gc, GEWindowPosition wpos)
{
	for (uint32_t i = 0; i < imageCount; i++)
	{
		vkDestroyImageView(gc->device, imageViews[i], nullptr);
	}
	vkDestroySwapchainKHR(gc->device, swapChain, nullptr);

	createSwapChain(gc, wpos);
	createImageViews(gc->device);
}

//
// FUNCIÓN: GEDrawingContext::getFormat()
//
// PROPÓSITO: Obtiene el formato de las imágenes
//
VkFormat GEDrawingContext::getFormat()
{
	return imageFormat;
}

//
// FUNCIÓN: GEDrawingContext::getExtent()
//
// PROPÓSITO: Obtiene el tamaño de las imágenes
//
VkExtent2D GEDrawingContext::getExtent()
{
	return imageExtent;
}

//
// FUNCIÓN: GEDrawingContext::getImageCount()
//
// PROPÓSITO: Obtiene el número de imágenes
//
uint32_t GEDrawingContext::getImageCount()
{
	return imageCount;
}

//
// FUNCIÓN: GEDrawingContext::getCurrentImage()
//
// PROPÓSITO: Obtiene el índice de la imagen a generar
//
uint32_t GEDrawingContext::getCurrentImage()
{
	return currentImage;
}

///////////////////////////////////////////////////////////////////////////////////////////
/////                                                                                 /////
/////                     Métodos de creación de los componentes                      /////
/////                                                                                 /////
///////////////////////////////////////////////////////////////////////////////////////////

//
// FUNCIÓN: GEDrawingContext::createSwapChain(GEGraphicsContext* gc, GEWindowPosition wpos)
//
// PROPÓSITO: Crea los buffers de intercambio de imágenes, el vector de imágenes
//            y sus formatos y tamaños
//
void GEDrawingContext::createSwapChain(GEGraphicsContext* gc, GEWindowPosition wpos)
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gc->physicalDevice, gc->surface, &capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(gc->physicalDevice, gc->surface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(gc->physicalDevice, gc->surface, &formatCount, formats.data());
	}

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(formats);
	VkExtent2D extent = chooseSwapExtent(capabilities, wpos);

	imageCount = 4;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
	{
		imageCount = capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = gc->surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	if (gc->graphicsQueueFamilyIndex != gc->presentQueueFamilyIndex)
	{
		std::vector<uint32_t> queueFamilyIndices;
		queueFamilyIndices.resize(2);
		queueFamilyIndices.push_back(gc->graphicsQueueFamilyIndex);
		queueFamilyIndices.push_back(gc->presentQueueFamilyIndex);

		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	}
	else
	{
		std::vector<uint32_t> queueFamilyIndices;
		queueFamilyIndices.resize(1);
		queueFamilyIndices.push_back(gc->graphicsQueueFamilyIndex);

		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 1;
		createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	}

	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	createInfo.clipped = VK_TRUE;

	if (vkCreateSwapchainKHR(gc->device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(gc->device, swapChain, &imageCount, nullptr);
	images.resize(imageCount);
	vkGetSwapchainImagesKHR(gc->device, swapChain, &imageCount, images.data());

	imageFormat = surfaceFormat.format;
	imageExtent = extent;
}

//
// FUNCIÓN: GEDrawingContext::createImageViews()
//
// PROPÓSITO: Crea una vista para cada imagen de la cadena de intercambio
//
void GEDrawingContext::createImageViews(VkDevice device)
{
	imageViews.resize(imageCount);

	for (size_t i = 0; i < imageCount; i++)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = imageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &createInfo, nullptr, &imageViews[i])
			!= VK_SUCCESS)
		{
			throw std::runtime_error("failed to create image views!");
		}
	}
}

//
// FUNCIÓN: GEDrawingContext::createSyncObjects(VkDevice device)
//
// PROPÓSITO: Crea los semáforos y los fences para no sobreescribir las imágenes
//
void GEDrawingContext::createSyncObjects(VkDevice device)
{
	frameCount = imageCount - 1;
	imageAvailableSemaphores.resize(frameCount);
	renderFinishedSemaphores.resize(frameCount);
	inFlightFences.resize(frameCount);
	imagesInFlight.resize(images.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < frameCount; i++)
	{
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}

//
// FUNCIÓN: GEDrawingContext::createQueues(GEGraphicsContext* gc)
//
// PROPÓSITO: Obtiene las colas del dispositivo para enviar los comandos de generación y presentación de los gráficos
//
void GEDrawingContext::createQueues(GEGraphicsContext* gc)
{
	vkGetDeviceQueue(gc->device, gc->graphicsQueueFamilyIndex, 0, &graphicsQueue);
	vkGetDeviceQueue(gc->device, gc->presentQueueFamilyIndex, 0, &presentQueue);
}

///////////////////////////////////////////////////////////////////////////////////////////
/////                                                                                 /////
/////                       Métodos de generación de la imagen                        /////
/////                                                                                 /////
///////////////////////////////////////////////////////////////////////////////////////////

//
// FUNCIÓN: GEDrawingContext::waitForNextImage(GEGraphicsContext* gc)
//
// PROPÓSITO: Espera hasta que la próxima imagen esté lista para ser generada
//
void GEDrawingContext::waitForNextImage(GEGraphicsContext* gc)
{
	vkWaitForFences(gc->device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(gc->device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
	currentImage = imageIndex;

	 if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR && result != VK_ERROR_OUT_OF_DATE_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}
}

//
// FUNCIÓN: GEDrawingContext::submitGraphicsCommands(GEGraphicsContext* gc, std::vector<VkCommandBuffer> commandBuffers)
//
// PROPÓSITO: Envía los comandos gráficos al dispositivo
//
void GEDrawingContext::submitGraphicsCommands(GEGraphicsContext* gc, std::vector<VkCommandBuffer> commandBuffers)
{
	if (imagesInFlight[currentImage] != VK_NULL_HANDLE)
	{
		vkWaitForFences(gc->device, 1, &imagesInFlight[currentImage], VK_TRUE, UINT64_MAX);
	}
	imagesInFlight[currentImage] = inFlightFences[currentFrame];

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[currentImage];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(gc->device, 1, &inFlightFences[currentFrame]);

	VkResult result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}
}

//
// FUNCIÓN: GEDrawingContext::submitPresentCommands(GEGraphicsContext* gc)
//
// PROPÓSITO: Envía los comandos de presentación al dispositivo
//
void GEDrawingContext::submitPresentCommands(GEGraphicsContext* gc)
{
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
	VkSwapchainKHR swapChains[] = { swapChain };

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &currentImage;

	VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

	if (result != VK_ERROR_OUT_OF_DATE_KHR && result != VK_SUBOPTIMAL_KHR && result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}

	currentFrame = (currentFrame + 1) % frameCount;
}

///////////////////////////////////////////////////////////////////////////////////////////
/////                                                                                 /////
/////                              Métodos auxiliares                                 /////
/////                                                                                 /////
///////////////////////////////////////////////////////////////////////////////////////////


//
// FUNCIÓN: GEDrawingContext::chooseSwapSurfaceFormat()
//
// PROPÓSITO: Escoge el formato de imagen entre los soportados por la superficie
//
VkSurfaceFormatKHR GEDrawingContext::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const VkSurfaceFormatKHR& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

//
// FUNCIÓN: GEDrawingContext::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GEWindowPosition wpos)
//
// PROPÓSITO: Escoge el tamaño de las imágenes asegurando que puede ser soportado por
//            la superficie
//
VkExtent2D GEDrawingContext::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GEWindowPosition wpos)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D actualExtent = { };
		if (wpos.fullScreen) 
		{
			actualExtent.width = wpos.screenWidth;
			actualExtent.height = wpos.screenHeight;
		}
		else
		{
			actualExtent.width = wpos.width;
			actualExtent.height = wpos.height;
		}


		actualExtent.width = glm::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = glm::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}