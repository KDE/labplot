/***************************************************************************
    File                 : Transform.h
    Project              : LabPlot
    Description          : transformation for mapping between scene and
                           logical coordinates of image
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "backend/datapicker/Image.h"

class Transform {
    public:
        Transform();
        QPointF mapSceneToLogical(const QPointF&,const Image::ReferencePoints&);
        QPointF mapSceneLengthToLogical(const QPointF&,const Image::ReferencePoints&);

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
