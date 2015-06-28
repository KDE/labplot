#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "backend/worksheet/Image.h"

class Transform {
    public:
        Transform();
        QPointF mapSceneToLogical(const QPointF&,const Image::ReferencePoints&);

    private:
        bool mapTypeToCartesian();
        QPointF mapCartesianToType(const QPointF&);
        Image::ReferencePoints m_points;

        //logical coordinates
        double x[4];
        double y[4];

        //Scene coordinates
        double X[4];
        double Y[4];

};

#endif // TRANSFORM_H
