/*
    File added for exercise P2
*/

#include <nori/emitter.h>
#include <nori/mesh.h>
#include <nori/warp.h>
NORI_NAMESPACE_BEGIN

/**
 * \brief Simple area emitter with uniform emittance
 */
    class AreaEmitter : public Emitter {
    public:
        AreaEmitter(const PropertyList& propList) : m_mesh(NULL) {
            /* Emitted radiance */
            m_radiance = propList.getColor("radiance");

        }

        Color3f sample(EmitterQueryRecord& lRec, const Point2f& sample) const {
 
            //1. Obtener un punto aleatorio en la malla asociada al emisor
            Point3f p = lRec.p;
            m_mesh->samplePoint(lRec, sample);
            lRec.o = p;
            lRec.wi = (lRec.p - lRec.o).normalized();
            


            //3. transformar dicha probabilidad al ángulo sólido del punto de destino de la luz
           // lRec.dist = (lRec.p - lRec.o).squaredNorm();
            //lRec.pdf = pdf(lRec);
            float dir = lRec.n.dot(-lRec.wi); //producto escalar de ambos vectores
            lRec.dist = (lRec.p - lRec.o).squaredNorm();
            if (dir > 0.0f) {
                float pdfSolido = (lRec.pdf * (lRec.dist)) / dir;
                lRec.pdf = pdfSolido;
            }
            else {
                lRec.pdf = 0;
            }
            //4. evaluar la radiancia y ponderarla con la inversa de la probabilidad. 
            //Además, hay que comprobar la orientación relativa entre el rayo incidente en la luz y la
            //normal de la malla.Si el rayo incide por la parte interior de la malla, simplemente se
             //devuelve radiancia nula.
             
            Color3f sampleDef;
            if((lRec.pdf > 0.0f)){
                 sampleDef = eval(lRec) / lRec.pdf;   
            }
            else {
                sampleDef = 0;
                lRec.pdf = 0;
            }
            return sampleDef;
        }

        float pdf(const EmitterQueryRecord& lRec) const {
            float dir = lRec.n.dot(-lRec.wi); //producto escalar de ambos vectores
            
            if (dir > 0.0f) {
                float pdfSolido = (lRec.pdf * (lRec.dist)) / dir;
                return pdfSolido;
            }
            else {
                return 0;
            }
           
        }

        Color3f eval(const EmitterQueryRecord& lRec) const {
            //devolver probabilidad de la muestra que me dan de un Emmiterqueryrecord
            //m_radiance
            //Comprobar si el rayo incidente está alineado con la normal del emmiter, si es paralela
            Color3f color=0;
            Normal3f n = lRec.n; //normal del emiter
            Vector3f wi = lRec.wi; //rayo incidente
            float dir = n.dot(wi); //producto escalar de ambos vectores
            //Si el producto escalar da un resultado menor que 0, el color resultante 
            if (dir < 0) {
                color = m_radiance;
            }
            else {
                color = 0;
            }
            return color;
        }

        void setParent(NoriObject* object) {
            if (object->getClassType() != EMesh)
                throw NoriException("AreaEmitter: attached to a non-mesh object!");
            m_mesh = static_cast<Mesh*>(object);
        }

        std::string toString() const {
            return tfm::format("AreaEmitter[radiance=%s]", m_radiance.toString());
        }

    /// Para AP 2
     Color3f getRadiance() const{ return m_radiance;}
     float getArea() const {return m_area;}
     void setArea(float area) {m_area = area;}

    private:
        Color3f m_radiance;
        Mesh* m_mesh;

        float m_area = 0;
};

NORI_REGISTER_CLASS(AreaEmitter, "area");
NORI_NAMESPACE_END