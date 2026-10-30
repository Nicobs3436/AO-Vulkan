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

	// Altura para que los pies apoyen en el suelo
	testSkeleton->setOffset(glm::vec3(0.0f, 1.038f, 0.0f));

	animation = new GEAnimation(testSkeleton);

	// Tronco ligeramente inclinado y cabeza erguida
	animation->addKeyframe("spine", 0.0f, glm::vec3(-5.0f, 0.0f, 0.0f));
	animation->addKeyframe("neck", 0.0f, glm::vec3(5.0f, 0.0f, 0.0f));

	// Brazo derecho: mano junto al cuello
	animation->addKeyframe("shoulder_r", 0.0f, glm::vec3(-15.0f, 15.0f, -10.0f));
	animation->addKeyframe("elbow_r", 0.0f, glm::vec3(-160.0f, 0.0f, 0.0f));
	animation->addKeyframe("wrist_r", 0.0f, glm::vec3(35.0f, 15.0f, -10.0f));

	// Brazo izquierdo junto al cuerpo
	animation->addKeyframe("shoulder_l", 0.0f, glm::vec3(80.0f, -10.0f, 0.0f));
	animation->addKeyframe("elbow_l", 0.0f, glm::vec3(5.0f, 0.0f, 0.0f));

	// Pierna derecha
	animation->addKeyframe("leg_r", 0.0f, glm::vec3(-5.0f, 8.0f, 0.0f));
	animation->addKeyframe("knee_r", 0.0f, glm::vec3(10.0f, 0.0f, 0.0f));
	animation->addKeyframe("ankle_r", 0.0f, glm::vec3(-5.0f, 0.0f, -8.0f));

	// Pierna izquierda
	animation->addKeyframe("leg_l", 0.0f, glm::vec3(-5.0f, -8.0f, 0.0f));
	animation->addKeyframe("knee_l", 0.0f, glm::vec3(10.0f, 0.0f, 0.0f));
	animation->addKeyframe("ankle_l", 0.0f, glm::vec3(-5.0f, 0.0f, 8.0f));


	// La pelvis comienza sin inclinacion
	animation->addKeyframe("pelvis", 0.0f, glm::vec3(0.0f, 0.0f, 0.0f));

	// Agrupamiento: 1.2 segundos
	animation->addKeyframe("pelvis", 1.2f, glm::vec3(-35.0f, 0.0f, 0.0f));
	animation->addKeyframe("spine", 1.2f, glm::vec3(-30.0f, 0.0f, 0.0f));

	// Brazo izquierdo: queda colgando al inclinar el tronco
	animation->addKeyframe("shoulder_l", 1.2f, glm::vec3(70.0f, -35.0f, -60.0f));

	// Pierna derecha: apoyo durante el agrupamiento
	animation->addKeyframe("leg_r", 1.2f, glm::vec3(-50.0f, 22.0f, 0.0f));
	animation->addKeyframe("knee_r", 1.2f, glm::vec3(100.0f, 0.0f, 0.0f));
	animation->addKeyframe("ankle_r", 1.2f, glm::vec3(-48.0f, -17.0f, -15.0f));

	// Pierna izquierda: se recoge junto a la derecha
	animation->addKeyframe("leg_l", 1.2f, glm::vec3(-65.0f, -25.0f, 0.0f));
	animation->addKeyframe("knee_l", 1.2f, glm::vec3(110.0f, 0.0f, 0.0f));
	animation->addKeyframe("ankle_l", 1.2f, glm::vec3(-17.0f, 17.0f, 18.0f));

	// Inicio del impulso: pierna derecha de apoyo
	animation->addKeyframe("leg_r", 1.6f, glm::vec3(-65.0f, 22.0f, 0.0f));
	animation->addKeyframe("knee_r", 1.6f, glm::vec3(80.0f, 0.0f, 0.0f));
	animation->addKeyframe("ankle_r", 1.6f, glm::vec3(-14.0f, -6.0f, -21.0f));

	// La pierna izquierda se dirige hacia el lanzamiento
	animation->addKeyframe("leg_l", 1.6f, glm::vec3(65.0f, -10.0f, 0.0f));

	// Primero conserva la flexion y despues extiende la rodilla
	animation->addKeyframe("knee_l", 1.4f, glm::vec3(115.0f, 0.0f, 0.0f));
	animation->addKeyframe("knee_l", 1.6f, glm::vec3(10.0f, 0.0f, 0.0f));

	// Orientacion del pie libre durante el movimiento
	animation->addKeyframe("ankle_l", 1.4f, glm::vec3(-76.0f, 16.0f, -8.0f));
	animation->addKeyframe("ankle_l", 1.6f, glm::vec3(-35.0f, 10.0f, 3.0f));


	// 1.72 s: termina el impulso del pie derecho
	animation->addKeyframe("pelvis", 1.72f, glm::vec3(-35.0f, 0.0f, 0.0f));
	animation->addKeyframe("spine", 1.72f, glm::vec3(-30.0f, 0.0f, 0.0f));
	animation->addKeyframe("shoulder_l", 1.72f, glm::vec3(70.0f, -35.0f, -60.0f));

	animation->addKeyframe("leg_r", 1.72f, glm::vec3(-50.0f, 20.0f, 0.0f));
	animation->addKeyframe("knee_r", 1.72f, glm::vec3(35.0f, 0.0f, 0.0f));
	animation->addKeyframe("ankle_r", 1.72f, glm::vec3(39.0f, 5.0f, -19.0f));

	animation->addKeyframe("knee_l", 1.72f, glm::vec3(80.0f, 0.0f, 0.0f));
	animation->addKeyframe("ankle_l", 1.72f, glm::vec3(-56.0f, 2.0f, 0.0f));

	// Recogemos los pies durante el avance
	animation->addKeyframe("ankle_r", 1.76f, glm::vec3(11.0f, 0.0f, -19.0f));
	animation->addKeyframe("ankle_l", 1.80f, glm::vec3(-53.0f, -2.0f, -2.0f));

	// 1.88 s: avance con las piernas recogidas
	animation->addKeyframe("leg_r", 1.88f, glm::vec3(-45.0f, 15.0f, 0.0f));
	animation->addKeyframe("knee_r", 1.88f, glm::vec3(85.0f, 0.0f, 0.0f));
	animation->addKeyframe("ankle_r", 1.88f, glm::vec3(-24.0f, -10.0f, -12.0f));

	animation->addKeyframe("leg_l", 1.88f, glm::vec3(-50.0f, 8.0f, -80.0f));
	animation->addKeyframe("knee_l", 1.88f, glm::vec3(70.0f, 0.0f, 0.0f));
	animation->addKeyframe("ankle_l", 1.88f, glm::vec3(-10.0f, -3.0f, -8.0f));

	// 2.05 s: llega primero el pie derecho
	animation->addKeyframe("pelvis", 2.05f, glm::vec3(-25.0f, 0.0f, -90.0f));
	animation->addKeyframe("spine", 2.05f, glm::vec3(-25.0f, 0.0f, 0.0f));
	animation->addKeyframe("shoulder_l", 2.05f, glm::vec3(70.0f, -25.0f, 40.0f));

	animation->addKeyframe("leg_r", 2.05f, glm::vec3(-40.0f, 8.0f, 0.0f));
	animation->addKeyframe("knee_r", 2.05f, glm::vec3(80.0f, 0.0f, 0.0f));
	animation->addKeyframe("ankle_r", 2.05f, glm::vec3(-40.0f, -5.0f, -6.0f));

	animation->addKeyframe("leg_l", 2.05f, glm::vec3(-50.0f, 8.0f, -90.0f));
	animation->addKeyframe("knee_l", 2.05f, glm::vec3(20.0f, 0.0f, 0.0f));
	animation->addKeyframe("ankle_l", 2.05f, glm::vec3(30.0f, 4.0f, -7.0f));

	// 2.20 s: termina de colocarse el pie izquierdo
	animation->addKeyframe("pelvis", 2.20f, glm::vec3(-20.0f, 0.0f, -90.0f));
	animation->addKeyframe("leg_l", 2.20f, glm::vec3(-45.0f, 8.0f, -90.0f));
	animation->addKeyframe("knee_l", 2.20f, glm::vec3(12.0f, 0.0f, 0.0f));
	animation->addKeyframe("ankle_l", 2.20f, glm::vec3(33.0f, 4.0f, -7.0f));

	animation->update(0.0f);
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

	float time = static_cast<float>(glfwGetTime() - animationStartTime);
	time = glm::clamp(time, 0.0f, 2.2f);

	animation->update(time);

	// Preparacion: 0.0 a 1.2 segundos
	float t = glm::clamp(time / 1.2f, 0.0f, 1.0f);

	float muslo = glm::radians(-5.0f - 45.0f * t);
	float rodilla = glm::radians(10.0f + 90.0f * t);
	float lateral = glm::radians(8.0f + 14.0f * t);

	// Extension de la pierna izquierda: 1.2 a 1.6 segundos
	if (time > 1.2f)
	{
		float avance = glm::clamp(
			(time - 1.2f) / 0.4f, 0.0f, 1.0f
		);

		muslo = glm::radians(-50.0f - 15.0f * avance);
		rodilla = glm::radians(100.0f - 20.0f * avance);
		lateral = glm::radians(22.0f);
	}

	float apoyoY = 0.1002f;
	float apoyoZ = 0.0087f;

	// Impulso: el talon derecho se levanta sobre la parte delantera del pie
	if (time > 1.6f)
	{
		float avance = glm::clamp(
			(time - 1.6f) / 0.12f, 0.0f, 1.0f
		);

		muslo = glm::radians(-65.0f + 15.0f * avance);
		rodilla = glm::radians(80.0f - 45.0f * avance);
		lateral = glm::radians(22.0f - 2.0f * avance);

		float pie = glm::radians(25.0f * avance);

		apoyoY = 0.05f * (float)cos(pie)
			+ 0.25f * (float)sin(pie);

		if (apoyoY < 0.1002f)
		{
			apoyoY = 0.1002f;
		}

		apoyoZ = 0.2587f - 0.25f * (float)cos(pie);
	}

	float alturaPierna =
		0.5f * (float)cos(muslo) +
		0.4f * (float)cos(muslo + rodilla);

	glm::vec3 posicion(
		0.1248f - alturaPierna * (float)sin(lateral),
		apoyoY + 0.05f + alturaPierna * (float)cos(lateral),
		apoyoZ + 0.5f * (float)sin(muslo)
		+ 0.4f * (float)sin(muslo + rodilla)
	);

	float giro = 0.0f;

	// Tras el impulso desplazamos el cuerpo hacia el lanzamiento
	if (time > 1.72f)
	{
		float avanceGiro = glm::clamp(
			(time - 1.72f) / 0.33f, 0.0f, 1.0f
		);

		giro = glm::radians(90.0f * avanceGiro);

		glm::vec3 salida(-0.1173f, 0.8661f, -0.4544f);
		glm::vec3 intermedia(-0.0600f, 0.9100f, -0.7000f);
		glm::vec3 llegada(0.0000f, 0.8330f, -0.9540f);

		if (time <= 1.88f)
		{
			float avance = (time - 1.72f) / 0.16f;
			posicion = (1.0f - avance) * salida
				+ avance * intermedia;
		}
		else
		{
			float avance = glm::clamp(
				(time - 1.88f) / 0.17f, 0.0f, 1.0f
			);

			posicion = (1.0f - avance) * intermedia
				+ avance * llegada;
		}
	}

	testSkeleton->setOffset(posicion);

	testSkeleton->setOrientation(
		glm::vec3((float)sin(giro), 0.0f, (float)cos(giro)),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	

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
	case GLFW_KEY_R:
		if (pressed)
		{
			animationStartTime = glfwGetTime();
		}
		break;
	case GLFW_KEY_F1:
		if (pressed)
		{
			testSkeleton->resetLocation();
		}
		break;

	case GLFW_KEY_F2:
		if (pressed)
		{
			testSkeleton->resetLocation();
			testSkeleton->rotate(90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		break;
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