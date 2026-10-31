#include "GEAnimation.h"


//
// FUNCIÓN: GEAnimation::GEAnimation()
//
// PROPÓSITO: Asocia la animacion al esqueleto recibido.
//
GEAnimation::GEAnimation(GESkeleton* skeleton)
{
    this->skeleton = skeleton;
}


//
// FUNCIÓN: GEAnimation::addKeyframe()
//
// PROPÓSITO: Añade un fotograma en orden temporal o sustituye el del mismo instante.
//
void GEAnimation::addKeyframe(
    const std::string& jointName,
    float time,
    glm::vec3 angles)
{
    // Buscamos si ya tenemos datos de esta articulacion
    for (JointTrack& track : tracks)
    {
        if (track.jointName == jointName)
        {
            // Insertamos el fotograma en orden temporal
            for (size_t i = 0; i < track.keyframes.size(); i++)
            {
                if (time == track.keyframes[i].time)
                {
                    track.keyframes[i].angles = angles;
                    return;
                }

                if (time < track.keyframes[i].time)
                {
                    track.keyframes.insert(
                        track.keyframes.begin() + i,
                        { time, angles }
                    );
                    return;
                }
            }

            track.keyframes.push_back({ time, angles });
            return;
        }
    }

    // Es la primera vez que animamos esta articulacion
    JointTrack track;
    track.jointName = jointName;
    track.keyframes.push_back({ time, angles });
    tracks.push_back(track);
}


//
// FUNCIÓN: GEAnimation::update()
//
// PROPÓSITO: Calcula la pose interpolada y conserva las poses extremas fuera de su intervalo.
//
void GEAnimation::update(float time)
{
    if (skeleton == nullptr)
    {
        return;
    }

    //Calcula la pose de cada articulacion animada
    for (JointTrack& track : tracks)
    {
        GEBalljoint* joint = skeleton->getJoint(track.jointName);

        if (joint == nullptr || track.keyframes.empty())
        {
            continue;
        }

        //Por defecto, utiliza el primer fotograma
        glm::vec3 angles = track.keyframes.front().angles;

        //Busca los dos fotogramas entre los que se encuentra el instante actual
        for (size_t i = 1; i < track.keyframes.size(); i++)
        {
            const Keyframe& previous = track.keyframes[i - 1];
            const Keyframe& next = track.keyframes[i];

            if (time <= next.time)
            {
                //Proporcion de tiempo entre ambos fotogramas
                float t = (time - previous.time) /
                    (next.time - previous.time);

                t = glm::clamp(t, 0.0f, 1.0f);

                //Interpolacion lineal de los angulos
                angles = (1.0f - t) * previous.angles
                    + t * next.angles;

                break;
            }

            //Si hemos superado este fotograma, conservamos su pose
            angles = next.angles;
        }

        //Aplicamos la pose calculada
        joint->setPose(angles.x, angles.y, angles.z);
    }
}
