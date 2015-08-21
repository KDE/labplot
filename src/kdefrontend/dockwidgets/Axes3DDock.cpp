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

using namespace DockHelpers;

Axes3DDock::Axes3DDock(QWidget* parent)
	: QWidget(parent)
	, recorder(this)
	, axes(0)
	, m_initializing(false) {
	ui.setupUi(this);
	retranslateUi();

	// Axes
	recorder.connect(ui.cbXLabelFormat, SIGNAL(currentIndexChanged(int)), SLOT(onFormatXChanged(int)));
	recorder.connect(ui.cbYLabelFormat, SIGNAL(currentIndexChanged(int)), SLOT(onFormatYChanged(int)));
	recorder.connect(ui.cbZLabelFormat, SIGNAL(currentIndexChanged(int)), SLOT(onFormatZChanged(int)));
	recorder.connect(ui.cbLabelFontSize, SIGNAL(valueChanged(int)), SLOT(onLabelFontChanged(int)));
	recorder.connect(ui.cbXLabelColor, SIGNAL(changed(const QColor&)), SLOT(onLabelColorChanged(const QColor&)));
	recorder.connect(ui.cbYLabelColor, SIGNAL(changed(const QColor&)), SLOT(onLabelColorChanged(const QColor&)));
	recorder.connect(ui.cbZLabelColor, SIGNAL(changed(const QColor&)), SLOT(onLabelColorChanged(const QColor&)));
	recorder.connect(ui.leXLabel, SIGNAL(returnPressed(const QString&)), SLOT(onLabelChanged(const QString&)));
	recorder.connect(ui.leYLabel, SIGNAL(returnPressed(const QString&)), SLOT(onLabelChanged(const QString&)));
	recorder.connect(ui.leZLabel, SIGNAL(returnPressed(const QString&)), SLOT(onLabelChanged(const QString&)));
	recorder.connect(ui.chkVisible, SIGNAL(toggled(bool)), SLOT(onVisibilityChanged(bool)));
}

void Axes3DDock::retranslateUi() {
	QVector<QComboBox*> labelFormats;
	labelFormats << ui.cbXLabelFormat << ui.cbYLabelFormat << ui.cbZLabelFormat;
	foreach(QComboBox* cb, labelFormats) {
		cb->insertItem(Axes::Format_Decimal, i18n("Decimal notation"));
		cb->insertItem(Axes::Format_Scientific, i18n("Scientific notation"));
		cb->insertItem(Axes::Format_PowerOf10, i18n("Powers of 10"));
		cb->insertItem(Axes::Format_PowerOf2, i18n("Powers of 2"));
		cb->insertItem(Axes::Format_PowerOfE, i18n("Powers of e"));
		cb->insertItem(Axes::Format_MultiplierOfPi, i18n("Multiplier of \u03C0"));
	}
}

void Axes3DDock::setAxes(Axes *axes) {
	this->axes = axes;

	{
	const SignalBlocker blocker(recorder.children());
	ui.chkVisible->setChecked(axes->isVisible());
	formatXChanged(axes->formatX());
	formatYChanged(axes->formatY());
	formatZChanged(axes->formatZ());
	fontSizeChanged(axes->fontSize());
	xLabelColorChanged(axes->xLabelColor());
	yLabelColorChanged(axes->yLabelColor());
	zLabelColorChanged(axes->zLabelColor());
	xLabelChanged(axes->xLabel());
	yLabelChanged(axes->yLabel());
	zLabelChanged(axes->zLabel());
	}

	connect(axes, SIGNAL(formatXChanged(Axes::Format)), SLOT(formatXChanged(Axes::Format)));
	connect(axes, SIGNAL(formatYChanged(Axes::Format)), SLOT(formatYChanged(Axes::Format)));
	connect(axes, SIGNAL(formatZChanged(Axes::Format)), SLOT(formatZChanged(Axes::Format)));
	connect(axes, SIGNAL(fontSizeChanged(int)), SLOT(fontSizeChanged(int)));
	connect(axes, SIGNAL(xLabelColorChanged(const QColor&)), SLOT(xLabelColorChanged(const QColor&)));
	connect(axes, SIGNAL(yLabelColorChanged(const QColor&)), SLOT(yLabelColorChanged(const QColor&)));
	connect(axes, SIGNAL(zLabelColorChanged(const QColor&)), SLOT(zLabelColorChanged(const QColor&)));
	connect(axes, SIGNAL(xLabelChanged(const QString&)), SLOT(xLabelChanged(const QString&)));
	connect(axes, SIGNAL(yLabelChanged(const QString&)), SLOT(yLabelChanged(const QString&)));
	connect(axes, SIGNAL(zLabelChanged(const QString&)), SLOT(zLabelChanged(const QString&)));
}

void Axes3DDock::onFormatXChanged(int index) {
	const Lock lock(m_initializing);
	axes->setFormatX(static_cast<Axes::Format>(index));
}

void Axes3DDock::onFormatYChanged(int index) {
	const Lock lock(m_initializing);
	axes->setFormatY(static_cast<Axes::Format>(index));
}

void Axes3DDock::onFormatZChanged(int index) {
	const Lock lock(m_initializing);
	axes->setFormatZ(static_cast<Axes::Format>(index));
}

void Axes3DDock::onLabelFontChanged(int size) {
	const Lock lock(m_initializing);
	axes->setFontSize(size);
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
}

void Axes3DDock::onLabelChanged(const QString& label) {
	const QObject *s = sender();
	const Lock lock(m_initializing);
	if (s == ui.leXLabel) {
		axes->setXLabel(label);
	} else if (s == ui.leYLabel) {
		axes->setYLabel(label);
	} else {
		axes->setZLabel(label);
	}
}

void Axes3DDock::onVisibilityChanged(bool visibility) {
	const Lock lock(m_initializing);
	axes->show(visibility);
}

void Axes3DDock::formatXChanged(Axes::Format index) {
	if (m_initializing)
		return;
	ui.cbXLabelFormat->setCurrentIndex(index);
}

void Axes3DDock::formatYChanged(Axes::Format index) {
	if (m_initializing)
		return;
	ui.cbYLabelFormat->setCurrentIndex(index);
}

void Axes3DDock::formatZChanged(Axes::Format index) {
	if (m_initializing)
		return;
	ui.cbZLabelFormat->setCurrentIndex(index);
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

void Axes3DDock::xLabelChanged(const QString& label) {
	if (m_initializing)
		return;
	ui.leXLabel->setText(label);
}

void Axes3DDock::yLabelChanged(const QString& label) {
	if (m_initializing)
		return;
	ui.leYLabel->setText(label);
}

void Axes3DDock::zLabelChanged(const QString& label) {
	if (m_initializing)
		return;
	ui.leZLabel->setText(label);
}