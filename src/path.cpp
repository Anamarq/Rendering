#include <nori/integrator.h>
#include <nori/scene.h>
#include <nori/warp.h>
#include <nori/mesh.h>
#include <nori/bsdf.h>
#include <nori/emitter.h>
#include <nori/scene.h>

NORI_NAMESPACE_BEGIN
#define MAX_PATH_LENGTH 4096
struct PathInfo {
public:
    unsigned int depth;
    Color3f pathThroughput;
    /// Return a human-readable string summary
    std::string toString() const {
        return tfm::format("[%d, %f, %f, %f]", depth, pathThroughput[0],
            pathThroughput[1], pathThroughput[2]);
    }
};



class PathTracing : public Integrator {
public:

    PathTracing(const PropertyList& props) : mesh(NULL) {}
    std::string toString() const {
        return "PathTracing[]";
    }
    Color3f Li(const Scene* scene, Sampler* sampler, const Ray3f& ray) const {
        PathInfo pathInfo;
        pathInfo.depth = 0;
        pathInfo.pathThroughput.setOnes();

        Color3f myLi;

        //Llamada recursiva cuando sea necesaria.
      
        myLi = LiRecursive(scene, sampler, ray, pathInfo);
        return myLi;

    }

    Color3f LiRecursive(const Scene* scene, Sampler* sampler, const Ray3f& ray, PathInfo& pathInfo) const {


        Color3f li = Color3f(0.0f);
        Color3f le = Color3f(0.0f); //Emisiva     
        Color3f bcolor = Color3f(0.0f); //Material
        Ray3f rayRec;

        //Si no choca con nada devuelvo 0
        Intersection its;
        if (!scene->rayIntersect(ray, its))
            return Color3f(0.0);

        if (pathInfo.depth <= MAX_PATH_LENGTH)
        {
            //Si es emitter devuelvo el color
            if (its.mesh->isEmitter()) {
                //Evaluar radiancia incidente
                Normal3f n = its.shFrame.n; //normal
                Point3f p = its.p; //punto de intersección
                Point3f o = ray.o; //origen del rayo
                //Estructura para almacenar propiedades emisivas del material
                EmitterQueryRecord rec = EmitterQueryRecord(o, p, n);
                //Calcular el color del material a partir del emitter
                le = its.mesh->getEmitter()->eval(rec);
                return le * pathInfo.pathThroughput;
            }

            pathInfo.depth++;

            //Indirecta
            BSDFQueryRecord bRec(its.shFrame.toLocal(-ray.d));
            bcolor = its.mesh->getBSDF()->sample(bRec, sampler->next2D());

            pathInfo.pathThroughput *= bcolor;
            //Nuevo rayo
            rayRec = Ray3f(its.p, its.toWorld(bRec.wo));


            li = LiRecursive(scene, sampler, rayRec, pathInfo);

            //Ruleta rusa
            float probRR = std::min(pathInfo.pathThroughput.x(), 0.9f);
            if (sampler->next1D() >= probRR)
                return Color3f(0.0f);

            pathInfo.pathThroughput /= probRR;

            return li;

        }
        return Color3f(0.0);
    }
    Color3f eval(const BSDFQueryRecord& lRec) const {
        //Evaluación del material para una muestra (dirección) dada

    }
    float pdf(const BSDFQueryRecord& lRec) const {
        //Valor de probabilidad para una muestra dada.
    }

private:
    Mesh* mesh;
    BSDF* m_bsdf;
};

NORI_REGISTER_CLASS(PathTracing, "pathtracer");
NORI_NAMESPACE_END



