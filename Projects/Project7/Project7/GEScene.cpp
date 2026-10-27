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

	GEMaterial blueMat = {};
	blueMat.Ka = glm::vec3(0.0f, 0.0f, 0.8f);
	blueMat.Kd = glm::vec3(0.0f, 0.0f, 0.8f);
	blueMat.Ks = glm::vec3(0.8f, 0.8f, 0.8f);
	blueMat.Shininess = 16.0f;

	column = new GECylinder(2, 10, 0.05f, 0.75f);
	column->initialize(gc, rc);
	column->translate(glm::vec3(0.0f, 0.75f, 0.0f));
	column->rotate(-90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	column->setLight(light);
	column->setMaterial(blueMat);

	cross = new GECylinder(2, 10, 0.05f, 0.25f);
	cross->initialize(gc, rc);
	cross->translate(glm::vec3(0.0f, 1.25f, 0.0f));
	cross->rotate(-90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	cross->setLight(light);
	cross->setMaterial(blueMat);

	armL = new GEBalljoint(0.4f);
	armL->initialize(gc, rc);
	armL->setLocation(glm::vec3(0.25f, 1.25f, 0.0f));
	armL->setOrientation(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	armL->setLight(light);

	armR = new GEBalljoint(0.4f);
	armR->initialize(gc, rc);
	armR->setLocation(glm::vec3(-0.25f, 1.25f, 0.0f));
	armR->setOrientation(glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	armR->setLight(light);


	//
	// PRUEBA DE CONSTRUCCION DEL ESQUELETO HUMANO
	// Rama pelvis -> spine -> neck
	//

	// Creamos el esqueleto
	testSkeleton = new GESkeleton("body");

	// Posicion y orientacion indicadas en body.skel
	testSkeleton->setOffset(glm::vec3(0.0f, 1.0f, 0.0f));
	testSkeleton->setOrientation(
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	// ARTICULACION: pelvis
	testRoot = new GEBalljoint(0.3f);
	testRoot->setName("pelvis");
	testRoot->setLocation(glm::vec3(0.0f, 0.0f, 0.0f));
	testRoot->setOrientation(
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);

	// ARTICULACION: spine
	GEBalljoint* spine = new GEBalljoint(0.4f);
	spine->setName("spine");
	spine->setLocation(glm::vec3(0.0f, 0.0f, 0.0f));
	spine->setOrientation(
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	// ARTICULACION: neck
	GEBalljoint* neck = new GEBalljoint(0.35f);
	neck->setName("neck");
	neck->setLocation(glm::vec3(0.0f, 0.0f, 0.0f));
	neck->setOrientation(
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	// Construimos la jerarquia
	testRoot->addChild(spine);
	spine->addChild(neck);


	//
	// ARTICULACION: clavicle_l
	//
	GEBalljoint* clavicleL = new GEBalljoint(0.25f);
	clavicleL->setName("clavicle_l");
	clavicleL->setLocation(glm::vec3(-0.05f, 0.0f, -0.05f));
	clavicleL->setOrientation(
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	//
	// ARTICULACION: clavicle_r
	//
	GEBalljoint* clavicleR = new GEBalljoint(0.25f);
	clavicleR->setName("clavicle_r");
	clavicleR->setLocation(glm::vec3(0.05f, 0.0f, -0.05f));
	clavicleR->setOrientation(
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	// Conectamos las claviculas a la columna
	spine->addChild(clavicleL);
	spine->addChild(clavicleR);


	//
	// ARTICULACION: shoulder_l
	//
	GEBalljoint* shoulderL = new GEBalljoint(0.35f);
	shoulderL->setName("shoulder_l");
	shoulderL->setLocation(glm::vec3(0.0f, 0.0f, 0.0f));
	shoulderL->setOrientation(
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(1.0f, 0.0f, 0.0f)
	);

	//
	// ARTICULACION: shoulder_r
	//
	GEBalljoint* shoulderR = new GEBalljoint(0.35f);
	shoulderR->setName("shoulder_r");
	shoulderR->setLocation(glm::vec3(0.0f, 0.0f, 0.0f));
	shoulderR->setOrientation(
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f)
	);

	// Conectamos cada hombro a su clavicula
	clavicleL->addChild(shoulderL);
	clavicleR->addChild(shoulderR);



	//
	// ARTICULACION: elbow_l
	//
	GEBalljoint* elbowL = new GEBalljoint(0.30f);
	elbowL->setName("elbow_l");
	elbowL->setLocation(glm::vec3(0.0f, 0.0f, 0.0f));
	elbowL->setOrientation(
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	//
	// ARTICULACION: wrist_l
	//
	GEBalljoint* wristL = new GEBalljoint(0.20f);
	wristL->setName("wrist_l");
	wristL->setLocation(glm::vec3(0.0f, 0.0f, 0.0f));
	wristL->setOrientation(
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	//
	// ARTICULACION: elbow_r
	//
	GEBalljoint* elbowR = new GEBalljoint(0.30f);
	elbowR->setName("elbow_r");
	elbowR->setLocation(glm::vec3(0.0f, 0.0f, 0.0f));
	elbowR->setOrientation(
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	//
	// ARTICULACION: wrist_r
	//
	GEBalljoint* wristR = new GEBalljoint(0.20f);
	wristR->setName("wrist_r");
	wristR->setLocation(glm::vec3(0.0f, 0.0f, 0.0f));
	wristR->setOrientation(
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	// Construimos las relaciones padre-hijo
	shoulderL->addChild(elbowL);
	elbowL->addChild(wristL);

	shoulderR->addChild(elbowR);
	elbowR->addChild(wristR);



	testSkeleton->addRootJoint(testRoot);



	//
	// PIERNA IZQUIERDA
	//

	// ARTICULACION: hip_l
	GEBalljoint* hipL = new GEBalljoint(0.2f);
	hipL->setName("hip_l");
	hipL->setLocation(glm::vec3(0.05f, -0.05f, 0.0f));
	hipL->setOrientation(
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	// ARTICULACION: leg_l
	GEBalljoint* legL = new GEBalljoint(0.5f);
	legL->setName("leg_l");
	legL->setLocation(glm::vec3(0.0f, 0.0f, 0.0f));
	legL->setOrientation(
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f)
	);

	// ARTICULACION: knee_l
	GEBalljoint* kneeL = new GEBalljoint(0.4f);
	kneeL->setName("knee_l");
	kneeL->setLocation(glm::vec3(0.0f, 0.0f, 0.0f));
	kneeL->setOrientation(
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	// ARTICULACION: ankle_l
	GEBalljoint* ankleL = new GEBalljoint(0.25f);
	ankleL->setName("ankle_l");
	ankleL->setLocation(glm::vec3(0.0f, 0.0f, 0.0f));
	ankleL->setOrientation(
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, -1.0f)
	);

	// Construimos la jerarquia izquierda
	hipL->addChild(legL);
	legL->addChild(kneeL);
	kneeL->addChild(ankleL);

	// La cadera depende directamente de la raiz
	testSkeleton->addRootJoint(hipL);


	//
	// PIERNA DERECHA
	//

	// ARTICULACION: hip_r
	GEBalljoint* hipR = new GEBalljoint(0.2f);
	hipR->setName("hip_r");
	hipR->setLocation(glm::vec3(-0.05f, -0.05f, 0.0f));
	hipR->setOrientation(
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	// ARTICULACION: leg_r
	GEBalljoint* legR = new GEBalljoint(0.5f);
	legR->setName("leg_r");
	legR->setLocation(glm::vec3(0.0f, 0.0f, 0.0f));
	legR->setOrientation(
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f)
	);

	// ARTICULACION: knee_r
	GEBalljoint* kneeR = new GEBalljoint(0.4f);
	kneeR->setName("knee_r");
	kneeR->setLocation(glm::vec3(0.0f, 0.0f, 0.0f));
	kneeR->setOrientation(
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	// ARTICULACION: ankle_r
	GEBalljoint* ankleR = new GEBalljoint(0.25f);
	ankleR->setName("ankle_r");
	ankleR->setLocation(glm::vec3(0.0f, 0.0f, 0.0f));
	ankleR->setOrientation(
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, -1.0f)
	);

	// Construimos la jerarquia derecha
	hipR->addChild(legR);
	legR->addChild(kneeR);
	kneeR->addChild(ankleR);

	// La cadera depende directamente de la raiz
	testSkeleton->addRootJoint(hipR);




	// Desplazamiento temporal para ver el esqueleto
	// separado de la figura en T
	testSkeleton->translate(glm::vec3(1.3f, 0.0f, 0.0f));

	// Inicializamos e iluminamos el esqueleto
	testSkeleton->initialize(gc, rc);
	testSkeleton->setLight(light);


	angle = 0.0;


	


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
	column->destroy(gc);
	cross->destroy(gc);
	armL->destroy(gc);
	armR->destroy(gc);

	testSkeleton->destroy(gc);

	delete ground;
	delete column;
	delete cross;
	delete armL;
	delete armR;
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

	angle += 1.0f;
	if (angle >= 90.0f) angle = 0.0f;
	armL->setPose(angle, 0.0f, 0.0f);
	armR->setPose(0.0f, angle, 0.0f);

	ground->update(gc, index, view, projection);
	column->update(gc, index, view, projection);
	cross->update(gc, index, view, projection);
	armL->update(gc, index, view, projection);
	armR->update(gc, index, view, projection);

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
		ground->addCommands(commandBuffers[i], rc->pipelineLayout, i);
		column->addCommands(commandBuffers[i], rc->pipelineLayout, i);
		cross->addCommands(commandBuffers[i], rc->pipelineLayout, i);
		armL->addCommands(commandBuffers[i], rc->pipelineLayout, i);
		armR->addCommands(commandBuffers[i], rc->pipelineLayout, i);

		testSkeleton->addCommands(
			commandBuffers[i],
			rc->pipelineLayout,
			i
		);
	}
	rc->endFillingCommandBuffers(commandBuffers);
}