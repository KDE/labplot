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

#include "Surface3D.h"
#include "backend/core/AbstractAspect.h"

#include <QColor>

#include <vtkSmartPointer.h>

class KUrl;

class AbstractColumn;
class Matrix;
class MatrixDataHandlerPrivate;
class SpreadsheetDataHandlerPrivate;
class FileDataHandlerPrivate;

class vtkActor;
class vtkPolyData;

struct DataHandlerConfig {
	Surface3D::VisualizationType type;
	Surface3D::ColorFilling colorFilling;
	QColor color;
	double opacity;
};

class IDataHandler : public AbstractAspect {
		Q_OBJECT
	public:
		IDataHandler();
		virtual ~IDataHandler() {}

		vtkSmartPointer<vtkActor> actor(const DataHandlerConfig& config);

		void update();

	protected:
		vtkSmartPointer<vtkActor> mapData(vtkPolyData* data);
		virtual vtkSmartPointer<vtkPolyData> generateData() = 0;
		vtkSmartPointer<vtkPolyData> extractEdges(vtkPolyData* data) const;
		void makeColorElevation(vtkPolyData* polydata);

	signals:
		void parametersChanged();
};

class MatrixDataHandler : public IDataHandler {
		Q_OBJECT
		Q_DISABLE_COPY(MatrixDataHandler)
		Q_DECLARE_PRIVATE(MatrixDataHandler)
	public:
		MatrixDataHandler();
		virtual ~MatrixDataHandler();

		POINTER_D_ACCESSOR_DECL(const Matrix, matrix, Matrix);
		const QString& matrixPath() const;

		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);

		typedef MatrixDataHandler BaseClass;
		typedef MatrixDataHandlerPrivate Private;

	private:
		vtkSmartPointer<vtkPolyData> generateData();

	private slots:
		void matrixAboutToBeRemoved(const AbstractAspect*);

	signals:
		friend class MatrixDataHandlerSetMatrixCmd;
		void matrixChanged(const Matrix*);

	private:
		const QScopedPointer<MatrixDataHandlerPrivate> d_ptr;
};

class SpreadsheetDataHandler : public IDataHandler {
		Q_OBJECT
		Q_DISABLE_COPY(SpreadsheetDataHandler)
		Q_DECLARE_PRIVATE(SpreadsheetDataHandler)
	public:
		SpreadsheetDataHandler();
		virtual ~SpreadsheetDataHandler();

		POINTER_D_ACCESSOR_DECL(const AbstractColumn, xColumn, XColumn);
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, yColumn, YColumn);
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, zColumn, ZColumn);

		POINTER_D_ACCESSOR_DECL(const AbstractColumn, firstNode, FirstNode);
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, secondNode, SecondNode);
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, thirdNode, ThirdNode);

		const QString& xColumnPath() const;
		const QString& yColumnPath() const;
		const QString& zColumnPath() const;

		const QString& firstNodePath() const;
		const QString& secondNodePath() const;
		const QString& thirdNodePath() const;

		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);

		typedef SpreadsheetDataHandler BaseClass;
		typedef SpreadsheetDataHandlerPrivate Private;

	private:
		vtkSmartPointer<vtkPolyData> generateData();

	private slots:
		void xColumnAboutToBeRemoved(const AbstractAspect*);
		void yColumnAboutToBeRemoved(const AbstractAspect*);
		void zColumnAboutToBeRemoved(const AbstractAspect*);

		void firstNodeAboutToBeRemoved(const AbstractAspect*);
		void secondNodeAboutToBeRemoved(const AbstractAspect*);
		void thirdNodeAboutToBeRemoved(const AbstractAspect*);

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

	private:
		const QScopedPointer<SpreadsheetDataHandlerPrivate> d_ptr;
};

class FileDataHandler : public IDataHandler {
		Q_OBJECT
		Q_DISABLE_COPY(FileDataHandler)
		Q_DECLARE_PRIVATE(FileDataHandler)
	public:
		FileDataHandler();
		virtual ~FileDataHandler();

		CLASS_D_ACCESSOR_DECL(KUrl, file, File);

		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);

		typedef FileDataHandler BaseClass;
		typedef FileDataHandlerPrivate Private;

	private:
		vtkSmartPointer<vtkPolyData> generateData();

	signals:
		friend class FileDataHandlerSetFileCmd;
		void pathChanged(const KUrl&);

	private:
		const QScopedPointer<FileDataHandlerPrivate> d_ptr;

};

class DemoDataHandler : public IDataHandler {
		Q_DISABLE_COPY(DemoDataHandler)
	public:
		DemoDataHandler();

	private:
		vtkSmartPointer<vtkPolyData> generateData();
};

#endif