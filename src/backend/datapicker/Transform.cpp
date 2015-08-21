/***************************************************************************
    File                 : Transform.cpp
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

#include "Transform.h"
#include "math.h"

#define PI 3.14159265

Transform::Transform() {
}

bool Transform::mapTypeToCartesian() {
    if (m_points.type == Image::LogarithmicX) {
        for(int i=0;i<3;i++){
            if (m_points.logicalPos[i].x() <= 0)
                return false;
            x[i] = log(m_points.logicalPos[i].x());
            y[i] = m_points.logicalPos[i].y();
            X[i] = m_points.scenePos[i].x();
            Y[i] = m_points.scenePos[i].y();
        }
    } else if (m_points.type == Image::LogarithmicY) {
        for(int i=0;i<3;i++){
            if (m_points.logicalPos[i].y() <= 0)
                return false;
            x[i] = m_points.logicalPos[i].x();
            y[i] = log(m_points.logicalPos[i].y());
            X[i] = m_points.scenePos[i].x();
            Y[i] = m_points.scenePos[i].y();
        }
    } else if (m_points.type == Image::PolarInDegree) {
        for(int i=0;i<3;i++){
            if (m_points.logicalPos[i].x() < 0)
                return false;
            x[i] = m_points.logicalPos[i].x()*cos(m_points.logicalPos[i].y()*PI / 180.0);
            y[i] = m_points.logicalPos[i].x()*sin(m_points.logicalPos[i].y()*PI / 180.0);
            X[i] = m_points.scenePos[i].x();
            Y[i] = m_points.scenePos[i].y();
        }
    } else if (m_points.type == Image::PolarInRadians) {
        for(int i=0;i<3;i++){
            if (m_points.logicalPos[i].x() < 0)
                return false;
            x[i] = m_points.logicalPos[i].x()*cos(m_points.logicalPos[i].y());
            y[i] = m_points.logicalPos[i].x()*sin(m_points.logicalPos[i].y());
            X[i] = m_points.scenePos[i].x();
            Y[i] = m_points.scenePos[i].y();
        }
    } else {
        for(int i=0;i<3;i++){
            x[i] = m_points.logicalPos[i].x();
            y[i] = m_points.logicalPos[i].y();
            X[i] = m_points.scenePos[i].x();
            Y[i] = m_points.scenePos[i].y();
        }
    }
    return true;
}

QPointF Transform::mapSceneToLogical(const QPointF& scenePoint, const Image::ReferencePoints& axisPoints) {
    m_points = axisPoints;

    X[3] = scenePoint.x();
    Y[3] = scenePoint.y();

    if (mapTypeToCartesian()){
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
        return mapCartesianToType(QPointF(x[3], y[3]));
    }
    return QPointF();
}

QPointF Transform::mapSceneLengthToLogical(const QPointF& errorSpan, const Image::ReferencePoints& axisPoints) {
    return mapSceneToLogical(errorSpan, axisPoints) - mapSceneToLogical(QPointF(0,0), axisPoints);
}

QPointF Transform::mapCartesianToType(const QPointF& point){
    if (m_points.type == Image::LogarithmicX) {
        return QPointF(exp(point.x()), point.y());
    } else if (m_points.type == Image::LogarithmicY) {
        return QPointF(point.x(), exp(point.y()));
    } else if (m_points.type == Image::PolarInDegree) {
        double r = sqrt(point.x()*point.x() + point.y()*point.y());
        double angle = atan(point.y()*180/(point.x()*PI));
        return QPointF(r, angle);
    } else if (m_points.type == Image::PolarInRadians) {
        double r = sqrt(point.x()*point.x() + point.y()*point.y());
        double angle = atan(point.y()/point.x());
        return QPointF(r, angle);
    } else {
        return point;
    }
}

