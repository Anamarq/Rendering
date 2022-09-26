/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob

    Nori is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3
    as published by the Free Software Foundation.

    Nori is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <nori/scene.h>
#include <nori/bitmap.h>
#include <nori/integrator.h>
#include <nori/sampler.h>
#include <nori/camera.h>
#include <nori/emitter.h>
#include <nori/bsdf.h>
#include <algorithm>

#include<list>

NORI_NAMESPACE_BEGIN

Scene::Scene(const PropertyList &) {
    m_accel = new Accel();
}

Scene::~Scene() {
    delete m_accel;
    delete m_sampler;
    delete m_camera;
    delete m_integrator;
}

void Scene::activate() {
    m_accel->build();

    if (!m_integrator)
        throw NoriException("No integrator was specified!");
    if (!m_camera)
        throw NoriException("No camera was specified!");
    
    if (!m_sampler) {
        /* Create a default (independent) sampler */
        m_sampler = static_cast<Sampler*>(
            NoriObjectFactory::createInstance("independent", PropertyList()));
    }
   
    //Área total por la radianza. Coger la radianza con getLuminance y multiplicarla por el área.
    //Función para coger la radiancia y declararla en clase área
    //en área devuelvo la m_radiance
    //El área está en la clase mesh.

     //Inicializar la estructura discrete pdf. (Igual que se inicializa en mesh.cpp)
    dPDF.clear();
    dPDF.reserve(m_emitters.size()); //En este caso se reserva para el número de emitters
    for (int i = 0; i < m_emitters.size(); ++i) { 
        //Área total por la radianza. Coger la radianza con getLuminance y multiplicarla por el área.
        //Color3f luminance = m_emitters[i]->getRadiance().getLuminance();
        dPDF.append(m_emitters[i]->getRadiance().getLuminance() * m_emitters[i]->getArea());
        cout << "Radianza: " << m_emitters[i]->getRadiance().getLuminance() << " Area: " << m_emitters[i]->getArea() <<endl;
    }
    dPDF.normalize(); //Normalizar


    cout << endl;
    cout << "Configuration: " << toString() << endl;
    cout << endl;
}

void Scene::addChild(NoriObject *obj) {
    switch (obj->getClassType()) {
        case EMesh: {
                Mesh *mesh = static_cast<Mesh *>(obj);
                m_accel->addMesh(mesh);
                m_meshes.push_back(mesh);
                if (mesh->isEmitter())
                {
            
                    mesh->getEmitter()->setArea(mesh->getAreaMesh());
                    m_emitters.push_back(mesh->getEmitter());
                    
                }
            }
            break;
        
        case EEmitter: {
                //Emitter *emitter = static_cast<Emitter *>(obj);
                /* TBD */
                throw NoriException("Scene::addChild(): You need to implement this for emitters");
            }
            break;

        case ESampler:
            if (m_sampler)
                throw NoriException("There can only be one sampler per scene!");
            m_sampler = static_cast<Sampler *>(obj);
            break;

        case ECamera:
            if (m_camera)
                throw NoriException("There can only be one camera per scene!");
            m_camera = static_cast<Camera *>(obj);
            break;
        
        case EIntegrator:
            if (m_integrator)
                throw NoriException("There can only be one integrator per scene!");
            m_integrator = static_cast<Integrator *>(obj);
            break;

        default:
            throw NoriException("Scene::addChild(<%s>) is not supported!",
                classTypeName(obj->getClassType()));
    }
}

std::string Scene::toString() const {
    std::string meshes;
    for (size_t i=0; i<m_meshes.size(); ++i) {
        meshes += std::string("  ") + indent(m_meshes[i]->toString(), 2);
        if (i + 1 < m_meshes.size())
            meshes += ",";
        meshes += "\n";
    }

    return tfm::format(
        "Scene[\n"
        "  integrator = %s,\n"
        "  sampler = %s\n"
        "  camera = %s,\n"
        "  meshes = {\n"
        "  %s  }\n"
        "]",
        indent(m_integrator->toString()),
        indent(m_sampler->toString()),
        indent(m_camera->toString()),
        indent(meshes, 2)
    );
}

Color3f Scene::sampleEmitter( EmitterQueryRecord& lRec, const Point2f& samplep) const{
    //Lista con todos los emisores. 
    //Mesh::m_emmitter, la creación de la lista se realizará comprobando si las mallas son a su vez luces de área

    bool ap1 = false;
    //TAREA 1
    //Seleccionar uno de los posibles emisores de la escena de forma aleatoria .
    Point2f sample(samplep);
    int index;
    if (ap1) {
        //Muestreo uniforme de luces de área
        index = std::min((int)(m_emitters.size() * sample.x()), (int)m_emitters.size() - 1);
        lRec.pdf = 1.0f / (float)m_emitters.size();
    }
    else {
        //Muestreo de emisores con probabilidad asociada a la radianza
        index = dPDF.sampleReuse(sample.x());
    }
    lRec.emitter = m_emitters[index];
    float f = m_emitters.size() * sample.x() - index;
    sample.x() = f;



    //TAREA 2
    Color3f radEm = lRec.emitter->sample(lRec, samplep);  
    ////////Rayo para comprobar la visibilidad:
    Intersection its;
    Ray3f ray = Ray3f(lRec.o,lRec.wi);
    
  //  rayIntersect(Ray3f(lRec.p, lRec.wi, Epsilon, lRec.dist * (1 - 1e-4f)));
    //ref origen (o)
    //d direccion desde origen hasta el punto sampleado de luz (wi)
    // dist es la distancia entre los 2 puntos .norm
   // Ray3f ray = (Ray3f(lRec.p, lRec.wi, Epsilon, lRec.dist * (1 - 1e-4f)));
    if (rayIntersect(ray,its) && !its.mesh->isEmitter())
        return  0;
    else {
        return radEm;  
    }
}

NORI_REGISTER_CLASS(Scene, "scene");
NORI_NAMESPACE_END
