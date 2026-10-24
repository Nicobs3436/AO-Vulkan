#pragma once

#include "GEGraphicsContext.h"
#include "GERenderingContext.h"
#include "GESphere.h"
#include "GECylinder.h"
#include "GELight.h"
#include <glm\glm.hpp>
#include <vector>

//
// CLASE: GEBalljoint
//
// DESCRIPCIÓN: Representa una articulación con 3 grados de libertad
// 
class GEBalljoint {
private:
	GLfloat length;//longitud del segmento
	glm::vec3 location;//posicion de la articulación
	glm::vec3 dir;//vectores para la orientación de la articulación
	glm::vec3 up;//vectores para la orientación de la articulación
	glm::vec3 right;//vectores para la orientación de la articulación
	GLfloat angles[3];//ángulos de rotación de la articulación
	GESphere* joint;//esfera que representa la articulación
	GECylinder* bone;//cilindro que representa el segmento

	//crea lka jerarquía de articulaciones
	GEBalljoint* parent = nullptr;
	std::vector<GEBalljoint*> children;

	glm::mat4 globalMatrix = glm::mat4(1.0f);//matriz de transformación global de la articulación

	void ComputeMatrix();//calcula la matriz de transformación de la articulación

public:
	GEBalljoint(float length);
	void initialize(GEGraphicsContext* gc, GERenderingContext* rc);//inicializa los recurso graficos de la articulación
	void destroy(GEGraphicsContext* gc);//libera los recursos graficos de la articulación
	void addCommands(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, int index);//añade ordenes de renderizado
	void update(GEGraphicsContext* gc, uint32_t index, glm::mat4 view, glm::mat4 projection);//actualiza los datos para dibujar

	void setLight(GELight l);//establece la luz de la articulación
	void setLocation(glm::vec3 loc);//establece la posición de la articulación
	void setOrientation(glm::vec3 nDir, glm::vec3 nUp);//establece la orientación de la articulación
	void setPose(float xrot, float yrot, float zrot);//establece los tres angulos de rotacion

	void addChild(GEBalljoint* child);//añade una articulación hija
};
