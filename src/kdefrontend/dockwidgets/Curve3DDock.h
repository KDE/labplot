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

	private slots:
		void onTreeViewIndexChanged(const QModelIndex& index);

		void nameChanged();
		void commentChanged();
		void onVisibilityChanged(bool visible);

		void xColumnChanged(const AbstractColumn* column);
		void yColumnChanged(const AbstractColumn* column);
		void zColumnChanged(const AbstractColumn* column);
		void pointRadiusChanged(float radius);
		void isClosedChanged(bool checked);
		void showVerticesChanged(bool checked);

		void onShowVerticesChanged(bool checked);
		void onClosedCurveChanged(bool checked);

	private:
		Ui::Curve3DDock ui;
		Curve3D *curve;
		AspectTreeModel *aspectTreeModel;
		bool m_initializing;
};

#endif