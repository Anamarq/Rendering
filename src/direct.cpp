#include <nori/integrator.h>
#include <nori/scene.h>
#include <nori/warp.h>
#include <nori/mesh.h>
#include <nori/bsdf.h>
#include <nori/emitter.h>
#include <nori/scene.h>

NORI_NAMESPACE_BEGIN



class DirectIntegrator : public Integrator {
public:
    enum class Integrador { Light=1, Material=2, MIS=3};
    Integrador integrador;
    DirectIntegrator(const PropertyList& props) : mesh(NULL) {
        /* Emitted radiance */
        std::string Tintegrador = props.getString("sampling");
       /* switch (Tintegrador)
        {
        case "light":
            integrador = Integrador::Ligth;
            break;
        default:
            break;
        }*/
        if (Tintegrador == "light") 
            integrador = Integrador::Light;
        else if (Tintegrador == "material")
            integrador = Integrador::Material;
        else if (Tintegrador == "MIS")
            integrador = Integrador::MIS;

    }
    Color3f Li(const Scene* scene, Sampler* sampler, const Ray3f& ray) const {
        /* Find the surface that is visible in the requested direction */
        //Devolver color de cada pixel de la escena contra el cual ha incidido el rayo de entrada ray
        //Color se almacena en una clae llamada BSDF.
        //rayIntersect devuelve intersección con una mesh la escena que tiene una propiedad que es el color de su propio material almacenada en una estructura llamada BSDF
        //Con ese material se crea una estructura para almacenar las propiedades del material contra el que se ha chocado
        //Para ello se usa el queryRecord. Se inicializa la estructura con el constructor
        //Calcular el color del material a partir del BSDF cogiendo una sampler a partir del query record
        //Sample devuelve el píxel contra el cual el rayo ha chocado. 



        //Conseguir la intersección del rayo con la escena con rayIntersect.
       


        if (integrador == Integrador::Material) {
            //comprobar si es emitter
            Intersection its;
            Color3f color = Color3f(0.0f);
            if (!scene->rayIntersect(ray, its))
                return color;

            if (!its.mesh->isEmitter()) {
                //Estructura para almacenar las propiedades del material. Pasarle la dirección del rayo de incidencia
                 //en coordenadas locales.
                BSDFQueryRecord rec = BSDFQueryRecord(its.shFrame.toLocal(-ray.d));
                //Calcular el color del material a partir del BSDF
                color = its.mesh->getBSDF()->sample(rec, sampler->next2D());

                //Para el segundo rayo: Sale de la psosición del objeto con el que ha chocado con dirección
                //de salida wo en coordenadas globales
                Intersection its2;
                Ray3f ray2 = Ray3f(its.p, its.shFrame.toWorld(rec.wo));
                if (!scene->rayIntersect(ray2, its2))
                    return 0;
                if (its2.mesh->isEmitter()) {
                    //BSDFQueryRecord rec = BSDFQueryRecord(its2.shFrame.toLocal(-ray2.d));
                     //Calcular el nuevo color con el emitter
                    //Evaluar radiancia incidente
                    Normal3f n = its2.shFrame.n;  //normal
                    Point3f p = its2.p; //punto de intersección del rayo
                    Point3f o = ray2.o; //origen del rayo desde el objeto del que sale
                    //Estructura para almacenar propiedades emisivas del material
                    EmitterQueryRecord recE = EmitterQueryRecord(o, p, n);
                    //Añadir al color
                    color *= its2.mesh->getEmitter()->eval(recE);
                }
                else {
                    color = 0;
                }

            }
            else {
                //Es emitter
                Normal3f n = its.shFrame.n; //normal
                Point3f p = its.p; //punto de intersección
                Point3f o = ray.o; //origen del rayo
                //Estructura para almacenar propiedades emisivas del material
                EmitterQueryRecord rec = EmitterQueryRecord(o, p, n);
                //Calcular el color del material a partir del emitter
                color = its.mesh->getEmitter()->eval(rec);
                
            }
            return color;
        }
        else if (integrador == Integrador::Light) {

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
            EmitterQueryRecord recE1 = EmitterQueryRecord(o, p, n);


            ////Añadir al color
            //Para obtener el color del segundo rayo lanzado internamente
            Color3f colorMuestra = scene->sampleEmitter(recE, sampler->next2D());//li

            ////Evaluar el material de la superficie BSDFQueryRecord eval
            BSDFQueryRecord bRec = BSDFQueryRecord(its.shFrame.toLocal(-ray.d),its.shFrame.toLocal(recE.wi), ESolidAngle);
            Color3f colorBSDF = 0; 
            colorBSDF = its.mesh->getBSDF()->eval(bRec);

            ////Multiplicar radiancia por el valor de la muestra de luz y el coseno de ambos
            float cos = Frame::cosTheta(its.shFrame.toLocal(recE.wi));
            
            Color3f colorDef = cos * colorBSDF *colorMuestra;
            color = colorDef;
               
            if (its.mesh->isEmitter()) {
                Color3f Le = 0;
                Le = its.mesh->getEmitter()->eval(recE1);
                //Le+BSDF*Li*cos
                color =(Le + colorDef);
            }    
  
            return color;
        }
        else if (integrador == Integrador::MIS) {
            //Sampleo de material y de luz
            //Color que devuelve el integrador de material y el de luz
            //probabilidad de cada parte
            //Luces:Coger la pdf del emiterQueryRecord .pdf
            //Material: Función del bsdf query record que se llama pdf, llamar a esa función y devuelve el pdf de ese material
            //MIS=(pdfMat*Colormat)+pdfluz*colorluz/pdfmat+pdflu, devuelve color
            //A ese color se le suma la Le. Al devulverel color del aterial y la luz evitar que se sume en la Le. La Le se suma al final
            //Comprobar si ha habido una intersección y si es una luz devolver el valor de la luz
            //

            Intersection its;
            Color3f color = Color3f(0.0f);
            //Si no se produce una intersección de devuelve 0
            if (!scene->rayIntersect(ray, its))
                return color;

            //Si se produce:
            Normal3f n = its.shFrame.n;  //normal
            Point3f p = its.p; //punto de intersección del rayo
            Point3f o = ray.o;

            EmitterQueryRecord lrecL = EmitterQueryRecord(o, p, n);
            EmitterQueryRecord recE = EmitterQueryRecord(o, p, n);
            Color3f colorLuz=0;
            Color3f colorMat=0;
            float pdfLuz = 0;
            float pdfMat = 0;
            Color3f MIS = 0;
            
            
            //Sampleo Mat
            if (!its.mesh->isEmitter()) {
                //Estructura para almacenar las propiedades del material. Pasarle la dirección del rayo de incidencia
                    //en coordenadas locales.
                BSDFQueryRecord brecM = BSDFQueryRecord(its.shFrame.toLocal(-ray.d));
                pdfMat = its.mesh->getBSDF()->pdf(brecM);
                //Calcular el color del material a partir del BSDF
                colorMat = its.mesh->getBSDF()->sample(brecM, sampler->next2D());

                //Para el segundo rayo: Sale de la psosición del objeto con el que ha chocado con dirección
                //de salida wo en coordenadas globales
                Intersection its2;
                Ray3f ray2 = Ray3f(its.p, its.shFrame.toWorld(brecM.wo));
                if (!scene->rayIntersect(ray2, its2))
                    return 0;
                if (its2.mesh->isEmitter()) {
                        //Calcular el nuevo color con el emitter
                    //Evaluar radiancia incidente
                    Normal3f n = its2.shFrame.n;  //normal
                    Point3f p = its2.p; //punto de intersección del rayo
                    Point3f o = ray2.o; //origen del rayo desde el objeto del que sale
                    //Estructura para almacenar propiedades emisivas del material
                    EmitterQueryRecord recE = EmitterQueryRecord(o, p, n);
                    //Añadir al color
                    colorMat *= its2.mesh->getEmitter()->eval(recE);
                }
                else {
                    colorMat = 0;
                }
                
            }


            //LUZ
            ////Añadir al color
            //Para obtener el color del segundo rayo lanzado internamente
            Color3f colorMuestra = scene->sampleEmitter(lrecL, sampler->next2D());//li
             ////Evaluar el material de la superficie BSDFQueryRecord eval
            BSDFQueryRecord bRec = BSDFQueryRecord(its.shFrame.toLocal(-ray.d), its.shFrame.toLocal(lrecL.wi), ESolidAngle);
            Color3f colorM = 0;
            colorM = its.mesh->getBSDF()->eval(bRec);
            ////Multiplicar radiancia por el valor de la muestra de luz y el coseno de ambos
            float cos = Frame::cosTheta(its.shFrame.toLocal(lrecL.wi));

            colorLuz = cos * colorM * colorMuestra;
            pdfLuz = lrecL.pdf;

            if ((pdfLuz + pdfMat) != 0) {
                MIS = (pdfMat * colorMat + pdfLuz * colorLuz) / (pdfMat + pdfLuz);
                color = MIS;
            }
            //Añadir Le
            if (its.mesh->isEmitter()) {
                Color3f Le = 0;
                Le = its.mesh->getEmitter()->eval(recE);
                color = (Le + MIS);
            }

            return color;

        }
    }

    std::string toString() const {
        return "DirectIntegrator[]";
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

NORI_REGISTER_CLASS(DirectIntegrator, "direct");
NORI_NAMESPACE_END

    
