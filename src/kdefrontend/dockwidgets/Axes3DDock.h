/***************************************************************************
    File                 : Axes3D.h
    Project              : LabPlot
    Description          : widget for 3D Axes properties
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

#ifndef AXES3DDOCK_H
#define AXES3DDOCK_H

#include <QWidget>

#include "backend/worksheet/plots/3d/Axes.h"
#include "ui_axes3ddock.h"
#include "DockHelpers.h"

class Axes3DDock : public QWidget {
		Q_OBJECT
	public:
		explicit Axes3DDock(QWidget* parent);
		void setAxes(Axes *axes);

	private slots:
		void retranslateUi();

		void onFormatXChanged(int index);
		void onFormatYChanged(int index);
		void onFormatZChanged(int index);
		void onLabelFontChanged(int size);
		void onLabelColorChanged(const QColor& color);
		void onLabelChanged(const QString& label);
		void onVisibilityChanged(bool);

		void formatXChanged(Axes::Format);
		void formatYChanged(Axes::Format);
		void formatZChanged(Axes::Format);
		void fontSizeChanged(int);
		void xLabelColorChanged(const QColor&);
		void yLabelColorChanged(const QColor&);
		void zLabelColorChanged(const QColor&);
		void xLabelChanged(const QString&);
		void yLabelChanged(const QString&);
		void zLabelChanged(const QString&);

	private:
		Ui::Axes3DDock ui;
		DockHelpers::ChildrenRecorder recorder;
		Axes *axes;
		bool m_initializing;
};

#endif