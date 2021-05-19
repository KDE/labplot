/***************************************************************************
    File                 : ReferenceLineDock.cpp
    Project              : LabPlot
    Description          : Dock widget for the reference line on the plot
    --------------------------------------------------------------------
    Copyright            : (C) 2020 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2021 Stefan Gerlach (stefan.gerlach@uni.kn)
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

#include "ReferenceLineDock.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/ReferenceLine.h"

#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/GuiTools.h"

#include <KLocalizedString>
#include <KConfig>

ReferenceLineDock::ReferenceLineDock(QWidget* parent) : BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_teComment = ui.teComment;
	m_teComment->setFixedHeight(m_leName->height());

	ui.cbOrientation->addItem(i18n("Horizontal"));
	ui.cbOrientation->addItem(i18n("Vertical"));

	ui.lePosition->setValidator( new QDoubleValidator(ui.lePosition) );

	SET_NUMBER_LOCALE
	ui.sbLineWidth->setLocale(numberLocale);

	//SLOTS
	//General
	connect(ui.leName, &QLineEdit::textChanged, this, &ReferenceLineDock::nameChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &ReferenceLineDock::commentChanged);

	connect(ui.cbOrientation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ReferenceLineDock::orientationChanged);
	connect(ui.lePosition, &QLineEdit::textChanged, this, &ReferenceLineDock::positionChanged);
	connect(ui.cbPlotRanges, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ReferenceLineDock::plotRangeChanged);
	connect(ui.chkVisible, &QCheckBox::clicked, this, &ReferenceLineDock::visibilityChanged);

	connect(ui.cbLineStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ReferenceLineDock::styleChanged);
	connect(ui.kcbLineColor, &KColorButton::changed, this, &ReferenceLineDock::colorChanged);
	connect(ui.sbLineWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ReferenceLineDock::widthChanged);
	connect(ui.sbLineOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &ReferenceLineDock::opacityChanged);

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbLineStyle, QColor(Qt::black));
	m_initializing = false;
}

void ReferenceLineDock::setReferenceLines(QList<ReferenceLine*> list) {
	m_initializing = true;
	m_linesList = list;
	m_line = list.first();
	m_aspect = list.first();
	Q_ASSERT(m_line);

	//if there is more then one point in the list, disable the comment and name widgets in "general"
	if (list.size() == 1) {
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.teComment->setEnabled(true);
		ui.leName->setText(m_line->name());
		ui.teComment->setText(m_line->comment());
	} else {
		ui.lName->setEnabled(false);
		ui.leName->setEnabled(false);
		ui.lComment->setEnabled(false);
		ui.teComment->setEnabled(false);
		ui.leName->setText(QString());
		ui.teComment->setText(QString());
	}
	ui.leName->setStyleSheet("");
	ui.leName->setToolTip("");

	//show the properties of the first reference line
	this->load();

	updatePlotRanges();

	//SIGNALs/SLOTs
	connect(m_line, &AbstractAspect::aspectDescriptionChanged,this, &ReferenceLineDock::aspectDescriptionChanged);
	connect(m_line, &WorksheetElement::plotRangeListChanged, this, &ReferenceLineDock::updatePlotRanges);
	connect(m_line, &ReferenceLine::visibleChanged, this, &ReferenceLineDock::lineVisibilityChanged);

	//position
	connect(m_line, &ReferenceLine::orientationChanged, this, &ReferenceLineDock::lineOrientationChanged);
	connect(m_line, &ReferenceLine::positionChanged, this, &ReferenceLineDock::linePositionChanged);

	//line
	connect(m_line, &ReferenceLine::penChanged, this, &ReferenceLineDock::linePenChanged);
	connect(m_line, &ReferenceLine::opacityChanged, this, &ReferenceLineDock::lineOpacityChanged);
}

/*
 * updates the locale in the widgets. called when the application settins are changed.
 */
void ReferenceLineDock::updateLocale() {
	SET_NUMBER_LOCALE
	ui.sbLineWidth->setLocale(numberLocale);

	Lock lock(m_initializing);
	ui.lePosition->setText(numberLocale.toString(m_line->position()));
}

void ReferenceLineDock::updatePlotRanges() const {
	updatePlotRangeList(ui.cbPlotRanges);
}

//**********************************************************
//*** SLOTs for changes triggered in ReferenceLineDock *****
//**********************************************************
//Position
void ReferenceLineDock::orientationChanged(int index) {
	auto orientation{ReferenceLine::Orientation(index)};
	if (orientation == ReferenceLine::Orientation::Horizontal)
		ui.lPosition->setText(QLatin1String("y:"));
	else
		ui.lPosition->setText(QLatin1String("x:"));

	if (m_initializing)
		return;

	for (auto* line : m_linesList)
		line->setOrientation(orientation);
}

void ReferenceLineDock::positionChanged() {
	if (m_initializing)
		return;

	bool ok;
	SET_NUMBER_LOCALE
	const double pos{ numberLocale.toDouble(ui.lePosition->text(), &ok) };
	if (ok) {
		for (auto* line : m_linesList)
			line->setPosition(pos);
	}
}

//Line
void ReferenceLineDock::styleChanged(int index) {
	if (m_initializing)
		return;

	const auto penStyle = Qt::PenStyle(index);
	QPen pen;
	for (auto* line : m_linesList) {
		pen = line->pen();
		pen.setStyle(penStyle);
		line->setPen(pen);
	}
}

void ReferenceLineDock::colorChanged(const QColor& color) {
	if (m_initializing)
		return;

	for (auto* line : m_linesList) {
		QPen pen = line->pen();
		pen.setColor(color);
		line->setPen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbLineStyle, color);
	m_initializing = false;
}

void ReferenceLineDock::widthChanged(double value) {
	if (m_initializing)
		return;

	for (auto* line : m_linesList) {
		QPen pen = line->pen();
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
		line->setPen(pen);
	}
}

void ReferenceLineDock::opacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (double)value/100.;
	for (auto* line : m_linesList)
		line->setOpacity(opacity);
}

void ReferenceLineDock::visibilityChanged(bool state) {
	if (m_initializing)
		return;

	for (auto* line : m_linesList)
		line->setVisible(state);
}

//*************************************************************
//******* SLOTs for changes triggered in ReferenceLine ********
//*************************************************************
void ReferenceLineDock::linePositionChanged(double position) {
	m_initializing = true;
	ui.lePosition->setText(QString::number(position));
	m_initializing = false;
}

void ReferenceLineDock::lineOrientationChanged(ReferenceLine::Orientation orientation) {
	m_initializing = true;
	ui.cbOrientation->setCurrentIndex(static_cast<int>(orientation));
	m_initializing = false;
}

void ReferenceLineDock::linePenChanged(const QPen& pen) {
	m_initializing = true;
	ui.cbLineStyle->setCurrentIndex( (int)pen.style());
	ui.kcbLineColor->setColor( pen.color());
	GuiTools::updatePenStyles(ui.cbLineStyle, pen.color());
	ui.sbLineWidth->setValue( Worksheet::convertFromSceneUnits( pen.widthF(), Worksheet::Unit::Point) );
	m_initializing = false;
}

void ReferenceLineDock::lineOpacityChanged(qreal opacity) {
	m_initializing = true;
	ui.sbLineOpacity->setValue( round(opacity*100.0) );
	m_initializing = false;
}

void ReferenceLineDock::lineVisibilityChanged(bool on) {
	m_initializing = true;
	ui.chkVisible->setChecked(on);
	m_initializing = false;
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void ReferenceLineDock::load() {
	if (!m_line)
		return;

	m_initializing = true;

	SET_NUMBER_LOCALE
	ui.cbOrientation->setCurrentIndex(static_cast<int>(m_line->orientation()));
	ui.lePosition->setText(numberLocale.toString(m_line->position()));
	ui.cbLineStyle->setCurrentIndex( (int) m_line->pen().style() );
	ui.kcbLineColor->setColor( m_line->pen().color() );
	ui.sbLineWidth->setValue( Worksheet::convertFromSceneUnits(m_line->pen().widthF(), Worksheet::Unit::Point) );
	ui.sbLineOpacity->setValue( round(m_line->opacity()*100.0) );
	ui.chkVisible->setChecked( m_line->isVisible() );

	m_initializing = false;
}
