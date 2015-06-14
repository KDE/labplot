#include "Transform.h"
#include "math.h"

Transform::Transform() {
}

bool Transform::setType(const Image::GraphType& type) {
    if (type == Image::Cartesian) {
        return cartesianTransformation();
    } else if (type == Image::Logarithmic) {
        return logarithmicTransformation();
    } else if (type == Image::Polar) {
        return polarTransformation();
    }

    return false;
}

QPointF Transform::mapSceneToLogical(const Image::ReferencePoints& points, const QPointF& scenePoint) {
    m_points = points;
    X[3] = scenePoint.x();
    Y[3] = scenePoint.y();

    if (setType(points.type)){
        double tan;
        double sin;
        double cos;
        double scaleOfX;
        double scaleOfY;
        if(((Y[1] - Y[0])*(x[2] - x[0]) - (Y[2] - Y[0])*(x[1] - x[0]))!=0){
            tan = ((X[1] - X[0])*(x[2] - x[0]) - (X[2] - X[0])*(x[1] - x[0]))/((Y[1] - Y[0])*(x[2] - x[0]) - (Y[2] - Y[0])*(x[1] - x[0]));
            sin = tan/sqrt(1 + tan*tan);
            cos = sqrt(1 - sin*sin);
        } else{
            sin=1;
            cos=0;
        }
        if((x[1] - x[0])!=0){
            scaleOfX = (x[1] - x[0])/((X[1] - X[0])*cos - (Y[1] - Y[0])*sin);
        } else{
            scaleOfX = (x[2] - x[0])/((X[2] - X[0])*cos - (Y[2] - Y[0])*sin);
        }
        if((y[1]-y[0])!=0){
            scaleOfY = (y[1] - y[0])/((X[1] - X[0])*sin + (Y[1] - Y[0])*cos);
        } else{
            scaleOfY = (y[2] - y[0])/((X[2] - X[0])*sin + (Y[2] - Y[0])*cos);
        }
        x[3] = x[0] + (((X[3] - X[0])*cos - (Y[3] - Y[0])*sin)*scaleOfX);
        y[3] = y[0] + (((X[3] - X[0])*sin + (Y[3] - Y[0])*cos)*scaleOfY);
        return QPointF(x[3], y[3]);
    }
    return QPointF();
}

bool Transform::logarithmicTransformation() {
    for(int i=0;i<3;i++){
        if (m_points.logicalPos[i].x() <= 0)
            return false;
        x[i] = log(m_points.logicalPos[i].x());
        y[i] = m_points.logicalPos[i].y();
        X[i] = m_points.scenePos[i].x();
        Y[i] = m_points.scenePos[i].y();
    }
    return true;
}

bool Transform::polarTransformation() {
    for(int i=0;i<3;i++){
        if (m_points.logicalPos[i].x() < 0)
            return false;
        x[i] = m_points.logicalPos[i].x()*cos(m_points.logicalPos[i].y());
        y[i] = m_points.logicalPos[i].x()*sin(m_points.logicalPos[i].y());
        X[i] = m_points.scenePos[i].x();
        Y[i] = m_points.scenePos[i].y();
    }
    return true;
}

bool Transform::cartesianTransformation() {
    for(int i=0;i<3;i++){
        x[i] = m_points.logicalPos[i].x();
        y[i] = m_points.logicalPos[i].y();
        X[i] = m_points.scenePos[i].x();
        Y[i] = m_points.scenePos[i].y();
    }
    return true;
}
