#include "GEGraphicsContext.h"
#include <iostream>
#include <vector>
#include <fstream>

///////////////////////////////////////////////////////////////////////////////////////////
/////                                                                                 /////
/////                              Métodos públicos                                   /////
/////                                                                                 /////
///////////////////////////////////////////////////////////////////////////////////////////

//
// FUNCIÓN: GEGraphicsContext::GEGraphicsContext()
//
// PROPÓSITO: Crea un contexto gráfico de Vulkan (instancia, dispositivo físico y lógico)
//
GEGraphicsContext::GEGraphicsContext(GLFWwindow* window)
{
	createInstance();
	createSurface(window);
    // showInstanceProperties();
	pickPhysicalDevice();
	// showDevices();
    createLogicalDevice();
}

//
// FUNCIÓN: GEGraphicsContext::~GEGraphicsContext()
//
// PROPÓSITO: Destruye el contexto gráfico
//
GEGraphicsContext::~GEGraphicsContext()
{
    vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
}

//
// FUNCIÓN: GEGraphicsContext::findMemoryType()
//
// PROPÓSITO: Busca el tipo de memoria adecuado para el filtro indicado
//
uint32_t GEGraphicsContext::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

//
// FUNCIÓN: GEGraphicsContext::findDepthFormat()
//
// PROPÓSITO: Busca el formato adecuado para el buffer de profundidad
//
VkFormat GEGraphicsContext::findDepthFormat()
{
	VkFormat candidates[] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}

///////////////////////////////////////////////////////////////////////////////////////////
/////                                                                                 /////
/////                     Métodos de inicialización de Vulkan                         /////
/////                                                                                 /////
///////////////////////////////////////////////////////////////////////////////////////////

//
// FUNCIÓN: GEGraphicsContext::createInstance()
//
// PROPÓSITO: Crea la instancia de Vulkan
//
void GEGraphicsContext::createInstance()
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Game Engine";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "GE";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;
	createInfo.enabledLayerCount = 0;

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
	}
}

//
// FUNCIÓN: GEGraphicsContext::createSurface(GLFWwindow* window)
//
// PROPÓSITO: Crea la superficie sobre la que mostrar la representación gráfica
//
void GEGraphicsContext::createSurface(GLFWwindow* window)
{
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}
}

//
// FUNCIÓN: GEGraphicsContext::pickPhysicalDevice()
//
// PROPÓSITO: Selecciona el dispositivo físico sobre el que generar los gráficos
//
void GEGraphicsContext::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	for (unsigned int i = 0; i < deviceCount; i++)
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && isDeviceSuitable(devices[i]))
		{
			physicalDevice = devices[i];
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
	{
		for (unsigned int i = 0; i < deviceCount; i++)
		{
			if (isDeviceSuitable(devices[i]))
			{
				physicalDevice = devices[i];
				break;
			}
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}

	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
}

//
// FUNCIÓN: GEGraphicsContext::createLogicalDevice()
//
// PROPÓSITO: Crea el dispositivo lógico sobre el que generar los gráficos 
//            y selecciona la familia de colas sobre el dispositivo
//
void GEGraphicsContext::createLogicalDevice()
{
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfo;
    uint32_t queueCreateInfoCount;

    if (graphicsQueueFamilyIndex == presentQueueFamilyIndex)
    {
        VkDeviceQueueCreateInfo graphicsQueueCreateInfo = {};
        graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        graphicsQueueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
        graphicsQueueCreateInfo.queueCount = 1;
        float graphicsQueuePriority = 1.0f;
        graphicsQueueCreateInfo.pQueuePriorities = &graphicsQueuePriority;

        queueCreateInfoCount = 1;
        queueCreateInfo = { graphicsQueueCreateInfo };
    }
    else
    {
        VkDeviceQueueCreateInfo graphicsQueueCreateInfo = {};
        graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        graphicsQueueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
        graphicsQueueCreateInfo.queueCount = 1;
        float graphicsQueuePriority = 1.0f;
        graphicsQueueCreateInfo.pQueuePriorities = &graphicsQueuePriority;

        VkDeviceQueueCreateInfo presentQueueCreateInfo = {};
        presentQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        presentQueueCreateInfo.queueFamilyIndex = presentQueueFamilyIndex;
        presentQueueCreateInfo.queueCount = 1;
        float presentQueuePriority = 1.0f;
        presentQueueCreateInfo.pQueuePriorities = &presentQueuePriority;

        queueCreateInfoCount = 2;
        queueCreateInfo = { graphicsQueueCreateInfo ,presentQueueCreateInfo };
    }

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfo.data();
    createInfo.queueCreateInfoCount = queueCreateInfoCount;
    createInfo.enabledExtensionCount = 0;
    createInfo.enabledLayerCount = 0;

    std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkPhysicalDeviceFeatures supportedFeatures = {};
    VkPhysicalDeviceFeatures requiredFeatures = {};
    vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);
    requiredFeatures.multiDrawIndirect = supportedFeatures.multiDrawIndirect;
    requiredFeatures.tessellationShader = VK_TRUE;
    requiredFeatures.geometryShader = VK_TRUE;
    requiredFeatures.samplerAnisotropy = VK_TRUE;
    createInfo.pEnabledFeatures = &requiredFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }

}

///////////////////////////////////////////////////////////////////////////////////////////
/////                                                                                 /////
/////                              Métodos auxiliares                                 /////
/////                                                                                 /////
///////////////////////////////////////////////////////////////////////////////////////////

//
// FUNCIÓN: GEGraphicsContext::showInstanceProperties()
//
// PROPÓSITO: Muestra las propiedades de la instancia
//
void GEGraphicsContext::showInstanceProperties() 
{
	// Mostrar propiedades de la instancia
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
	std::cout << "Available extensions:" << std::endl;
	for (const VkExtensionProperties& extension : extensions)
	{
		std::cout << "\t" << extension.extensionName << std::endl;
	}

	// Mostrar requisitos de GLFW
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::cout << "GLFW extensions:" << std::endl;
	for (uint32_t i = 0; i < glfwExtensionCount; i++)
	{
		std::cout << "\t" << glfwExtensions[i] << std::endl;
	}
}

//
// FUNCIÓN: GEGraphicsContext::isDeviceSuitable(VkPhysicalDevice pDevice)
//
// PROPÓSITO: Verifica si un dispositivo físico admite generación de gráficos
//
bool GEGraphicsContext::isDeviceSuitable(VkPhysicalDevice pDevice)
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount, nullptr);
	std::vector< VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount, queueFamilies.data());

	bool graphics = false;
	bool present = false;
	for (uint32_t i = 0; i < queueFamilyCount; i++)
	{
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(pDevice, i, surface, &presentSupport);
		if (presentSupport && (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
		{
			graphicsQueueFamilyIndex = i;
			presentQueueFamilyIndex = i;
			graphics = true;
			present = true;
			break;
		}
	}

	if (!(graphics && present))
	{
		for (uint32_t i = 0; i < queueFamilyCount; i++)
		{
			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				graphicsQueueFamilyIndex = i;
				graphics = true;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(pDevice, i, surface, &presentSupport);
			if (presentSupport)
			{
				presentQueueFamilyIndex = i;
				present = true;
			}
		}
	}

	if (graphics && present) return true;

	return false;
}
//
// FUNCIÓN: GEGraphicsContext::showInstanceProperties()
//
// PROPÓSITO: Muestra los dispositivos físicos disponibles 
//            y genera un archivo con las propiedades de cada uno
//
void GEGraphicsContext::showDevices()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    // Mostrar los dispositivos físicos disponibles
    std::cout << "Physical devices:" << std::endl;
    for (unsigned int i = 0; i < deviceCount; i++)
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);
        std::cout << "\t" << deviceProperties.deviceName << std::endl;

        resumeDeviceProperties(devices[i], i);
    }

    std::cout << "Selected device:" << std::endl;
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    std::cout << "\t" << deviceProperties.deviceName << std::endl;
}

//
// FUNCIÓN: GEGraphicsContext::resumeDeviceProperties(VkPhysicalDevice pDevice, int index)
//
// PROPÓSITO: Escribe las caracterísiticas del dispositivo físico
//
#define BOOL(x) ( (x)!=0? "true" : "false")
void GEGraphicsContext::resumeDeviceProperties(VkPhysicalDevice pDevice, int index)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    VkPhysicalDeviceMemoryProperties deviceMemories;
    uint32_t queueFamilyCount = 0;
    uint32_t layerCount = 0;
    uint32_t extensionCount = 0;

    vkGetPhysicalDeviceProperties(pDevice, &deviceProperties);
    vkGetPhysicalDeviceFeatures(pDevice, &deviceFeatures);
    vkGetPhysicalDeviceMemoryProperties(pDevice, &deviceMemories);
    vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount, queueProperties.data());
    vkEnumerateDeviceLayerProperties(pDevice, &layerCount, nullptr);
    std::vector<VkLayerProperties> layerProperties(layerCount);
    vkEnumerateDeviceLayerProperties(pDevice, &layerCount, layerProperties.data());
    vkEnumerateDeviceExtensionProperties(pDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensionProperties(extensionCount);
    vkEnumerateDeviceExtensionProperties(pDevice, nullptr, &extensionCount, extensionProperties.data());

    char filename[13];
    sprintf_s(filename, "device_%i.txt", index);
    std::ofstream fout(filename);
    fout << "Device properties" << std::endl;
    fout << "\tapiVersion     " << deviceProperties.apiVersion << std::endl;
    fout << "\tdriverVersion  " << deviceProperties.driverVersion << std::endl;
    fout << "\tvendorID       " << deviceProperties.vendorID << std::endl;
    fout << "\tdeviceID       " << deviceProperties.deviceID << std::endl;
    fout << "\tdeviceType     " << deviceProperties.deviceType << std::endl;
    fout << "\tdeviceName     " << deviceProperties.deviceName << std::endl;
    fout << std::endl;
    fout << "Device layers" << std::endl;
    for (uint32_t i = 0; i < layerCount; i++)
    {
        fout << "\t" << layerProperties[i].layerName << std::endl;
    }
    fout << std::endl;
    fout << "Device extensions" << std::endl;
    for (uint32_t i = 0; i < extensionCount; i++)
    {
        fout << "\t" << extensionProperties[i].extensionName << std::endl;
    }
    fout << std::endl;
    fout << "Device limits" << std::endl;
    fout << "\tmaxImageDimension1D     " << deviceProperties.limits.maxImageDimension1D << std::endl;
    fout << "\tmaxImageDimension2D     " << deviceProperties.limits.maxImageDimension2D << std::endl;
    fout << "\tmaxImageDimension3D     " << deviceProperties.limits.maxImageDimension3D << std::endl;
    fout << "\tmaxImageDimensionCube   " << deviceProperties.limits.maxImageDimensionCube << std::endl;
    fout << "\tmaxImageArrayLayers     " << deviceProperties.limits.maxImageArrayLayers << std::endl;
    fout << "\tmaxTexelBufferElements  " << deviceProperties.limits.maxTexelBufferElements << std::endl;
    fout << "\tmaxUniformBufferRange   " << deviceProperties.limits.maxUniformBufferRange << std::endl;
    fout << "\tmaxStorageBufferRange   " << deviceProperties.limits.maxStorageBufferRange << std::endl;
    fout << "\tmaxPushConstantsSize    " << deviceProperties.limits.maxPushConstantsSize << std::endl;
    fout << "\tmaxMemoryAllocationCount       " << deviceProperties.limits.maxMemoryAllocationCount << std::endl;
    fout << "\tmaxSamplerAllocationCount      " << deviceProperties.limits.maxSamplerAllocationCount << std::endl;
    fout << "\tbufferImageGranularity  " << deviceProperties.limits.bufferImageGranularity << std::endl;
    fout << "\tsparseAddressSpaceSize  " << deviceProperties.limits.sparseAddressSpaceSize << std::endl;
    fout << "\tmaxBoundDescriptorSets  " << deviceProperties.limits.maxBoundDescriptorSets << std::endl;
    fout << "\tmaxPerStageDescriptorSamplers  " << deviceProperties.limits.maxPerStageDescriptorSamplers << std::endl;
    fout << "\tmaxPerStageDescriptorUniformBuffers    " << deviceProperties.limits.maxPerStageDescriptorUniformBuffers << std::endl;
    fout << "\tmaxPerStageDescriptorStorageBuffers    " << deviceProperties.limits.maxPerStageDescriptorStorageBuffers << std::endl;
    fout << "\tmaxPerStageDescriptorSampledImages     " << deviceProperties.limits.maxPerStageDescriptorSampledImages << std::endl;
    fout << "\tmaxPerStageDescriptorStorageImages     " << deviceProperties.limits.maxPerStageDescriptorStorageImages << std::endl;
    fout << "\tmaxPerStageDescriptorInputAttachments  " << deviceProperties.limits.maxPerStageDescriptorInputAttachments << std::endl;
    fout << "\tmaxPerStageResources    " << deviceProperties.limits.maxPerStageResources << std::endl;
    fout << "\tmaxDescriptorSetSamplers       " << deviceProperties.limits.maxDescriptorSetSamplers << std::endl;
    fout << "\tmaxDescriptorSetUniformBuffers " << deviceProperties.limits.maxDescriptorSetUniformBuffers << std::endl;
    fout << "\tmaxDescriptorSetUniformBuffersDynamic  " << deviceProperties.limits.maxDescriptorSetUniformBuffersDynamic << std::endl;
    fout << "\tmaxDescriptorSetStorageBuffers  " << deviceProperties.limits.maxDescriptorSetStorageBuffers << std::endl;
    fout << "\tmaxDescriptorSetStorageBuffersDynamic  " << deviceProperties.limits.maxDescriptorSetStorageBuffersDynamic << std::endl;
    fout << "\tmaxDescriptorSetSampledImages  " << deviceProperties.limits.maxDescriptorSetSampledImages << std::endl;
    fout << "\tmaxDescriptorSetStorageImages  " << deviceProperties.limits.maxDescriptorSetStorageImages << std::endl;
    fout << "\tmaxDescriptorSetInputAttachments  " << deviceProperties.limits.maxDescriptorSetInputAttachments << std::endl;
    fout << "\tmaxVertexInputAttributes  " << deviceProperties.limits.maxVertexInputAttributes << std::endl;
    fout << "\tmaxVertexInputBindings  " << deviceProperties.limits.maxVertexInputBindings << std::endl;
    fout << "\tmaxVertexInputAttributeOffset  " << deviceProperties.limits.maxVertexInputAttributeOffset << std::endl;
    fout << "\tmaxVertexInputBindingStride  " << deviceProperties.limits.maxVertexInputBindingStride << std::endl;
    fout << "\tmaxVertexOutputComponents  " << deviceProperties.limits.maxVertexOutputComponents << std::endl;
    fout << "\tmaxTessellationGenerationLevel  " << deviceProperties.limits.maxTessellationGenerationLevel << std::endl;
    fout << "\tmaxTessellationPatchSize  " << deviceProperties.limits.maxTessellationPatchSize << std::endl;
    fout << "\tmaxTessellationControlPerVertexInputComponents  " << deviceProperties.limits.maxTessellationControlPerVertexInputComponents << std::endl;
    fout << "\tmaxTessellationControlPerVertexOutputComponents  " << deviceProperties.limits.maxTessellationControlPerVertexOutputComponents << std::endl;
    fout << "\tmaxTessellationControlPerPatchOutputComponents  " << deviceProperties.limits.maxTessellationControlPerPatchOutputComponents << std::endl;
    fout << "\tmaxTessellationControlTotalOutputComponents  " << deviceProperties.limits.maxTessellationControlTotalOutputComponents << std::endl;
    fout << "\tmaxTessellationEvaluationInputComponents  " << deviceProperties.limits.maxTessellationEvaluationInputComponents << std::endl;
    fout << "\tmaxTessellationEvaluationOutputComponents  " << deviceProperties.limits.maxTessellationEvaluationOutputComponents << std::endl;
    fout << "\tmaxGeometryShaderInvocations  " << deviceProperties.limits.maxGeometryShaderInvocations << std::endl;
    fout << "\tmaxGeometryInputComponents  " << deviceProperties.limits.maxGeometryInputComponents << std::endl;
    fout << "\tmaxGeometryOutputComponents  " << deviceProperties.limits.maxGeometryOutputComponents << std::endl;
    fout << "\tmaxGeometryOutputVertices  " << deviceProperties.limits.maxGeometryOutputVertices << std::endl;
    fout << "\tmaxGeometryTotalOutputComponents  " << deviceProperties.limits.maxGeometryTotalOutputComponents << std::endl;
    fout << "\tmaxFragmentInputComponents  " << deviceProperties.limits.maxFragmentInputComponents << std::endl;
    fout << "\tmaxFragmentOutputAttachments  " << deviceProperties.limits.maxFragmentOutputAttachments << std::endl;
    fout << "\tmaxFragmentDualSrcAttachments  " << deviceProperties.limits.maxFragmentDualSrcAttachments << std::endl;
    fout << "\tmaxFragmentCombinedOutputResources  " << deviceProperties.limits.maxFragmentCombinedOutputResources << std::endl;
    fout << "\tmaxComputeSharedMemorySize  " << deviceProperties.limits.maxComputeSharedMemorySize << std::endl;
    fout << "\tmaxComputeWorkGroupCount[0]  " << deviceProperties.limits.maxComputeWorkGroupCount[0] << std::endl;
    fout << "\tmaxComputeWorkGroupCount[1]  " << deviceProperties.limits.maxComputeWorkGroupCount[1] << std::endl;
    fout << "\tmaxComputeWorkGroupCount[2]  " << deviceProperties.limits.maxComputeWorkGroupCount[2] << std::endl;
    fout << "\tmaxComputeWorkGroupInvocations  " << deviceProperties.limits.maxComputeWorkGroupInvocations << std::endl;
    fout << "\tmaxComputeWorkGroupSize[0]  " << deviceProperties.limits.maxComputeWorkGroupSize[0] << std::endl;
    fout << "\tmaxComputeWorkGroupSize[1]  " << deviceProperties.limits.maxComputeWorkGroupSize[1] << std::endl;
    fout << "\tmaxComputeWorkGroupSize[2]  " << deviceProperties.limits.maxComputeWorkGroupSize[2] << std::endl;
    fout << "\tsubPixelPrecisionBits  " << deviceProperties.limits.subPixelPrecisionBits << std::endl;
    fout << "\tsubTexelPrecisionBits  " << deviceProperties.limits.subTexelPrecisionBits << std::endl;
    fout << "\tmipmapPrecisionBits  " << deviceProperties.limits.mipmapPrecisionBits << std::endl;
    fout << "\tmaxDrawIndexedIndexValue  " << deviceProperties.limits.maxDrawIndexedIndexValue << std::endl;
    fout << "\tmaxDrawIndirectCount  " << deviceProperties.limits.maxDrawIndirectCount << std::endl;
    fout << "\tmaxSamplerLodBias  " << deviceProperties.limits.maxSamplerLodBias << std::endl;
    fout << "\tmaxSamplerAnisotropy  " << deviceProperties.limits.maxSamplerAnisotropy << std::endl;
    fout << "\tmaxViewports  " << deviceProperties.limits.maxViewports << std::endl;
    fout << "\tmaxViewportDimensions[0]  " << deviceProperties.limits.maxViewportDimensions[0] << std::endl;
    fout << "\tmaxViewportDimensions[1]  " << deviceProperties.limits.maxViewportDimensions[1] << std::endl;
    fout << "\tviewportBoundsRange[0]  " << deviceProperties.limits.viewportBoundsRange[0] << std::endl;
    fout << "\tviewportBoundsRange[1]  " << deviceProperties.limits.viewportBoundsRange[1] << std::endl;
    fout << "\tviewportSubPixelBits  " << deviceProperties.limits.viewportSubPixelBits << std::endl;
    fout << "\tminMemoryMapAlignment  " << deviceProperties.limits.minMemoryMapAlignment << std::endl;
    fout << "\tminTexelBufferOffsetAlignment  " << deviceProperties.limits.minTexelBufferOffsetAlignment << std::endl;
    fout << "\tminUniformBufferOffsetAlignment  " << deviceProperties.limits.minUniformBufferOffsetAlignment << std::endl;
    fout << "\tminStorageBufferOffsetAlignment  " << deviceProperties.limits.minStorageBufferOffsetAlignment << std::endl;
    fout << "\tminTexelOffset  " << deviceProperties.limits.minTexelOffset << std::endl;
    fout << "\tmaxTexelOffset  " << deviceProperties.limits.maxTexelOffset << std::endl;
    fout << "\tminTexelGatherOffset  " << deviceProperties.limits.minTexelGatherOffset << std::endl;
    fout << "\tmaxTexelGatherOffset  " << deviceProperties.limits.maxTexelGatherOffset << std::endl;
    fout << "\tminInterpolationOffset  " << deviceProperties.limits.minInterpolationOffset << std::endl;
    fout << "\tmaxInterpolationOffset  " << deviceProperties.limits.maxInterpolationOffset << std::endl;
    fout << "\tsubPixelInterpolationOffsetBits  " << deviceProperties.limits.subPixelInterpolationOffsetBits << std::endl;
    fout << "\tmaxFramebufferWidth  " << deviceProperties.limits.maxFramebufferWidth << std::endl;
    fout << "\tmaxFramebufferHeight  " << deviceProperties.limits.maxFramebufferHeight << std::endl;
    fout << "\tmaxFramebufferLayers  " << deviceProperties.limits.maxFramebufferLayers << std::endl;
    fout << "\tframebufferColorSampleCounts  " << deviceProperties.limits.framebufferColorSampleCounts << std::endl;
    fout << "\tframebufferDepthSampleCounts  " << deviceProperties.limits.framebufferDepthSampleCounts << std::endl;
    fout << "\tframebufferStencilSampleCounts  " << deviceProperties.limits.framebufferStencilSampleCounts << std::endl;
    fout << "\tframebufferNoAttachmentsSampleCounts  " << deviceProperties.limits.framebufferNoAttachmentsSampleCounts << std::endl;
    fout << "\tmaxColorAttachments  " << deviceProperties.limits.maxColorAttachments << std::endl;
    fout << "\tsampledImageColorSampleCounts  " << deviceProperties.limits.sampledImageColorSampleCounts << std::endl;
    fout << "\tsampledImageIntegerSampleCounts  " << deviceProperties.limits.sampledImageIntegerSampleCounts << std::endl;
    fout << "\tsampledImageDepthSampleCounts  " << deviceProperties.limits.sampledImageDepthSampleCounts << std::endl;
    fout << "\tsampledImageStencilSampleCounts  " << deviceProperties.limits.sampledImageStencilSampleCounts << std::endl;
    fout << "\tstorageImageSampleCounts  " << deviceProperties.limits.storageImageSampleCounts << std::endl;
    fout << "\tmaxSampleMaskWords  " << deviceProperties.limits.maxSampleMaskWords << std::endl;
    fout << "\ttimestampComputeAndGraphics  " << deviceProperties.limits.timestampComputeAndGraphics << std::endl;
    fout << "\ttimestampPeriod  " << deviceProperties.limits.timestampPeriod << std::endl;
    fout << "\tmaxClipDistances  " << deviceProperties.limits.maxClipDistances << std::endl;
    fout << "\tmaxCullDistances  " << deviceProperties.limits.maxCullDistances << std::endl;
    fout << "\tmaxCombinedClipAndCullDistances  " << deviceProperties.limits.maxCombinedClipAndCullDistances << std::endl;
    fout << "\tdiscreteQueuePriorities  " << deviceProperties.limits.discreteQueuePriorities << std::endl;
    fout << "\tpointSizeRange[0]  " << deviceProperties.limits.pointSizeRange[0] << std::endl;
    fout << "\tpointSizeRange[1]  " << deviceProperties.limits.pointSizeRange[1] << std::endl;
    fout << "\tlineWidthRange[0]  " << deviceProperties.limits.lineWidthRange[0] << std::endl;
    fout << "\tlineWidthRange[1]  " << deviceProperties.limits.lineWidthRange[1] << std::endl;
    fout << "\tpointSizeGranularity  " << deviceProperties.limits.pointSizeGranularity << std::endl;
    fout << "\tlineWidthGranularity  " << deviceProperties.limits.lineWidthGranularity << std::endl;
    fout << "\tstrictLines  " << deviceProperties.limits.strictLines << std::endl;
    fout << "\tstandardSampleLocations  " << deviceProperties.limits.standardSampleLocations << std::endl;
    fout << "\toptimalBufferCopyOffsetAlignment  " << deviceProperties.limits.optimalBufferCopyOffsetAlignment << std::endl;
    fout << "\toptimalBufferCopyRowPitchAlignment  " << deviceProperties.limits.optimalBufferCopyRowPitchAlignment << std::endl;
    fout << "\tnonCoherentAtomSize  " << deviceProperties.limits.nonCoherentAtomSize << std::endl;
    fout << std::endl;
    fout << "Device features" << std::endl;
    fout << "\trobustBufferAccess     " << BOOL(deviceFeatures.robustBufferAccess) << std::endl;
    fout << "\tfullDrawIndexUint32     " << BOOL(deviceFeatures.fullDrawIndexUint32) << std::endl;
    fout << "\timageCubeArray     " << BOOL(deviceFeatures.imageCubeArray) << std::endl;
    fout << "\tindependentBlend     " << BOOL(deviceFeatures.independentBlend) << std::endl;
    fout << "\tgeometryShader     " << BOOL(deviceFeatures.geometryShader) << std::endl;
    fout << "\ttessellationShader     " << BOOL(deviceFeatures.tessellationShader) << std::endl;
    fout << "\tsampleRateShading     " << BOOL(deviceFeatures.sampleRateShading) << std::endl;
    fout << "\tdualSrcBlend     " << BOOL(deviceFeatures.dualSrcBlend) << std::endl;
    fout << "\tlogicOp     " << BOOL(deviceFeatures.logicOp) << std::endl;
    fout << "\tmultiDrawIndirect     " << BOOL(deviceFeatures.multiDrawIndirect) << std::endl;
    fout << "\tdrawIndirectFirstInstance     " << BOOL(deviceFeatures.drawIndirectFirstInstance) << std::endl;
    fout << "\tdepthClamp     " << BOOL(deviceFeatures.depthClamp) << std::endl;
    fout << "\tdepthBiasClamp     " << BOOL(deviceFeatures.depthBiasClamp) << std::endl;
    fout << "\tfillModeNonSolid     " << BOOL(deviceFeatures.fillModeNonSolid) << std::endl;
    fout << "\tdepthBounds     " << BOOL(deviceFeatures.depthBounds) << std::endl;
    fout << "\twideLines     " << BOOL(deviceFeatures.wideLines) << std::endl;
    fout << "\tlargePoints     " << BOOL(deviceFeatures.largePoints) << std::endl;
    fout << "\talphaToOne     " << BOOL(deviceFeatures.alphaToOne) << std::endl;
    fout << "\tmultiViewport     " << BOOL(deviceFeatures.multiViewport) << std::endl;
    fout << "\tsamplerAnisotropy     " << BOOL(deviceFeatures.samplerAnisotropy) << std::endl;
    fout << "\ttextureCompressionETC2     " << BOOL(deviceFeatures.textureCompressionETC2) << std::endl;
    fout << "\ttextureCompressionASTC_LDR     " << BOOL(deviceFeatures.textureCompressionASTC_LDR) << std::endl;
    fout << "\ttextureCompressionBC     " << BOOL(deviceFeatures.textureCompressionBC) << std::endl;
    fout << "\tocclusionQueryPrecise     " << BOOL(deviceFeatures.occlusionQueryPrecise) << std::endl;
    fout << "\tpipelineStatisticsQuery     " << BOOL(deviceFeatures.pipelineStatisticsQuery) << std::endl;
    fout << "\tvertexPipelineStoresAndAtomics     " << BOOL(deviceFeatures.vertexPipelineStoresAndAtomics) << std::endl;
    fout << "\tfragmentStoresAndAtomics     " << BOOL(deviceFeatures.fragmentStoresAndAtomics) << std::endl;
    fout << "\tshaderTessellationAndGeometryPointSize     " << BOOL(deviceFeatures.shaderTessellationAndGeometryPointSize) << std::endl;
    fout << "\tshaderImageGatherExtended     " << BOOL(deviceFeatures.shaderImageGatherExtended) << std::endl;
    fout << "\tshaderStorageImageExtendedFormats     " << BOOL(deviceFeatures.shaderStorageImageExtendedFormats) << std::endl;
    fout << "\tshaderStorageImageMultisample     " << BOOL(deviceFeatures.shaderStorageImageMultisample) << std::endl;
    fout << "\tshaderStorageImageReadWithoutFormat     " << BOOL(deviceFeatures.shaderStorageImageReadWithoutFormat) << std::endl;
    fout << "\tshaderStorageImageWriteWithoutFormat     " << BOOL(deviceFeatures.shaderStorageImageWriteWithoutFormat) << std::endl;
    fout << "\tshaderUniformBufferArrayDynamicIndexing     " << BOOL(deviceFeatures.shaderUniformBufferArrayDynamicIndexing) << std::endl;
    fout << "\tshaderSampledImageArrayDynamicIndexing     " << BOOL(deviceFeatures.shaderSampledImageArrayDynamicIndexing) << std::endl;
    fout << "\tshaderStorageBufferArrayDynamicIndexing     " << BOOL(deviceFeatures.shaderStorageBufferArrayDynamicIndexing) << std::endl;
    fout << "\tshaderStorageImageArrayDynamicIndexing     " << BOOL(deviceFeatures.shaderStorageImageArrayDynamicIndexing) << std::endl;
    fout << "\tshaderClipDistance     " << BOOL(deviceFeatures.shaderClipDistance) << std::endl;
    fout << "\tshaderCullDistance     " << BOOL(deviceFeatures.shaderCullDistance) << std::endl;
    fout << "\tshaderFloat64     " << BOOL(deviceFeatures.shaderFloat64) << std::endl;
    fout << "\tshaderInt64     " << BOOL(deviceFeatures.shaderInt64) << std::endl;
    fout << "\tshaderInt16     " << BOOL(deviceFeatures.shaderInt16) << std::endl;
    fout << "\tshaderResourceResidency     " << BOOL(deviceFeatures.shaderResourceResidency) << std::endl;
    fout << "\tshaderResourceMinLod     " << BOOL(deviceFeatures.shaderResourceMinLod) << std::endl;
    fout << "\tsparseBinding     " << BOOL(deviceFeatures.sparseBinding) << std::endl;
    fout << "\tsparseResidencyBuffer     " << BOOL(deviceFeatures.sparseResidencyBuffer) << std::endl;
    fout << "\tsparseResidencyImage2D     " << BOOL(deviceFeatures.sparseResidencyImage2D) << std::endl;
    fout << "\tsparseResidencyImage3D     " << BOOL(deviceFeatures.sparseResidencyImage3D) << std::endl;
    fout << "\tsparseResidency2Samples     " << BOOL(deviceFeatures.sparseResidency2Samples) << std::endl;
    fout << "\tsparseResidency4Samples     " << BOOL(deviceFeatures.sparseResidency4Samples) << std::endl;
    fout << "\tsparseResidency8Samples     " << BOOL(deviceFeatures.sparseResidency8Samples) << std::endl;
    fout << "\tsparseResidency16Samples     " << BOOL(deviceFeatures.sparseResidency16Samples) << std::endl;
    fout << "\tsparseResidencyAliased     " << BOOL(deviceFeatures.sparseResidencyAliased) << std::endl;
    fout << "\tvariableMultisampleRate     " << BOOL(deviceFeatures.variableMultisampleRate) << std::endl;
    fout << "\tinheritedQueries     " << BOOL(deviceFeatures.inheritedQueries) << std::endl;
    fout << std::endl;
    fout << "Device memory" << std::endl;
    for (uint32_t i = 0; i < deviceMemories.memoryTypeCount; i++)
    {
        fout << "\tmemorytype[" << i << "].heapIndex     " << deviceMemories.memoryTypes[i].heapIndex << std::endl;
        fout << "\tmemorytype[" << i << "] - DEVICE LOCAL     " << BOOL(deviceMemories.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) << std::endl;
        fout << "\tmemorytype[" << i << "] - HOST VISIBLE     " << BOOL(deviceMemories.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) << std::endl;
        fout << "\tmemorytype[" << i << "] - HOST COHERENT    " << BOOL(deviceMemories.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) << std::endl;
        fout << "\tmemorytype[" << i << "] - HOST CACHED      " << BOOL(deviceMemories.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) << std::endl;
        fout << "\tmemorytype[" << i << "] - LAZILY ALLOCATED " << BOOL(deviceMemories.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) << std::endl;
        fout << "\tmemorytype[" << i << "] - PROTECTED        " << BOOL(deviceMemories.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) << std::endl;
    }
    for (uint32_t i = 0; i < deviceMemories.memoryHeapCount; i++)
    {
        fout << "\tmemoryheap[" << i << "].size     " << deviceMemories.memoryHeaps[i].size << std::endl;
        fout << "\tmemoryheap[" << i << "] - DEVICE LOCAL     " << BOOL(deviceMemories.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) << std::endl;
        fout << "\tmemoryheap[" << i << "] - MULTI INSTANCE   " << BOOL(deviceMemories.memoryHeaps[i].flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT) << std::endl;
    }
    fout << std::endl;
    fout << "Device queue families" << std::endl;
    for (uint32_t i = 0; i < queueFamilyCount; i++)
    {
        fout << "\tqueuefamily[" << i << "].queueCount     " << queueProperties[i].queueCount << std::endl;
        fout << "\tqueuefamily[" << i << "] - GRAPHICS     " << BOOL(queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) << std::endl;
        fout << "\tqueuefamily[" << i << "] - COMPUTE      " << BOOL(queueProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) << std::endl;
        fout << "\tqueuefamily[" << i << "] - TRANSFER     " << BOOL(queueProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) << std::endl;
        fout << "\tqueuefamily[" << i << "] - SPARSE BINDING " << BOOL(queueProperties[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) << std::endl;
        fout << "\tqueuefamily[" << i << "] - PROTECTED    " << BOOL(queueProperties[i].queueFlags & VK_QUEUE_PROTECTED_BIT) << std::endl;
    }
    fout.close();
}

