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



class PathTracingNEE : public Integrator {
public:

    PathTracingNEE(const PropertyList& props) : mesh(NULL) {}
    std::string toString() const {
        return "PathTracingNEE[]";
    }
    Color3f Li(const Scene* scene, Sampler* sampler, const Ray3f& ray) const {
        PathInfo pathInfo;
        pathInfo.depth = 0;
        pathInfo.pathThroughput.setOnes();

        Color3f myLi;
        Color3f colorLuz;
        Color3f colorFin;
        float pdfLuzDirect = 0;
        float pdfMatDirect = 0;
        float weightD = 0.0f;
        float weightI = 0.0f;

        Intersection its;
        Color3f le=0;

        //Llamada recursiva cuando sea necesaria.

        myLi = LiRecursive(scene, sampler, ray, pathInfo);
        //colorLuz = LiLight(scene, sampler, ray, weightD);


        return myLi;

    }

    Color3f LiRecursive(const Scene* scene, Sampler* sampler, const Ray3f& ray, PathInfo& pathInfo) const {

        //Conseguir la intersección del rayo con la escena con rayIntersect.
        //Pesos y color para la luz directa
        float weightD = 0.0f;
        Color3f ld = Color3f(0.0f);

        //Luz indirecta
        float weightI = 0.0f;
        float pdfLuzI = 0.0f;
        float pdfMatI = 0.0f;
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
            

            ///Rayo directo
            ld = LiLight(scene, sampler, ray,  weightD);
            ld *= pathInfo.pathThroughput;

            //Indirecta
            BSDFQueryRecord bRec(its.shFrame.toLocal(-ray.d));
            bcolor = its.mesh->getBSDF()->sample(bRec, sampler->next2D());
            pdfMatI = its.mesh->getBSDF()->pdf(bRec); ////PDF MAT


            pathInfo.pathThroughput *= bcolor;
                //Nuevo rayo
            rayRec = Ray3f(its.p, its.toWorld(bRec.wo));

            Intersection its2;
            if (scene->rayIntersect(rayRec, its2) && (its2.mesh->isEmitter())) {

                Normal3f n = its2.shFrame.n; //normal
                Point3f p = its2.p; //punto de intersección
                Point3f o = rayRec.o; //origen del rayo
                EmitterQueryRecord lRec2 = EmitterQueryRecord(o, p, n);
                pdfLuzI = its2.mesh->getEmitter()->pdf(lRec2);
            }
            else {
                pdfLuzI = 0;
            }

            li = LiRecursive(scene, sampler, rayRec, pathInfo);

            if ((pdfMatI + pdfLuzI) > 0)
                weightI = pdfMatI / (pdfMatI + pdfLuzI);
            else
                weightI = 0;
            //Por los espejos
            if (bRec.measure == EDiscrete)
                weightI = 1.0f;


            //Ruleta rusa
            //float probRR = 0.05;
            float probRR = std::min(pathInfo.pathThroughput.x(), 0.9f);
            if (sampler->next1D() >= probRR)
                return Color3f(0.0f);

            pathInfo.pathThroughput /= probRR;
            
            return li * weightI + ld * weightD;
     
        }
        return Color3f(0.0);
    }
    
    Color3f LiLight(const Scene* scene, Sampler* sampler, const Ray3f& ray, float& weight) const {
        float pdfLuz = 0.0f;
        float pdfMat = 0.0f;
        Intersection its;
        Color3f color = Color3f(0.0f);
        //Si no se produce una intersección de devuelve 0
        if (!scene->rayIntersect(ray, its))
            return color;

        //Si se produce:
        Normal3f n = its.shFrame.n;  //normal
        Point3f p = its.p; //punto de intersección del rayo
        Point3f o = ray.o;

        EmitterQueryRecord recE = EmitterQueryRecord(o, p, n);

        ////Añadir al color
        //Para obtener el color del segundo rayo lanzado internamente
        Color3f colorMuestra = scene->sampleEmitter(recE, sampler->next2D());//li
        pdfLuz = recE.pdf;
        ////Evaluar el material de la superficie BSDFQueryRecord eval
        BSDFQueryRecord bRec = BSDFQueryRecord(its.shFrame.toLocal(-ray.d), its.shFrame.toLocal(recE.wi), ESolidAngle);
        Color3f colorBSDF = 0;
        colorBSDF = its.mesh->getBSDF()->eval(bRec);
        pdfMat = its.mesh->getBSDF()->pdf(bRec);

        ////Multiplicar radiancia por el valor de la muestra de luz y el coseno de ambos
        float cos = Frame::cosTheta(its.shFrame.toLocal(recE.wi));

        Color3f colorDef = cos * colorBSDF * colorMuestra;
        color = colorDef;
        if ((pdfMat + pdfLuz) > 0)
            weight = pdfLuz / (pdfMat + pdfLuz);
        else
            weight = 0;

        return color;
 
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

NORI_REGISTER_CLASS(PathTracingNEE, "pathtracer_nee");
NORI_NAMESPACE_END
/*
    File added for P3
*/
