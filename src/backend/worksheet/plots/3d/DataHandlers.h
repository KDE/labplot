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

#ifndef DATAHANDLERS_H
#define DATAHANDLERS_H

#include <KUrl>

#include <vtkSmartPointer.h>

class AbstractColumn;
class Matrix;

class vtkActor;

class IDataHandler : public QObject {
		Q_OBJECT
	public:
		virtual ~IDataHandler() {}

		virtual vtkSmartPointer<vtkActor> actor() = 0;

	signals:
		void parametersChanged();
};

class MatrixDataHandler : public IDataHandler {
	public:
		MatrixDataHandler();

		vtkSmartPointer<vtkActor> actor();
		void setMatrix(Matrix* matrix);

	private:
		Matrix* matrix;
};

class SpreadsheetDataHandler : public IDataHandler {
	public:
		SpreadsheetDataHandler();

		vtkSmartPointer<vtkActor> actor();
		void setXColumn(AbstractColumn *column);
		void setYColumn(AbstractColumn *column);
		void setZColumn(AbstractColumn *column);

		void setNodeColumn(int node, AbstractColumn *column);

	private:
		AbstractColumn *xColumn;
		AbstractColumn *yColumn;
		AbstractColumn *zColumn;
		AbstractColumn *nodeColumn[3];
};

class FileDataHandler : public IDataHandler {
	public:
		vtkSmartPointer<vtkActor> actor();
		void setFile(const KUrl& path);

	private:
		KUrl path;
};

class DemoDataHandler : public IDataHandler {
	public:
		vtkSmartPointer<vtkActor> actor();
};

#endif