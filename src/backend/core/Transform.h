#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "backend/worksheet/Image.h"

class Transform {
    public:
        Transform(Image*);
        QPointF mapSceneToLogical(const QPointF &);

    private:
        bool mapTypeToCartesian();
        QPointF mapCartesianToType(const QPointF&);
        Image::ReferencePoints m_points;
        Image* m_image;

        //logical coordinates
        double x[4];
        double y[4];

        //Scene coordinates
        double X[4];
        double Y[4];

};

#endif // TRANSFORM_H
