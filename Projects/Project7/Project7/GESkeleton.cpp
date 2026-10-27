
#include "GESkeleton.h"
#include <glm/gtc/matrix_transform.hpp>

//
// FUNCIÓN: GESkeleton::GESkeleton()
//
//Construye el esqueleto.
//
GESkeleton::GESkeleton(const std::string& skeletonName)
{
    name = skeletonName;

    offset = glm::vec3(0.0f, 0.0f, 0.0f);
    dir = glm::vec3(0.0f, 0.0f, 1.0f);
    up = glm::vec3(0.0f, 1.0f, 0.0f);
    right = glm::vec3(1.0f, 0.0f, 0.0f);

    location = glm::mat4(1.0f);
}

//
// FUNCIÓN: GESkeleton::getName()
//
// PROPÓSITO: Devuelve el nombre del esqueleto.
//
const std::string& GESkeleton::getName() const
{
    return name;
}

//
// FUNCIÓN: GESkeleton::getOffset()
//
glm::vec3 GESkeleton::getOffset() const
{
    return offset;
}

//
// FUNCIÓN: GESkeleton::getDirection()
//
glm::vec3 GESkeleton::getDirection() const
{
    return dir;
}

//
// FUNCIÓN: GESkeleton::getUp()
//
glm::vec3 GESkeleton::getUp() const
{
    return up;
}

//
// FUNCIÓN: GESkeleton::setOffset()
//
// PROPÓSITO: Asigna la posicion de la raiz.
//
void GESkeleton::setOffset(glm::vec3 newOffset)
{
    offset = newOffset;
    ComputeMatrix();
}

//
// FUNCIÓN: GESkeleton::setOrientation()
//
// PROPÓSITO: Asigna la orientacion de la raiz.
//
void GESkeleton::setOrientation(glm::vec3 newDir, glm::vec3 newUp)
{
    dir = newDir;
    up = newUp;
    right = glm::cross(up, dir);

    ComputeMatrix();
}

//
// FUNCIÓN: GESkeleton::addRootJoint()
//
// PROPÓSITO: Incorpora una articulacion a la raiz.
//
void GESkeleton::addRootJoint(GEBalljoint* joint)
{
    if (joint != nullptr)
    {
        rootJoints.push_back(joint);
        ComputeMatrix();
    }
}


//
// FUNCIÓN: GESkeleton::initialize()
//
// PROPÓSITO: Inicializa todas las articulaciones del esqueleto.
//
void GESkeleton::initialize(GEGraphicsContext* gc, GERenderingContext* rc)
{
    for (GEBalljoint* joint : rootJoints)
    {
        joint->initialize(gc, rc);
    }
}



//
// FUNCIÓN: GESkeleton::update()
//
// PROPÓSITO: Actualiza las variables uniformes del esqueleto.
//
void GESkeleton::update(GEGraphicsContext* gc, uint32_t index,
    glm::mat4 view, glm::mat4 projection)
{
    for (GEBalljoint* joint : rootJoints)
    {
        joint->update(gc, index, view, projection);
    }
}



//
// FUNCIÓN: GESkeleton::addCommands()
//
// PROPÓSITO: Añade los comandos de dibujo del esqueleto.
//
void GESkeleton::addCommands(VkCommandBuffer commandBuffer,
    VkPipelineLayout pipelineLayout, int index)
{
    for (GEBalljoint* joint : rootJoints)
    {
        joint->addCommands(commandBuffer, pipelineLayout, index);
    }
}



//
// FUNCIÓN: GESkeleton::destroy()
//
// PROPÓSITO: Libera los recursos de las articulaciones.
//
void GESkeleton::destroy(GEGraphicsContext* gc)
{
    for (GEBalljoint* joint : rootJoints)
    {
        joint->destroy(gc);
        delete joint;
    }

    rootJoints.clear();
}



//
// FUNCIÓN: GESkeleton::ComputeMatrix()
//
// PROPÓSITO: Calcula la matriz de transformacion de la raiz
//            y la transmite a sus articulaciones.
//
void GESkeleton::ComputeMatrix()
{
    // Matriz de posicion y orientacion del esqueleto
    glm::mat4 skeletonm(1.0f);

    // Eje X
    skeletonm[0][0] = right.x;
    skeletonm[0][1] = right.y;
    skeletonm[0][2] = right.z;

    // Eje Y
    skeletonm[1][0] = up.x;
    skeletonm[1][1] = up.y;
    skeletonm[1][2] = up.z;

    // Eje Z
    skeletonm[2][0] = dir.x;
    skeletonm[2][1] = dir.y;
    skeletonm[2][2] = dir.z;

    // Posicion de la raiz
    skeletonm[3][0] = offset.x;
    skeletonm[3][1] = offset.y;
    skeletonm[3][2] = offset.z;

    // Transformacion global del esqueleto
    glm::mat4 matrix = location * skeletonm;

    // Transmitimos la matriz a las articulaciones raiz
    for (GEBalljoint* joint : rootJoints)
    {
        joint->setRootMatrix(matrix);
    }
}


//
// FUNCION: GESkeleton::resetLocation()
//
// PROPOSITO: Resetea la matriz de localizacion.
//
void GESkeleton::resetLocation()
{
    location = glm::mat4(1.0f);
    ComputeMatrix();
}


//
// FUNCION: GESkeleton::setLocation()
//
// PROPOSITO: Asigna la matriz de localizacion.
//
void GESkeleton::setLocation(glm::mat4 m)
{
    location = glm::mat4(m);
    ComputeMatrix();
}


//
// FUNCION: GESkeleton::translate()
//
// PROPOSITO: Anade un desplazamiento al esqueleto.
//
void GESkeleton::translate(glm::vec3 t)
{
    location = glm::translate(location, t);
    ComputeMatrix();
}


//
// FUNCION: GESkeleton::rotate()
//
// PROPOSITO: Anade una rotacion al esqueleto.
//
void GESkeleton::rotate(float angle, glm::vec3 axis)
{
    location = glm::rotate(location, glm::radians(angle), axis);
    ComputeMatrix();
}


//
// FUNCION: GESkeleton::setLight()
//
// PROPOSITO: Asigna la iluminacion a todas las articulaciones.
//
void GESkeleton::setLight(GELight l)
{
    for (GEBalljoint* joint : rootJoints)
    {
        joint->setLight(l);
    }
}



//
// FUNCION: GESkeleton::setMaterial()
//
// PROPOSITO: Asigna un material a todas las articulaciones.
//
void GESkeleton::setMaterial(GEMaterial m)
{
    for (GEBalljoint* joint : rootJoints)
    {
        joint->setMaterial(m);
    }
}


//
// FUNCION: GESkeleton::buildBody()
//
// PROPOSITO: Construye manualmente el esqueleto humano
//            descrito en body.skel.
//
void GESkeleton::buildBody()
{
	// Posicion y orientacion indicadas en body.skel
	setOffset(glm::vec3(0.0f, 1.0f, 0.0f));
	setOrientation(glm::vec3(0.0f, 0.0f, 1.0f),glm::vec3(0.0f, 1.0f, 0.0f));

	// ARTICULACION: pelvis
	GEBalljoint* bodyRoot = new GEBalljoint(0.3f);
	bodyRoot->setName("pelvis");
	bodyRoot->setLocation(glm::vec3(0.0f, 0.0f, 0.0f));
	bodyRoot->setOrientation(
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
	bodyRoot->addChild(spine);
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


    // Añadimos la pelvis a la raiz del esqueleto
    addRootJoint(bodyRoot);

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

    // La cadera izquierda depende de la raiz
    addRootJoint(hipL);

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

    // La cadera derecha depende de la raiz
    addRootJoint(hipR);

}

