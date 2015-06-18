#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "backend/worksheet/Image.h"

class Transform
{
public:
    Transform(Image*);
    QPointF mapSceneToLogical(const QPointF&);

    Image::ReferencePoints m_points;
    Image* m_image;

    //logical coordinates
    double x[3];
    double y[3];

    //Scene coordinates
    double X[3];
    double Y[3];

private:
    bool setType();
    QPointF setOutput(const QPointF&);

};

#endif // TRANSFORM_H
