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

#pragma once

#include <nori/object.h>

NORI_NAMESPACE_BEGIN

struct EmitterQueryRecord {
   
    // Normal 
    Normal3f n;
    //Dirección incidente
    Vector3f wi;
    // punto origen
    Point3f o;
    // punto final
    Point3f p;
    //Emitter
    Emitter* emitter;
    /// Probabilidad
    float pdf;

    float dist;

    /// Constructores
    EmitterQueryRecord() { }
    EmitterQueryRecord(const Normal3f& n, const Vector3f& wi) : n(n), wi(wi) { }
    EmitterQueryRecord(const Point3f& o, const Point3f& p, const Normal3f& n) :
        o(o), p(p), n(n) {
        dist = (p - o).norm();
        wi = (p - o).normalized(); //Normalizar, lo importante es la dirección no la magnitud
    }

};

/**
 * \brief Superclass of all emitters
 */
class Emitter : public NoriObject {
public:
    /**
 * \brief Sample the BSDF and return the importance weight (i.e. the
 * value of the BSDF * cos(theta_o) divided by the probability density
 * of the sample with respect to solid angles).
 *
 * \param bRec    A BSDF query record
 * \param sample  A uniformly distributed sample on \f$[0,1]^2\f$
 *
 * \return The BSDF value divided by the probability density of the sample
 *         sample. The returned value also includes the cosine
 *         foreshortening factor associated with the outgoing direction,
 *         when this is appropriate. A zero value means that sampling
 *         failed.
 */
    virtual Color3f sample(EmitterQueryRecord& eRec, const Point2f& sample) const = 0;

    /**
     * \brief Evaluate the BSDF for a pair of directions and measure
     * specified in \code bRec
     *
     * \param bRec
     *     A record with detailed information on the BSDF query
     * \return
     *     The BSDF value, evaluated for each color channel
     */
    virtual Color3f eval(const EmitterQueryRecord& eRec) const = 0;

    /**
     * \brief Compute the probability of sampling \c bRec.wo
     * (conditioned on \c bRec.wi).
     *
     * This method provides access to the probability density that
     * is realized by the \ref sample() method.
     *
     * \param bRec
     *     A record with detailed information on the BSDF query
     *
     * \return
     *     A probability/density value expressed with respect
     *     to the specified measure
     */

    virtual float pdf(const EmitterQueryRecord& eRec) const = 0;
    /**
     * \brief Return the type of object (i.e. Mesh/Emitter/etc.) 
     * provided by this instance
     * */

    virtual Color3f getRadiance()  const = 0;
    virtual float getArea() const = 0;
    virtual void setArea(float area) = 0;
    //Usar eval y getclasstype, crear las funciones
    //Crear clase emiterquertrecord guarda propiedades del emiter
    EClassType getClassType() const { return EEmitter; }
};

NORI_NAMESPACE_END
