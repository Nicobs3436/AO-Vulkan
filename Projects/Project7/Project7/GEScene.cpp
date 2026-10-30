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
#include <string>

void addLegPose(
	GEAnimation* animation, const std::string& side, float time,
	float lx, float ly, float lz, float knee,
	float ax, float ay, float az)
{
	animation->addKeyframe(
		"leg_" + side, time, glm::vec3(lx, ly, lz)
	);

	animation->addKeyframe(
		"knee_" + side, time, glm::vec3(knee, 0.0f, 0.0f)
	);

	animation->addKeyframe(
		"ankle_" + side, time, glm::vec3(ax, ay, az)
	);
}

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

	// Conservamos la llegada antes de iniciar el lanzamiento
	animation->addKeyframe("leg_r", 2.2f, glm::vec3(-40.0f, 8.0f, 0.0f));
	animation->addKeyframe("knee_r", 2.2f, glm::vec3(80.0f, 0.0f, 0.0f));
	animation->addKeyframe("ankle_r", 2.2f, glm::vec3(-40.0f, -5.0f, -6.0f));
	animation->addKeyframe("spine", 2.2f, glm::vec3(-25.0f, 0.0f, 0.0f));
	animation->addKeyframe("shoulder_l", 2.2f, glm::vec3(70.0f, -25.0f, 40.0f));
	animation->addKeyframe("neck", 2.2f, glm::vec3(5.0f, 0.0f, 0.0f));

	// Pierna derecha: giro y extension progresiva
	animation->addKeyframe("leg_r", 2.325f, glm::vec3(-34.5f, 5.0f, 7.5f));
	animation->addKeyframe("knee_r", 2.325f, glm::vec3(80.0f, 0.0f, 0.0f));
	animation->addKeyframe("ankle_r", 2.325f, glm::vec3(-44.0f, -3.5f, -3.5f));

	animation->addKeyframe("leg_r", 2.45f, glm::vec3(-28.3f, 1.9f, 15.0f));
	animation->addKeyframe("knee_r", 2.45f, glm::vec3(78.2f, 0.0f, 0.0f));
	animation->addKeyframe("ankle_r", 2.45f, glm::vec3(-44.2f, -1.5f, -1.2f));

	animation->addKeyframe("leg_r", 2.7f, glm::vec3(-16.4f, -7.2f, 30.0f));
	animation->addKeyframe("knee_r", 2.7f, glm::vec3(74.5f, 0.0f, 0.0f));
	animation->addKeyframe("ankle_r", 2.7f, glm::vec3(-43.5f, 6.1f, 3.8f));

	animation->addKeyframe("leg_r", 2.95f, glm::vec3(-3.5f, -19.2f, 45.0f));
	animation->addKeyframe("knee_r", 2.95f, glm::vec3(63.6f, 0.0f, 0.0f));
	animation->addKeyframe("ankle_r", 2.95f, glm::vec3(-35.5f, 16.5f, 9.8f));

	animation->addKeyframe("leg_r", 3.075f, glm::vec3(3.5f, -25.9f, 52.5f));
	animation->addKeyframe("knee_r", 3.075f, glm::vec3(53.3f, 0.0f, 0.0f));
	animation->addKeyframe("ankle_r", 3.075f, glm::vec3(-26.4f, 21.4f, 14.9f));

	animation->addKeyframe("leg_r", 3.2f, glm::vec3(13.2f, -32.1f, 60.0f));
	animation->addKeyframe("knee_r", 3.2f, glm::vec3(36.1f, 0.0f, 0.0f));
	animation->addKeyframe("ankle_r", 3.2f, glm::vec3(-9.6f, 23.7f, 22.2f));

	// Pierna izquierda: mantiene el apoyo y la rodilla firme
	animation->addKeyframe("leg_l", 3.2f, glm::vec3(-41.3f, -8.3f, 0.0f));
	animation->addKeyframe("ankle_l", 3.2f, glm::vec3(29.0f, -4.0f, 7.2f));

	// Primero avanza la cadera; el tronco permanece retrasado
	animation->addKeyframe("pelvis", 2.7f, glm::vec3(-12.0f, 0.0f, -70.0f));
	animation->addKeyframe("spine", 2.7f, glm::vec3(-18.0f, 0.0f, 0.0f));
	animation->addKeyframe("shoulder_l", 2.7f, glm::vec3(5.0f, -10.0f, 80.0f));
	animation->addKeyframe("elbow_l", 2.7f, glm::vec3(5.0f, 0.0f, 0.0f));
	animation->addKeyframe("wrist_l", 2.7f, glm::vec3(0.0f, 0.0f, 0.0f));

	// El tronco comienza a alcanzar el giro de la cadera
	animation->addKeyframe("pelvis", 3.05f, glm::vec3(-2.0f, 0.0f, -30.0f));
	animation->addKeyframe("spine", 3.05f, glm::vec3(-5.0f, 0.0f, 0.0f));

	// Cuerpo orientado hacia el lanzamiento
	animation->addKeyframe("pelvis", 3.2f, glm::vec3(5.0f, 0.0f, 0.0f));
	animation->addKeyframe("spine", 3.2f, glm::vec3(0.0f, 0.0f, 0.0f));
	animation->addKeyframe("neck", 3.2f, glm::vec3(0.0f, 0.0f, 0.0f));

	// Recogemos el brazo izquierdo
	animation->addKeyframe("shoulder_l", 3.2f, glm::vec3(80.0f, -10.0f, 0.0f));
	animation->addKeyframe("elbow_l", 3.2f, glm::vec3(140.0f, 0.0f, 0.0f));
	animation->addKeyframe("wrist_l", 3.2f, glm::vec3(-20.0f, 0.0f, 0.0f));

	// Conservamos el agarre hasta terminar el giro
	animation->addKeyframe("shoulder_r", 3.2f, glm::vec3(-15.0f, 15.0f, -10.0f));
	animation->addKeyframe("elbow_r", 3.2f, glm::vec3(-160.0f, 0.0f, 0.0f));
	animation->addKeyframe("wrist_r", 3.2f, glm::vec3(35.0f, 15.0f, -10.0f));

	// Empuje del brazo derecho
	animation->addKeyframe("shoulder_r", 3.4f, glm::vec3(-45.0f, 10.0f, -45.0f));
	animation->addKeyframe("elbow_r", 3.4f, glm::vec3(-100.0f, 0.0f, 0.0f));
	animation->addKeyframe("wrist_r", 3.4f, glm::vec3(20.0f, 8.0f, -5.0f));

	// Final del empuje: brazo extendido hacia delante y arriba
	animation->addKeyframe("pelvis", 3.6f, glm::vec3(-5.0f, 0.0f, 0.0f));
	animation->addKeyframe("spine", 3.6f, glm::vec3(-5.0f, 0.0f, 0.0f));
	animation->addKeyframe("shoulder_r", 3.6f, glm::vec3(-80.0f, 70.0f, -30.0f));
	animation->addKeyframe("elbow_r", 3.6f, glm::vec3(-5.0f, 0.0f, 0.0f));
	animation->addKeyframe("wrist_r", 3.6f, glm::vec3(0.0f, 0.0f, 0.0f));


	// Pierna izquierda: sostiene el cuerpo y despues queda detras
	addLegPose(animation, "l", 3.6f, -41.3f, -8.3f, 0.0f, 12.0f, 29.0f, -4.0f, 7.2f);
	addLegPose(animation, "l", 3.66f, -46.5f, -8.6f, 0.0f, 28.9f, 17.4f, -2.6f, 8.2f);
	addLegPose(animation, "l", 3.725f, -48.4f, -9.0f, 0.0f, 39.2f, 9.1f, -1.4f, 8.9f);
	addLegPose(animation, "l", 3.85f, -48.8f, -9.7f, 0.0f, 52.4f, -3.6f, 0.6f, 9.7f);
	addLegPose(animation, "l", 3.925f, -47.7f, -10.2f, 0.0f, 58.0f, -10.1f, 1.8f, 10.0f);
	addLegPose(animation, "l", 4.0f, -45.9f, -10.6f, 0.0f, 62.4f, -16.2f, 3.0f, 10.2f);
	addLegPose(animation, "l", 4.15f, -45.4f, -10.7f, 0.0f, 65.9f, -20.2f, 3.7f, 10.0f);
	addLegPose(animation, "l", 4.25f, -47.3f, -11.5f, 0.0f, 85.6f, -17.7f, 7.1f, 9.1f);
	addLegPose(animation, "l", 4.6f, -8.7f, -11.4f, 0.0f, 78.5f, -49.4f, 10.7f, 4.0f);
	addLegPose(animation, "l", 5.0f, 14.2f, -8.5f, 0.0f, 20.4f, -11.3f, 4.8f, 7.0f);

	// Pierna derecha: avanza, recibe el peso y recupera la extension
	addLegPose(animation, "r", 3.6f, 13.2f, -32.1f, 60.0f, 36.1f, -9.6f, 23.7f, 22.2f);
	addLegPose(animation, "r", 3.85f, -1.0f, -15.1f, 30.0f, 90.0f, -64.0f, 15.1f, 0.3f);
	addLegPose(animation, "r", 4.0f, -35.3f, 14.0f, 0.0f, 106.6f, -60.8f, -13.3f, -4.6f);
	addLegPose(animation, "r", 4.15f, -45.8f, 12.0f, 0.0f, 67.6f, -21.4f, -4.4f, -11.2f);
	addLegPose(animation, "r", 4.25f, -44.2f, 11.9f, 0.0f, 67.6f, -22.9f, -4.7f, -11.0f);
	addLegPose(animation, "r", 4.425f, -37.1f, 11.3f, 0.0f, 54.8f, -17.4f, -3.4f, -10.8f);
	addLegPose(animation, "r", 4.5125f, -33.0f, 11.0f, 0.0f, 47.2f, -13.9f, -2.7f, -10.7f);
	addLegPose(animation, "r", 4.6f, -28.3f, 10.7f, 0.0f, 38.0f, -9.5f, -1.8f, -10.6f);
	addLegPose(animation, "r", 5.0f, -21.6f, 10.7f, 0.0f, 23.6f, -2.0f, -0.4f, -10.7f);

	// Conservamos estas poses hasta terminar el empuje
	animation->addKeyframe("neck", 3.6f, glm::vec3(0.0f, 0.0f, 0.0f));
	animation->addKeyframe("shoulder_l", 3.6f, glm::vec3(80.0f, -10.0f, 0.0f));
	animation->addKeyframe("elbow_l", 3.6f, glm::vec3(140.0f, 0.0f, 0.0f));
	animation->addKeyframe("wrist_l", 3.6f, glm::vec3(-20.0f, 0.0f, 0.0f));

	// Continuacion del gesto: tronco hacia delante y brazo descendiendo
	animation->addKeyframe("pelvis", 3.85f, glm::vec3(-15.0f, 0.0f, 0.0f));
	animation->addKeyframe("spine", 3.85f, glm::vec3(-15.0f, 0.0f, 0.0f));
	animation->addKeyframe("neck", 3.85f, glm::vec3(5.0f, 0.0f, 0.0f));
	animation->addKeyframe("shoulder_r", 3.85f, glm::vec3(-40.0f, 70.0f, -20.0f));
	animation->addKeyframe("elbow_r", 3.85f, glm::vec3(-15.0f, 0.0f, 0.0f));
	animation->addKeyframe("wrist_r", 3.85f, glm::vec3(-15.0f, 0.0f, 0.0f));

	// Frenada durante el cambio de apoyos
	animation->addKeyframe("pelvis", 4.25f, glm::vec3(-20.0f, 0.0f, 0.0f));
	animation->addKeyframe("spine", 4.25f, glm::vec3(-15.0f, 0.0f, 0.0f));
	animation->addKeyframe("shoulder_r", 4.25f, glm::vec3(30.0f, 50.0f, 0.0f));
	animation->addKeyframe("elbow_r", 4.25f, glm::vec3(-25.0f, 0.0f, 0.0f));
	animation->addKeyframe("wrist_r", 4.25f, glm::vec3(0.0f, 0.0f, 0.0f));
	animation->addKeyframe("shoulder_l", 4.25f, glm::vec3(60.0f, -15.0f, 0.0f));
	animation->addKeyframe("elbow_l", 4.25f, glm::vec3(60.0f, 0.0f, 0.0f));
	animation->addKeyframe("wrist_l", 4.25f, glm::vec3(0.0f, 0.0f, 0.0f));

	// Recuperacion del equilibrio y brazos relajados
	animation->addKeyframe("pelvis", 5.0f, glm::vec3(-5.0f, 0.0f, 0.0f));
	animation->addKeyframe("spine", 5.0f, glm::vec3(-5.0f, 0.0f, 0.0f));
	animation->addKeyframe("neck", 5.0f, glm::vec3(5.0f, 0.0f, 0.0f));
	animation->addKeyframe("shoulder_r", 5.0f, glm::vec3(80.0f, 10.0f, 0.0f));
	animation->addKeyframe("elbow_r", 5.0f, glm::vec3(-10.0f, 0.0f, 0.0f));
	animation->addKeyframe("wrist_r", 5.0f, glm::vec3(0.0f, 0.0f, 0.0f));
	animation->addKeyframe("shoulder_l", 5.0f, glm::vec3(80.0f, -10.0f, 0.0f));
	animation->addKeyframe("elbow_l", 5.0f, glm::vec3(5.0f, 0.0f, 0.0f));
	animation->addKeyframe("wrist_l", 5.0f, glm::vec3(0.0f, 0.0f, 0.0f));


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
	time = glm::clamp(time, 0.0f, 5.0f);

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


	// Giro y transferencia del cuerpo durante el lanzamiento
	if (time > 2.2f)
	{
		float avanceGiro = glm::clamp(
			(time - 2.2f) / 1.0f, 0.0f, 1.0f
		);

		giro = glm::radians(90.0f + 90.0f * avanceGiro);

		glm::vec3 p0(0.0000f, 0.8330f, -0.9540f);
		glm::vec3 p1(0.0457f, 0.8476f, -0.9845f);
		glm::vec3 p2(0.0768f, 0.8581f, -1.0502f);
		glm::vec3 p3(0.0810f, 0.8649f, -1.1428f);
		glm::vec3 p4(0.0500f, 0.8679f, -1.2500f);

		if (time <= 2.45f)
		{
			float avance = (time - 2.2f) / 0.25f;
			posicion = (1.0f - avance) * p0 + avance * p1;
		}
		else if (time <= 2.7f)
		{
			float avance = (time - 2.45f) / 0.25f;
			posicion = (1.0f - avance) * p1 + avance * p2;
		}
		else if (time <= 2.95f)
		{
			float avance = (time - 2.7f) / 0.25f;
			posicion = (1.0f - avance) * p2 + avance * p3;
		}
		else
		{
			float avance = glm::clamp(
				(time - 2.95f) / 0.25f, 0.0f, 1.0f
			);

			posicion = (1.0f - avance) * p3 + avance * p4;
		}
	}


	// Recuperacion tras el lanzamiento
	if (time > 3.6f)
	{
		giro = glm::radians(180.0f);

		glm::vec3 p0(0.050f, 0.8679f, -1.250f);
		glm::vec3 p1(0.020f, 0.8700f, -1.530f);
		glm::vec3 p2(0.020f, 0.8600f, -1.580f);
		glm::vec3 p3(0.025f, 0.9700f, -1.600f);
		glm::vec3 p4(0.030f, 1.0000f, -1.600f);

		if (time <= 4.0f)
		{
			float avance = (time - 3.6f) / 0.4f;
			posicion = (1.0f - avance) * p0 + avance * p1;
		}
		else if (time <= 4.25f)
		{
			float avance = (time - 4.0f) / 0.25f;
			posicion = (1.0f - avance) * p1 + avance * p2;
		}
		else if (time <= 4.6f)
		{
			float avance = (time - 4.25f) / 0.35f;
			posicion = (1.0f - avance) * p2 + avance * p3;
		}
		else
		{
			float avance = glm::clamp(
				(time - 4.6f) / 0.4f, 0.0f, 1.0f
			);

			posicion = (1.0f - avance) * p3 + avance * p4;
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


