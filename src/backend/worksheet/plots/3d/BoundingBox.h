/***************************************************************************
    File                 : BoundingBox
    Project              : LabPlot
    Description          : Bounding Box
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Minh Ngo (minh@fedoraproject.org)

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
#ifndef PLOT3D_BOUNDINGBOX_H
#define PLOT3D_BOUNDINGBOX_H

#include <cmath>

#include <vtkBoundingBox.h>

class BoundingBox : public vtkBoundingBox {
	public:
		inline BoundingBox()
			: vtkBoundingBox() {}
		inline BoundingBox(double bounds[6]) : vtkBoundingBox(bounds) {}
		inline BoundingBox(double xMin, double xMax,
				double yMin, double yMax,
				double zMin, double zMax) : vtkBoundingBox(xMin, xMax, yMin, yMax, zMin, zMax) {}

		inline void setXMin(double val) { MinPnt[0] = val; }
		inline void setXMax(double val) { MaxPnt[0] = val; }
		inline void setYMin(double val) { MinPnt[1] = val; }
		inline void setYMax(double val) { MaxPnt[1] = val; }
		inline void setZMin(double val) { MinPnt[2] = val; }
		inline void setZMax(double val) { MaxPnt[2] = val; }

		inline double xMin() const { return MinPnt[0]; }
		inline double xMax() const { return MaxPnt[0]; }
		inline double yMin() const { return MinPnt[1]; }
		inline double yMax() const { return MaxPnt[1]; }
		inline double zMin() const { return MinPnt[2]; }
		inline double zMax() const { return MaxPnt[2]; }

		inline double* getBounds() const {
			double* mutableBounds = const_cast<double*>(bounds);
			vtkBoundingBox::GetBounds(mutableBounds);
			return mutableBounds;
		}

		inline bool isInitialized() const {
			return !(MinPnt[0] == VTK_DOUBLE_MAX  && MinPnt[1] == VTK_DOUBLE_MAX && MinPnt[2] == VTK_DOUBLE_MAX
					&& MaxPnt[0] == VTK_DOUBLE_MIN && MaxPnt[1] == VTK_DOUBLE_MIN && MaxPnt[2] == VTK_DOUBLE_MIN);
		}
	private:
		double bounds[6];
};

#endif