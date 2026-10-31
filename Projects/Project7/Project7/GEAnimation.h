
#pragma once

#include "GESkeleton.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>

class GEAnimation
{
private:
    //Fotograma clave de una articulacion
    struct Keyframe
    {
        float time;
        glm::vec3 angles;
    };

    //Secuencia de fotogramas clave de una articulacion
    struct JointTrack
    {
        std::string jointName;
        std::vector<Keyframe> keyframes;
    };

    //Esqueleto que vamos a animar
    GESkeleton* skeleton;

    //Datos de animacion de las articulaciones
    std::vector<JointTrack> tracks;

public:
    GEAnimation(GESkeleton* skeleton);

    void addKeyframe(
        const std::string& jointName,
        float time,
        glm::vec3 angles
    );

    void update(float time);
};
