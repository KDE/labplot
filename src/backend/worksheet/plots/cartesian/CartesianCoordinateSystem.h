/***************************************************************************
    File                 : CartesianCoordinateSystem.h
    Project              : LabPlot/SciDAVis
    Description          : Cartesian coordinate system for plots.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2012 by Alexander Semke (alexander.semke*web.de)
                           (replace * with @ in the email addresses) 
                           
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

#ifndef CARTESIANCOORDINATESYSTEM_H
#define CARTESIANCOORDINATESYSTEM_H

#include "backend/worksheet/plots/AbstractCoordinateSystem.h"
#include "backend/lib/macros.h"
#include "backend/lib/Interval.h"

#include <vector>

class CartesianPlot;
class CartesianCoordinateSystemPrivate;
class CartesianCoordinateSystemSetScalePropertiesCmd;
class CartesianCoordinateSystem: public AbstractCoordinateSystem {

	public:
		class Scale {
			public:
				static const double LIMIT_MAX = 1e15;
				static const double LIMIT_MIN = -1e15;

				virtual ~Scale();
				enum ScaleType {
					ScaleLinear,
					ScaleLog,
				};
				
				static Scale *createScale(ScaleType type, const Interval<double> &interval, double a, double b, double c);
				static Scale *createLinearScale(const Interval<double> &interval, double sceneStart, double sceneEnd, 
					double logicalStart, double logicalEnd);
				static Scale *createLogScale(const Interval<double> &interval, double sceneStart, double sceneEnd, 
					double logicalStart, double logicalEnd, double base);

				virtual void getProperties(ScaleType *type = NULL, Interval<double> *interval = NULL, 
						double *a = NULL, double *b = NULL, double *c = NULL) const;

				virtual bool map(double *value) const = 0;
				virtual bool inverseMap(double *value) const = 0;
				virtual int direction() const = 0;
				virtual void getPropertiesOnResize(double ratio, 
					ScaleType *type, Interval<double> *interval, double *a, double *b, double *c) const = 0;
			
				// changing properties is done via this command:
				friend class CartesianCoordinateSystemSetScalePropertiesCmd;
			protected:
				Scale(ScaleType type, const Interval<double> &interval, double a, double b, double c);
				ScaleType m_type;
				Interval<double> m_interval;
				double m_a;
				double m_b;
				double m_c;
		};

		CartesianCoordinateSystem(CartesianPlot*);
		virtual ~CartesianCoordinateSystem();

		virtual QList<QPointF> mapLogicalToScene(const QList<QPointF> &points, const MappingFlags &flags = DefaultMapping) const;
		void mapLogicalToScene(const QList<QPointF>& logicalPoints, QList<QPointF>& scenePoints, std::vector<bool>& visiblePoints, const MappingFlags& flags = DefaultMapping) const;
		virtual QList<QPointF> mapSceneToLogical(const QList<QPointF> &points, const MappingFlags &flags = DefaultMapping) const;
		virtual QList<QLineF> mapLogicalToScene(const QList<QLineF> &lines, const MappingFlags &flags = DefaultMapping) const;

		virtual void handlePageResize(double horizontalRatio, double verticalRatio);

		int xDirection() const;
		int yDirection() const;
		bool setXScales(const QList<Scale *> &scales);
		QList<Scale *> xScales() const;
		bool setYScales(const QList<Scale *> &scales);
		QList<Scale *> yScales() const;
		
		virtual void save(QXmlStreamWriter *) const;
		virtual bool load(XmlStreamReader *);

	private:
		void init();
		CartesianCoordinateSystemPrivate* d;
};

#endif
