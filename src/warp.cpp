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

#include <nori/warp.h>
#include <nori/vector.h>
#include <nori/frame.h>

NORI_NAMESPACE_BEGIN

Point2f Warp::squareToUniformSquare(const Point2f &sample) {

    return sample;
}

float Warp::squareToUniformSquarePdf(const Point2f &sample) {
    return ((sample.array() >= 0).all() && (sample.array() <= 1).all()) ? 1.0f : 0.0f;
}

Point2f Warp::squareToTent(const Point2f &sample) {
    
    float xu = sample.x();
    float yu = sample.y();

    Point2f A = Point2f(-1.0f, 0.0f);
    Point2f B = Point2f(1.0f, 0.0f);
    Point2f C = Point2f(0.0f, 1.0f);

    float x = 1 - sqrt(xu);
    float y = sqrt(xu) * (1 - yu);
    float z = yu*sqrt(xu);

    Point2f point =  x*A + y*B + z*C;
    return point;
 
}
float sign(Point2f A, Point2f B, Point2f C)
{
    return (A.x() - C.x()) * (B.y() - C.y()) - (B.x() - C.x()) * (A.y() - C.y());
}


float Warp::squareToTentPdf(const Point2f &p) {

    //Analizando los signos de las baricéntricas se puede calcular si es interior o exterior del triángulo
    float s1, s2, s3;
    bool neg, pos;
    Point2f A = Point2f(-1.0f, 0.0f);
    Point2f B = Point2f(1.0f, 0.0f);
    Point2f C = Point2f(0.0f, 1.0f);
    s1 = sign(p, A, B);
    s2 = sign(p, B, C);
    s3 = sign(p, C, A);

    neg = (s1 < 0) || (s2 < 0) || (s3 < 0);
    pos = (s1 > 0) || (s2 > 0) || (s3 > 0);

    return !(neg && pos);
}

Point2f Warp::squareToUniformDisk(const Point2f &sample) {
    float xu = sample.x();
    float yu = sample.y();
    float r = sqrt(xu);
    float theta = 2.0f * M_PI * yu;
    //float r = xu;
    //float theta = (yu / xu) * (M_PI / 4.0f);

    float x = r * cos(theta);
    float y = r * sin(theta);

    return Point2f(x,y);
}

float Warp::squareToUniformDiskPdf(const Point2f &p) {
    throw NoriException("Warp::squareToUniformDiskPdf() is not yet implemented!");
}

Vector3f Warp::squareToUniformSphere(const Point2f &sample) {
    throw NoriException("Warp::squareToUniformSphere() is not yet implemented!");
}

float Warp::squareToUniformSpherePdf(const Vector3f &v) {
    throw NoriException("Warp::squareToUniformSpherePdf() is not yet implemented!");
}

Vector3f Warp::squareToUniformHemisphere(const Point2f &sample) {

    //Muestreo con probabilidad uniforme de la superficie de una semiesfera de radio unidad.
    //La semiesfera está cortada por el plano XY, por lo que el eje Z define la vertical.


    //punto 2D aleatorio ‘sample’, generado con probabilidad uniforme en
    //un cuadrado unitario(entre(0, 0) y(1, 1)).

    float xu = sample.x();
    float yu = sample.y();
    float theta = acos(1.0f - xu);
    float phi = 2.0f * M_PI * yu;

    float x = sin(theta) * cos(phi);
    float y = sin(theta) * sin(phi);
    float z = cos(theta);

    return Vector3f(x, y, z);

}

float Warp::squareToUniformHemispherePdf(const Vector3f &v) {
    float p = 1.0 / (2.0 * M_PI);
    return (v.z()>0) ? p : 0.0f;
}

Vector3f Warp::squareToCosineHemisphere(const Point2f& sample) {

    Point2f d = squareToUniformDisk(sample);
    float h = 1 - d.x() * d.x() - d.y() * d.y();
    float z = sqrt(fmaxl(0.0, h));
    return Vector3f(d.x(), d.y(), z);
}
float Warp::squareToCosineHemispherePdf(const Vector3f& v) {
    float p = v.z()*(1.0/M_PI);
    return (v.z() > 0) ? p : 0.0f;
}
Vector3f Warp::squareToBeckmann(const Point2f &sample, float alpha) {
    throw NoriException("Warp::squareToBeckmann() is not yet implemented!");
}

float Warp::squareToBeckmannPdf(const Vector3f &m, float alpha) {
    throw NoriException("Warp::squareToBeckmannPdf() is not yet implemented!");
}

NORI_NAMESPACE_END
