/***************************************************************************
    File                 : Axes3D.cpp
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

#include "Axes3DDock.h"
#include "DockHelpers.h"

using namespace DockHelpers;

Axes3DDock::Axes3DDock(QWidget* parent)
	: QWidget(parent)
	, axes(0)
	, m_initializing(false) {
	ui.setupUi(this);
	retranslateUi();

	// Axes
	connect(ui.cbType, SIGNAL(currentIndexChanged(int)), SLOT(onTypeChanged(int)));
	connect(ui.cbLabelFontSize, SIGNAL(valueChanged(int)), SLOT(onLabelFontChanged(int)));
	connect(ui.cbXLabelColor, SIGNAL(changed(const QColor&)), SLOT(onLabelColorChanged(const QColor&)));
	connect(ui.cbYLabelColor, SIGNAL(changed(const QColor&)), SLOT(onLabelColorChanged(const QColor&)));
	connect(ui.cbZLabelColor, SIGNAL(changed(const QColor&)), SLOT(onLabelColorChanged(const QColor&)));
}

void Axes3DDock::retranslateUi() {
	ui.cbType->insertItem(Axes::AxesType_Cube, i18n("Cube Axes"));
	ui.cbType->insertItem(Axes::AxesType_Plain, i18n("Plain Axes"));
}

void Axes3DDock::setAxes(Axes *axes) {
	this->axes = axes;

	blockSignals(true);
	axesTypeChanged(axes->type());
	fontSizeChanged(axes->fontSize());
	xLabelColorChanged(axes->xLabelColor());
	yLabelColorChanged(axes->yLabelColor());
	zLabelColorChanged(axes->zLabelColor());
	blockSignals(false);

	connect(axes, SIGNAL(typeChanged(Axes::AxesType)), SLOT(axesTypeChanged(Axes::AxesType)));
	connect(axes, SIGNAL(fontSizeChanged(int)), SLOT(fontSizeChanged(int)));
	connect(axes, SIGNAL(xLabelColorChanged(const QColor&)), SLOT(xLabelColorChanged(const QColor&)));
	connect(axes, SIGNAL(yLabelColorChanged(const QColor&)), SLOT(yLabelColorChanged(const QColor&)));
	connect(axes, SIGNAL(zLabelColorChanged(const QColor&)), SLOT(zLabelColorChanged(const QColor&)));
}

void Axes3DDock::onTypeChanged(int type) {
	const Lock lock(m_initializing);
	axes->setType(static_cast<Axes::AxesType>(type));
}

void Axes3DDock::onLabelFontChanged(int size) {
	const Lock lock(m_initializing);
	axes->setFontSize(size);
	axes->setType(static_cast<Axes::AxesType>(ui.cbType->currentIndex()));
}

void Axes3DDock::onLabelColorChanged(const QColor& color) {
	const QObject *s = sender();
	const Lock lock(m_initializing);
	if (s == ui.cbXLabelColor) {
		axes->setXLabelColor(color);
	} else if (s == ui.cbYLabelColor) {
		axes->setYLabelColor(color);
	} else {
		axes->setZLabelColor(color);
	}

	axes->setType(static_cast<Axes::AxesType>(ui.cbType->currentIndex()));
}

void Axes3DDock::fontSizeChanged(int value){
	if (m_initializing)
		return;
	ui.cbLabelFontSize->setValue(value);
}

void Axes3DDock::xLabelColorChanged(const QColor& color){
	if (m_initializing)
		return;
	ui.cbXLabelColor->setColor(color);
}

void Axes3DDock::yLabelColorChanged(const QColor& color){
	if (m_initializing)
		return;
	ui.cbYLabelColor->setColor(color);
}

void Axes3DDock::zLabelColorChanged(const QColor& color){
	if (m_initializing)
		return;
	ui.cbZLabelColor->setColor(color);
}

void Axes3DDock::axesTypeChanged(Axes::AxesType type){
	if (m_initializing)
		return;
	ui.cbType->setCurrentIndex(type);
}