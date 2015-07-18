/***************************************************************************
    File                 : DataHandlers.h
    Project              : LabPlot
    Description          : 3D plot data handlers
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

#ifndef PLOT3D_DATAHANDLERS_H
#define PLOT3D_DATAHANDLERS_H

#include "Plot3D.h"

#include <vtkSmartPointer.h>

class KUrl;

class AbstractColumn;
class Matrix;
class BaseDataHandlerPrivate;
class MatrixDataHandlerPrivate;
class SpreadsheetDataHandlerPrivate;
class FileDataHandlerPrivate;

class vtkActor;

class IDataHandler : public AbstractAspect {
		Q_OBJECT
	public:
		IDataHandler(BaseDataHandlerPrivate *d);
		virtual ~IDataHandler();

		vtkSmartPointer<vtkActor> actor(Plot3D::VisualizationType type);

		void update();

	private:
		virtual vtkSmartPointer<vtkActor> trianglesActor() = 0;

	signals:
		void parametersChanged();

	protected:
		const QScopedPointer<BaseDataHandlerPrivate> d_ptr;
};

class MatrixDataHandler : public IDataHandler {
		Q_OBJECT
		Q_DISABLE_COPY(MatrixDataHandler)
		Q_DECLARE_PRIVATE(MatrixDataHandler)
	public:
		MatrixDataHandler();
		virtual ~MatrixDataHandler();

		void setMatrix(const Matrix* matrix);

		typedef MatrixDataHandler BaseClass;
		typedef MatrixDataHandlerPrivate Private;

	private:
		vtkSmartPointer<vtkActor> trianglesActor();

	signals:
		friend class MatrixDataHandlerSetMatrixCmd;
		void matrixChanged(const Matrix*);
};

class SpreadsheetDataHandler : public IDataHandler {
		Q_OBJECT
		Q_DISABLE_COPY(SpreadsheetDataHandler)
		Q_DECLARE_PRIVATE(SpreadsheetDataHandler)
	public:
		SpreadsheetDataHandler();
		virtual ~SpreadsheetDataHandler();

		void setXColumn(const AbstractColumn *column);
		void setYColumn(const AbstractColumn *column);
		void setZColumn(const AbstractColumn *column);

		void setFirstNode(const AbstractColumn *column);
		void setSecondNode(const AbstractColumn *column);
		void setThirdNode(const AbstractColumn *column);

		typedef SpreadsheetDataHandler BaseClass;
		typedef SpreadsheetDataHandlerPrivate Private;

	private:
		vtkSmartPointer<vtkActor> trianglesActor();

	signals:
		friend class SpreadsheetDataHandlerSetXColumnCmd;
		friend class SpreadsheetDataHandlerSetYColumnCmd;
		friend class SpreadsheetDataHandlerSetZColumnCmd;
		friend class SpreadsheetDataHandlerSetFirstNodeCmd;
		friend class SpreadsheetDataHandlerSetSecondNodeCmd;
		friend class SpreadsheetDataHandlerSetThirdNodeCmd;
		void xColumnChanged(const AbstractColumn*);
		void yColumnChanged(const AbstractColumn*);
		void zColumnChanged(const AbstractColumn*);
		void firstNodeChanged(const AbstractColumn*);
		void secondNodeChanged(const AbstractColumn*);
		void thirdNodeChanged(const AbstractColumn*);
};

class FileDataHandler : public IDataHandler {
		Q_OBJECT
		Q_DISABLE_COPY(FileDataHandler)
		Q_DECLARE_PRIVATE(FileDataHandler)
	public:
		FileDataHandler();
		virtual ~FileDataHandler();

		void setFile(const KUrl& path);

		typedef FileDataHandler BaseClass;
		typedef FileDataHandlerPrivate Private;

	private:
		vtkSmartPointer<vtkActor> trianglesActor();

	signals:
		friend class FileDataHandlerSetFileCmd;
		void pathChanged(const KUrl&);
};

class DemoDataHandler : public IDataHandler {
		Q_DISABLE_COPY(DemoDataHandler)
	public:
		DemoDataHandler();

	private:
		vtkSmartPointer<vtkActor> trianglesActor();
};

#endif