/***************************************************************************
    File                 : Curve3DDock.h
    Project              : LabPlot
    Description          : widget for 3D curves properties
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

#ifndef CURVE3DDOCK_H
#define CURVE3DDOCK_H

#include <QWidget>

#include "ui_curve3ddock.h"

class Curve3D;
class AbstractColumn;
class AspectTreeModel;

class Curve3DDock : public QWidget {
	Q_OBJECT

	public:
		explicit Curve3DDock(QWidget* parent);
		void setCurve(Curve3D* curve);

	private:
		void setModelFromAspect(TreeViewComboBox* cb, const AbstractAspect* aspect);

	private slots:
		//SLOTs for changes triggered in Curve3DDock
		void onTreeViewIndexChanged(const QModelIndex& index);

		void onNameChanged();
		void onCommentChanged();
		void onVisibilityChanged(bool visible);

		void onShowEdgesChanged(bool checked);
		void onIsClosedChanged(bool checked);
		void onPointRadiusChanged(double size);

		void onXColumnChanged(const AbstractColumn* column);
		void onYColumnChanged(const AbstractColumn* column);
		void onZColumnChanged(const AbstractColumn* column);


		void showEdgesChanged(bool checked);
		void isClosedChanged(bool checked);
		void pointRadiusChanged(float radius);

		void xColumnChanged(const AbstractColumn* column);
		void yColumnChanged(const AbstractColumn* column);
		void zColumnChanged(const AbstractColumn* column);


		//load and save
		void loadConfigFromTemplate(KConfig&);
		void saveConfigAsTemplate(KConfig&);

	private:
		Ui::Curve3DDock ui;
		QVector<QObject*> children;
		Curve3D *curve;
		AspectTreeModel *aspectTreeModel;
		bool m_initializing;

		void load();
		void loadConfig(KConfig&);
};

#endif
