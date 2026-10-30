#pragma once


#pragma once

#include "GEGraphicsContext.h"
#include "GERenderingContext.h"
#include "GEBalljoint.h"
#include "GEMaterial.h"
#include "GELight.h"

#include <glm/glm.hpp>
#include <vector>
#include <string>


class GESkeleton
{
private:
    std::string name;

    glm::vec3 offset = glm::vec3(0.0f);
    glm::vec3 dir = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);

    glm::mat4 location = glm::mat4(1.0f);

    std::vector<GEBalljoint*> rootJoints;

    void ComputeMatrix();//Calcula la matriz de transformacion del esqueleto

public:
    GESkeleton(const std::string& skeletonName);

    
    void buildBody();// Construye el esqueleto humano de body.skel
    GEBalljoint* getJoint(const std::string& jointName);

    void initialize(GEGraphicsContext* gc, GERenderingContext* rc);
    void destroy(GEGraphicsContext* gc);
    void addCommands(VkCommandBuffer commandBuffer,
        VkPipelineLayout pipelineLayout, int index);
    void update(GEGraphicsContext* gc, uint32_t index,
        glm::mat4 view, glm::mat4 projection);

    void resetLocation();
    void setLocation(glm::mat4 m);
    void translate(glm::vec3 t);
    void rotate(float angle, glm::vec3 axis);
    void setMaterial(GEMaterial m);
    void setLight(GELight l);

    const std::string& getName() const;

    glm::vec3 getOffset() const;
    glm::vec3 getDirection() const;
    glm::vec3 getUp() const;

    void setOffset(glm::vec3 newOffset);
    void setOrientation(glm::vec3 newDir, glm::vec3 newUp);
    void addRootJoint(GEBalljoint* joint);
};
