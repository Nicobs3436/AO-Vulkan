#include "GEBalljoint.h"

#include "GEMaterial.h"
#include "GESphere.h"
#include "GECylinder.h"
#include "GELight.h"
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

//
// FUNCIÓN: GEBalljoint::GEBalljoint()
//
// PROPÓSITO: Construye una articulación.
//
GEBalljoint::GEBalljoint(float l)
{
    length = l;

    location = glm::vec3(0.0f, 0.0f, 0.0f);
    dir = glm::vec3(0.0f, 0.0f, 1.0f);
    up = glm::vec3(0.0f, 1.0f, 0.0f);
    right = glm::vec3(1.0f, 0.0f, 0.0f);

    angles[0] = 0.0f;
    angles[1] = 0.0f;
    angles[2] = 0.0f;

    joint = nullptr;
    bone = nullptr;
}

//
// FUNCIÓN: GEBalljoint::ComputeMatrix()
//
// PROPÓSITO: Crea la matriz de transformación a partir de la posición, 
//            la orientación y la pose.
//
void GEBalljoint::ComputeMatrix()
{
    // Formato glm::mat4[column][row]
    glm::mat4 jointm;
    jointm[0][0] = right.x;
    jointm[0][1] = right.y;
    jointm[0][2] = right.z;
    jointm[0][3] = 0.0;

    jointm[1][0] = up.x;
    jointm[1][1] = up.y;
    jointm[1][2] = up.z;
    jointm[1][3] = 0.0;

    jointm[2][0] = dir.x;
    jointm[2][1] = dir.y;
    jointm[2][2] = dir.z;
    jointm[2][3] = 0.0;

    jointm[3][0] = location.x;
    jointm[3][1] = location.y;
    jointm[3][2] = location.z;
    jointm[3][3] = 1.0;

    float cx = (float)cos(glm::radians(angles[0]));
    float sx = (float)sin(glm::radians(angles[0]));
    float cy = (float)cos(glm::radians(angles[1]));
    float sy = (float)sin(glm::radians(angles[1]));
    float cz = (float)cos(glm::radians(angles[2]));
    float sz = (float)sin(glm::radians(angles[2]));

    glm::mat4 posem;
    posem[0][0] = cz * cy;
    posem[1][0] = -sz * cx + cz * sy * sx;
    posem[2][0] = sz * sx + cz * sy * cx;
    posem[3][0] = 0;

    posem[0][1] = sz * cy;
    posem[1][1] = cz * cx + sz * sy * sx;
    posem[2][1] = -cz * sx + sz * sy * cx;
    posem[3][1] = 0;

    posem[0][2] = -sy;
    posem[1][2] = cy * sx;
    posem[2][2] = cy * cx;
    posem[3][2] = 0;

    posem[0][3] = 0;
    posem[1][3] = 0;
    posem[2][3] = 0;
    posem[3][3] = 1;



    glm::mat4 matrix = jointm * posem;

    if (parent != nullptr)
    {
        //Articulacion hija: hereda la transformacion del padre
        glm::mat4 parentEnd = glm::translate(parent->globalMatrix,glm::vec3(0.0f, 0.0f, parent->length));

        matrix = parentEnd * matrix;
    }
    else
    {
        //Articulacion raiz hereda la transformacion del esqueleto
        matrix = rootMatrix * matrix;
    }


    //Guardiola transformación global
    globalMatrix = matrix;

    //Actualizo la posicion de la esfera y del hueso
    if (joint != nullptr && bone != nullptr)
    {
        joint->setLocation(globalMatrix);

        glm::mat4 mm = glm::translate(globalMatrix,glm::vec3(0.0f, 0.0f, length / 2.0f));

        bone->setLocation(mm);
    }

    // Actualizo las articulaciones hijas recursivamente
    for (GEBalljoint* child : children)
    {
        child->ComputeMatrix();
    }
}

//
// FUNCIÓN: GEBalljoint::initialize(GEGraphicsContext* gc, GERenderingContext* rc)
//
// PROPÓSITO: Inicializa las piezas de la articulación
//
void GEBalljoint::initialize(GEGraphicsContext* gc, GERenderingContext* rc)
{
    GEMaterial jointMat = {};
    jointMat.Ka = glm::vec3(1.0f, 0.0f, 0.0f);
    jointMat.Kd = glm::vec3(1.0f, 0.0f, 0.0f);
    jointMat.Ks = glm::vec3(0.8f, 0.8f, 0.8f);
    jointMat.Shininess = 16.0f;

    joint = new GESphere(10, 20, 0.1f);
    joint->initialize(gc, rc);
    joint->setMaterial(jointMat);

    GEMaterial boneMat = {};
    boneMat.Ka = glm::vec3(0.0f, 0.0f, 0.8f);
    boneMat.Kd = glm::vec3(0.0f, 0.0f, 0.8f);
    boneMat.Ks = glm::vec3(0.8f, 0.8f, 0.8f);
    boneMat.Shininess = 16.0f;

    bone = new GECylinder(2, 10, 0.05f, length / 2);
    bone->initialize(gc, rc);

    bone->setMaterial(boneMat);

    //Inicializa primero las articulaciones hijas
    for (GEBalljoint* child : children)
    {
        child->initialize(gc, rc);
    }

    //Calcula las matrices de toda la jerarquia
    ComputeMatrix();

}

//
// FUNCIÓN: GEBalljoint::update(GEGraphicsContext* gc, uint32_t index, glm::mat4 view, glm::mat4 projection)
//
// PROPÓSITO: Actualiza las variables uniformes
//

void GEBalljoint::update(GEGraphicsContext* gc, uint32_t index,
    glm::mat4 view, glm::mat4 projection)
{
    //Actualiza la esfera y el cilindro de esta articulacion
    joint->update(gc, index, view, projection);
    bone->update(gc, index, view, projection);

    //Actualiza todas las articulaciones hijas
    for (GEBalljoint* child : children)
    {
        child->update(gc, index, view, projection);
    }
}


//
// FUNCIÓN: GEBalljoint::destroy(CAVulkanState* vulkan)
//
// PROPÓSITO: Destruye los buffers de las piezas
//

void GEBalljoint::destroy(GEGraphicsContext* gc)
{
    //Destruye las articulaciones hijas
    for (GEBalljoint* child : children)
    {
        child->destroy(gc);
        delete child;
    }

    children.clear();

    //Libera los recursos de esta articulacion
    joint->destroy(gc);
    bone->destroy(gc);

    delete joint;
    delete bone;

    joint = nullptr;
    bone = nullptr;
}


//
// FUNCIÓN: GEBalljoint::setLocation(glm::vec3 loc)
//
// PROPÓSITO: Asigna la posición de la articulación
//
void GEBalljoint::setLocation(glm::vec3 loc)
{
    location = loc;
    ComputeMatrix();
}

//
// FUNCIÓN: GEBalljoint::setOrientation(glm::vec3 nDir, glm::vec3 nUp)
//
// PROPÓSITO: Asigna la orientación de la articulación (con pose 0,0,0).
//
void GEBalljoint::setOrientation(glm::vec3 nDir, glm::vec3 nUp)
{
    dir = nDir;
    up = nUp;
    right = glm::cross(up, dir);
    ComputeMatrix();
}

//
// FUNCIÓN: GEBalljoint::setPose()
//
// PROPÓSITO: Asigna la rotación de la articulación
//
void GEBalljoint::setPose(float xrot, float yrot, float zrot)
{
    glm::vec3 pose(xrot, yrot, zrot);

    //Solo restringimos los angulos si hay limites definidos
    if (limitsEnabled)
    {
        pose = glm::clamp(pose, minAngles, maxAngles);
    }

    angles[0] = pose.x;
    angles[1] = pose.y;
    angles[2] = pose.z;

    ComputeMatrix();
}

//
// FUNCIÓN: GEBalljoint::addCommands()
//
// PROPÓSITO: Añade los comandos de dibujo
//

void GEBalljoint::addCommands(VkCommandBuffer commandBuffer,
    VkPipelineLayout pipelineLayout, int index)
{
    // Dibuja la articulacion actual
    joint->addCommands(commandBuffer, pipelineLayout, index);
    bone->addCommands(commandBuffer, pipelineLayout, index);

    // Añade los comandos de dibujo de las articulaciones hijas
    for (GEBalljoint* child : children)
    {
        child->addCommands(commandBuffer, pipelineLayout, index);
    }
}

//
// FUNCIÓN: GEBalljoint::setLight(GELight l)
//
// PROPÓSITO: Asigna las propiedades de la luz
//

void GEBalljoint::setLight(GELight l)
{
    joint->setLight(l);
    bone->setLight(l);

    //Aplicamos la luz a las articulaciones hijas
    for (GEBalljoint* child : children)
    {
        child->setLight(l);
    }
}


//
// FUNCIÓN: GEBalljoint::addChild()
//
// PROPÓSITO: Añade una articulación hija.
//
void GEBalljoint::addChild(GEBalljoint* child)
{
	//comprueba q no se añada a si mismo, que no sea nulo y que no tenga padre
    if (child == nullptr || child == this ||
        child->parent != nullptr)
    {
        return;
    }

	//comprueba que no se cree un ciclo en la jerarquía
    for (GEBalljoint* p = this; p != nullptr; p = p->parent)
    {
        if (p == child)
        {
            return;
        }
    }

	//asigna el padre y añade a la lista de hijos
    child->parent = this;
    children.push_back(child);
    child->ComputeMatrix();
}



//
// FUNCIÓN: GEBalljoint::setRootMatrix()
//
// PROPÓSITO: Asigna la transformacion de la raiz del esqueleto.
//
void GEBalljoint::setRootMatrix(glm::mat4 matrix)
{
    if (parent == nullptr)
    {
        rootMatrix = matrix;
        ComputeMatrix();
    }
}



//
// FUNCION: GEBalljoint::setMaterial()
//
// PROPOSITO: Asigna un material a las piezas de la
//            articulacion y a sus articulaciones hijas.
//
void GEBalljoint::setMaterial(GEMaterial m)
{
    //Asigna el material a la esfera y al cilindro
    joint->setMaterial(m);
    bone->setMaterial(m);

    //Aplica el material a las articulaciones hijas
    for (GEBalljoint* child : children)
    {
        child->setMaterial(m);
    }
}


//
// FUNCION: GEBalljoint::setName()
//
// PROPOSITO: Asigna el nombre de la articulacion.
//
void GEBalljoint::setName(const std::string& newName)
{
    name = newName;
}

//
// FUNCION: GEBalljoint::getName()
//
// PROPOSITO: Devuelve el nombre de la articulacion.
//
const std::string& GEBalljoint::getName() const
{
    return name;
}

// Consulta de la geometria, la pose y la jerarquia de la articulacion.
float GEBalljoint::getLength() const
{
    return length;
}

glm::vec3 GEBalljoint::getLocation() const
{
    return location;
}

glm::vec3 GEBalljoint::getDirection() const
{
    return dir;
}

glm::vec3 GEBalljoint::getUp() const
{
    return up;
}

glm::vec3 GEBalljoint::getRight() const
{
    return right;
}

glm::vec3 GEBalljoint::getPose() const
{
    return glm::vec3(angles[0], angles[1], angles[2]);
}

glm::vec3 GEBalljoint::getMinAngles() const
{
    return minAngles;
}

glm::vec3 GEBalljoint::getMaxAngles() const
{
    return maxAngles;
}

bool GEBalljoint::hasLimits() const
{
    return limitsEnabled;
}

GEBalljoint* GEBalljoint::getParent() const
{
    return parent;
}

size_t GEBalljoint::getChildCount() const
{
    return children.size();
}

GEBalljoint* GEBalljoint::getChild(size_t index) const
{
    if (index >= children.size())
    {
        return nullptr;
    }
    return children[index];
}


void GEBalljoint::setLimits(glm::vec3 minimum, glm::vec3 maximum)
{

    minAngles = minimum;
    maxAngles = maximum;
    limitsEnabled = true;

    // Ajustamos la pose actual a los nuevos limites
    setPose(angles[0], angles[1], angles[2]);
}




//
// FUNCION: GEBalljoint::findJoint()
//
// PROPOSITO: compara el nombre de la articulación actual. Si no coincide, busca recursivamente entre sus hijas.
//
GEBalljoint* GEBalljoint::findJoint(const std::string& jointName)
{
    // Comprobamos si la articulacion actual es la buscada
    if (name == jointName)
    {
        return this;
    }

    // Buscamos entre las articulaciones hijas
    for (GEBalljoint* child : children)
    {
        GEBalljoint* found = child->findJoint(jointName);

        if (found != nullptr)
        {
            return found;
        }
    }

    // No hemos encontrado la articulacion
    return nullptr;
}
