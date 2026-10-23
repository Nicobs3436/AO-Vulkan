#pragma once

#include <vulkan/vulkan.h>
#include <iostream>
#include "GEGraphicsContext.h"
#include "GEDrawingContext.h"
#include "GEPipelineConfig.h"
#include "GEDepthBuffer.h"

//
// CLASE: GERenderingContext
//
// DESCRIPCIÓN: Clase que describe un contexto de renderizado
//
class GERenderingContext
{
public:
	uint32_t imageCount;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;

private:
	VkFormat format;
	VkExtent2D extent;
	VkRenderPass renderPass;
	VkPipeline graphicsPipeline;
	std::vector<GEDepthBuffer*> depthBuffers;
	std::vector<VkFramebuffer> framebuffers;
	VkViewport viewport;
	VkRect2D scissor;

public:
	GERenderingContext(GEGraphicsContext* gc, GEDrawingContext* dc, GEPipelineConfig* config);
	void destroy(GEGraphicsContext* gc);
	void startFillingCommandBuffers(std::vector<VkCommandBuffer> commandBuffers);
	void endFillingCommandBuffers(std::vector<VkCommandBuffer> commandBuffers);

private:
	// Métodos de creación de componentes
	void createRenderPass(GEGraphicsContext* gc);
	void createGraphicsPipeline(GEGraphicsContext* gc, GEPipelineConfig* config);
	void createDepthBuffers(GEGraphicsContext* gc);
	void createFramebuffers(GEGraphicsContext* gc, GEDrawingContext* dc);

	// Métodos de definición del pipeline de renderizado
	void createPipelineLayout(GEGraphicsContext* gc, GEPipelineConfig* config);
	void createVertexShaderStageCreateInfo(GEGraphicsContext* gc, int resource, VkShaderModule* vertShaderModule, VkPipelineShaderStageCreateInfo* vertShaderStageInfo);
	void createFragmentShaderStageCreateInfo(GEGraphicsContext* gc, int resource, VkShaderModule* fragShaderModule, VkPipelineShaderStageCreateInfo* fragShaderStageInfo);
	void createPipelineVertexInputStateCreateInfo(GEPipelineConfig* config,VkPipelineVertexInputStateCreateInfo* vertexInputInfo,std::vector<VkVertexInputBindingDescription>& bindingDescriptions,std::vector<VkVertexInputAttributeDescription>& attributeDescriptions);
	void createPipelineInputAssemblyStateCreateInfo(VkPipelineInputAssemblyStateCreateInfo* inputAssembly);
	void createPipelineViewportStateCreateInfo(VkPipelineViewportStateCreateInfo* viewportState);
	void createPipelineRasterizationStateCreateInfo(GEPipelineConfig* config, VkPipelineRasterizationStateCreateInfo* rasterizer);
	void createPipelineMultisampleStateCreateInfo(VkPipelineMultisampleStateCreateInfo* multisampling);
	void createPipelineDepthStencilStateCreateInfo(GEPipelineConfig* config, VkPipelineDepthStencilStateCreateInfo* depthStencil);
	void createPipelineColorBlendStateCreateInfo(VkPipelineColorBlendAttachmentState* colorBlendAttachment, VkPipelineColorBlendStateCreateInfo* colorBlending);

	// Métodos auxiliares
	VkShaderModule createShaderModule(GEGraphicsContext* gc, const std::vector<char>& code);
	std::vector<char> getFileFromResource(int resource);
};

