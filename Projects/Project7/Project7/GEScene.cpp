#include "GEScene.h"

#include "GECylinder.h"
#include "GESphere.h"
#include "GEGround.h"
#include "GEBalljoint.h"
#include "GETransform.h"
#include "GEMaterial.h"
#include "GELight.h"
#include <windows.h>
#include "resource.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>


//
// FUNCIÓN: GEScene::GEScene(GEGraphicsContext* gc, GEDrawingContext* dc)
//
// PROPÓSITO: Crea la escena
//
GEScene::GEScene(GEGraphicsContext* gc, GEDrawingContext* dc, GECommandContext* cc)
{
	VkExtent2D extent = dc->getExtent();
	double aspect = (double)extent.width / (double)extent.height;
	aspect_ratio(aspect);

	GEPipelineConfig* config = createPipelineConfig(dc->getExtent());
	rc = new GERenderingContext(gc, dc, config);

	this->camera = new GECamera();
	this->camera->setPosition(glm::vec3(0.0f, 1.0f, 10.0f));
	this->camera->setMoveStep(0.0f);

	GELight light = {};
	light.Ldir = glm::normalize(glm::vec3(1.0f, -0.8f, -0.7f));
	light.La = glm::vec3(0.2f, 0.2f, 0.2f);
	light.Ld = glm::vec3(0.8f, 0.8f, 0.8f);
	light.Ls = glm::vec3(1.0f, 1.0f, 1.0f);

	GEMaterial groundMat = {};
	groundMat.Ka = glm::vec3(0.0f, 0.3f, 0.0f);
	groundMat.Kd = glm::vec3(0.0f, 0.3f, 0.0f);
	groundMat.Ks = glm::vec3(0.8f, 0.8f, 0.8f);
	groundMat.Shininess = 16.0f;

	ground = new GEGround(5.0f, 5.0f);
	ground->initialize(gc, rc);
	ground->setLight(light);
	ground->setMaterial(groundMat);


	// Creamos el esqueleto humano
	testSkeleton = new GESkeleton("body");

	// Construimos las 19 articulaciones
	testSkeleton->buildBody();


	// Inicializamos las piezas graficas
	testSkeleton->initialize(gc, rc);

	// Aplicamos la iluminacion
	testSkeleton->setLight(light);


	animation = new GEAnimation(testSkeleton);



	//
	// LANZAMIENTO DE PESO: PRUEBA DE PREPARACION
	//

	// HOMBRO DERECHO

	// Inicio: brazo recogido
	animation->addKeyframe(
		"shoulder_r", 0.0f,
		glm::vec3(-45.0f, -30.0f, -175.0f)
	);

	// Mantiene la carga hasta la posicion de fuerza
	animation->addKeyframe(
		"shoulder_r", 3.2f,
		glm::vec3(-45.0f, -30.0f, -175.0f)
	);

	// Extension durante el lanzamiento
	animation->addKeyframe(
		"shoulder_r", 3.8f,
		glm::vec3(-10.0f, -10.0f, -60.0f)
	);

	// Recuperacion
	animation->addKeyframe(
		"shoulder_r", 5.0f,
		glm::vec3(-15.0f, -10.0f, -30.0f)
	);


	// CODO DERECHO

	// Inicio: codo flexionado
	animation->addKeyframe(
		"elbow_r", 0.0f,
		glm::vec3(-160.0f, 0.0f, 0.0f)
	);

	// Conservamos la flexion durante la preparacion
	animation->addKeyframe(
		"elbow_r", 3.2f,
		glm::vec3(-160.0f, 0.0f, 0.0f)
	);

	// Empuje y extension
	animation->addKeyframe(
		"elbow_r", 3.8f,
		glm::vec3(0.0f, 0.0f, 0.0f)
	);

	// Recuperacion
	animation->addKeyframe(
		"elbow_r", 5.0f,
		glm::vec3(-15.0f, 0.0f, 0.0f)
	);




	// Guardamos el instante de inicio
	animationStartTime = glfwGetTime();


	fillCommandBuffers(cc->commandBuffers);
}

//
// FUNCIÓN: GEScene::destroy(GEGraphicsContext* gc)
//
// PROPÓSITO: Reconstruye los componentes gráficos de la escena
//
void GEScene::destroy(GEGraphicsContext* gc)
{
	rc->destroy(gc);
	delete rc;

	ground->destroy(gc);
	delete animation;
	testSkeleton->destroy(gc);

	delete ground;
	delete testSkeleton;
}

//
// FUNCIÓN: GEScene::recreate(GEGraphicsContext* gc, GEDrawingContext* dc)
//
// PROPÓSITO: Reconstruye los componentes gráficos de la escena
//
void GEScene::recreate(GEGraphicsContext* gc, GEDrawingContext* dc, GECommandContext* cc)
{
	rc->destroy(gc);
	GEPipelineConfig* config = createPipelineConfig(dc->getExtent());
	this->rc = new GERenderingContext(gc, dc, config);
	fillCommandBuffers(cc->commandBuffers);
}

//
// FUNCIÓN: GEScene::update(GEGraphicsContext* gc, uint32_t index)
//
// PROPÓSITO: Actualiza la información para generar la imagen 
//

void GEScene::update(GEGraphicsContext* gc, uint32_t index)
{
	camera->update();

	glm::mat4 view = camera->getViewMatrix();

	// Tiempo transcurrido desde el inicio de la animacion
	double now = glfwGetTime();
	double elapsed = now - animationStartTime;

	// Repetimos la prueba cada 5 segundos
	if (elapsed >= 5.0)
	{
		//animationStartTime = now;
		elapsed = 5.0;
	}

	// Calculamos y aplicamos los angulos interpolados
	animation->update(static_cast<float>(0.0f));

	// Actualizamos los objetos graficos
	ground->update(gc, index, view, projection);
	testSkeleton->update(gc, index, view, projection);
}


//
// FUNCIÓN: GEScene::key_action(int key, bool pressed)
//
// PROPÓSITO: Respuesta a acciones de teclado
//
void GEScene::key_action(int key, bool pressed)
{
	switch (key)
	{
	case GLFW_KEY_UP:
		camera->setTurnDown(pressed);
		break;
	case GLFW_KEY_DOWN:
		camera->setTurnUp(pressed);
		break;
	case GLFW_KEY_LEFT:
		camera->setTurnCCW(pressed);
		break;
	case GLFW_KEY_RIGHT:
		camera->setTurnCW(pressed);
		break;
	case GLFW_KEY_S:
		camera->setMoveStep(0.0f);
		break;
	case GLFW_KEY_KP_ADD:
	case GLFW_KEY_1:
		camera->setMoveStep(camera->getMoveStep() + 0.1f);
		break;
	case GLFW_KEY_KP_SUBTRACT:
	case GLFW_KEY_2:
		camera->setMoveStep(camera->getMoveStep() - 0.1f);
		break;
	case GLFW_KEY_Q:
		camera->setMoveUp(pressed);
		break;
	case GLFW_KEY_A:
		camera->setMoveDown(pressed);
		break;
	case GLFW_KEY_O:
		camera->setMoveLeft(pressed);
		break;
	case GLFW_KEY_P:
		camera->setMoveRight(pressed);
		break;
	case GLFW_KEY_K:
		camera->setTurnLeft(pressed);
		break;
	case GLFW_KEY_L:
		camera->setTurnRight(pressed);
		break;
	}
}

//
// FUNCIÓN: GEScene::aspect_ratio(double)
//
// PROPÓSITO: Modifica la relación anchura/altura del modelo
//
void GEScene::aspect_ratio(double aspect)
{
	constexpr double fov = glm::radians(30.0f);
	double sin_fov = sin(fov);
	double cos_fov = cos(fov);
	float wHeight = (float)(sin_fov * 0.2 / cos_fov);
	float wWidth = (float)(wHeight * aspect);

	projection = glm::perspective((float)fov, (float)aspect, 0.2f, 400.0f);
	projection[1][1] *= -1.0f;
}

//
// FUNCIÓN: GEScene::createPipelineConfig()
//
// PROPÓSITO: Obtiene la configuración del pipeline de renderizado
//
GEPipelineConfig* GEScene::createPipelineConfig(VkExtent2D extent)
{
	GEPipelineConfig* config = new GEPipelineConfig();
	config->vertex_shader = IDR_HTML1;
	config->fragment_shader = IDR_HTML2;

	config->attrStride = sizeof(GEVertex);
	config->attrOffsets.resize(2);
	config->attrOffsets[0] = offsetof(GEVertex, pos);
	config->attrOffsets[1] = offsetof(GEVertex, norm);
	config->attrFormats.resize(2);
	config->attrFormats[0] = VK_FORMAT_R32G32B32_SFLOAT;
	config->attrFormats[1] = VK_FORMAT_R32G32B32_SFLOAT;

	config->descriptorTypes.resize(3);
	config->descriptorTypes[0] = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	config->descriptorTypes[1] = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	config->descriptorTypes[2] = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	config->descriptorStages.resize(3);
	config->descriptorStages[0] = VK_SHADER_STAGE_ALL_GRAPHICS;
	config->descriptorStages[1] = VK_SHADER_STAGE_FRAGMENT_BIT;
	config->descriptorStages[2] = VK_SHADER_STAGE_FRAGMENT_BIT;

	config->depthTestEnable = VK_TRUE;
	config->cullMode = VK_CULL_MODE_BACK_BIT;
	config->extent = extent;

	return config;
}

//
// FUNCIÓN: GEScene::fillCommandBuffers(std::vector<VkCommandBuffer> commandBuffers)
//
// PROPÓSITO: Rellena los buffers de comandos
//
void GEScene::fillCommandBuffers(std::vector<VkCommandBuffer> commandBuffers)
{
	rc->startFillingCommandBuffers(commandBuffers);

	for (size_t i = 0; i < commandBuffers.size(); i++)
	{
		ground->addCommands(
			commandBuffers[i],
			rc->pipelineLayout,
			i
		);

		testSkeleton->addCommands(
			commandBuffers[i],
			rc->pipelineLayout,
			i
		);
	}

	rc->endFillingCommandBuffers(commandBuffers);
}