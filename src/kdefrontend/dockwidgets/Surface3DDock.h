/***************************************************************************
    File                 : Surface3DDock.h
    Project              : LabPlot
    Description          : widget for 3D surfaces properties
    --------------------------------------------------------------------
    Copyright            : (C) 2015 Minh Ngo (minh@fedoraproject.org)

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

#ifndef SURFACE3DDOCK_H
#define SURFACE3DDOCK_H

#include <QWidget>
#include "backend/worksheet/plots/3d/Surface3D.h"
#include "ui_surface3ddock.h"

class Surface3D;
class Matrix;
class AbstractColumn;
class AspectTreeModel;

class Surface3DDock : public QWidget {
	Q_OBJECT

	public:
		explicit Surface3DDock(QWidget* parent);
		void setSurface(Surface3D *surface);

	private:
		void showTriangleInfo(bool pred);
		void setModelFromAspect(TreeViewComboBox* cb, const AbstractAspect* aspect);

	private slots:
		void retranslateUi();

		//SLOTs for changes triggered in Surface3DDock
		void nameChanged();
		void commentChanged();

		void onTreeViewIndexChanged(const QModelIndex&);
		void onDataSourceChanged(int);
		void onVisualizationTypeChanged(int);
		void onFileChanged(const KUrl&);
		void onVisibilityChanged(bool);

		// Surface 3D
		void visualizationTypeChanged(Surface3D::VisualizationType);
		void sourceTypeChanged(Surface3D::DataSource);
		void colorFillingChanged(Surface3D::ColorFilling);

		// Color filling
		void onColorFillingTypeChanged(int);

		// File handling
		void pathChanged(const KUrl&);

		// Matrix handling
		void matrixChanged(const Matrix*);

		// Spreadsheet handling
		void xColumnChanged(const AbstractColumn*);
		void yColumnChanged(const AbstractColumn*);
		void zColumnChanged(const AbstractColumn*);
		void firstNodeChanged(const AbstractColumn*);
		void secondNodeChanged(const AbstractColumn*);
		void thirdNodeChanged(const AbstractColumn*);

		//SLOTs for changes triggered in Surface3D
		//TODO

		//load and save
		void loadConfigFromTemplate(KConfig&);
		void saveConfigAsTemplate(KConfig&);

	signals:
		void elementVisibilityChanged();

	private:
		Ui::Surface3DDock ui;
		Surface3D *surface;
		AspectTreeModel *aspectTreeModel;
		bool m_initializing;

		void load();
		void loadConfig(KConfig&);
};

#endif
