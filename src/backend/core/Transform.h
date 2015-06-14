#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "backend/worksheet/Image.h"

class Transform
{
public:
    Transform();
    bool setType(const Image::GraphType&);
    QPointF mapSceneToLogical(const Image::ReferencePoints& , const QPointF&);

    Image::ReferencePoints m_points;
    //logical coordinates
    double x[3];
    double y[3];

    //Scene coordinates
    double X[3];
    double Y[3];

private:

    bool logarithmicTransformation();
    bool polarTransformation();
    bool cartesianTransformation();
};

#endif // TRANSFORM_H
