/*
	File                 : LineWidget.cpp
	Project              : LabPlot
	Description          : line settings widget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "LineWidget.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/dockwidgets/BaseDock.h"

#include <KConfigGroup>
#include <QCompleter>
#include <QDirModel>
#include <QFile>
#include <QTimer>

/*!
	\class LineWidget
	\brief Widget for editing the properties of a Line object, mostly used in an appropriate dock widget.

	\ingroup kdefrontend
 */
LineWidget::LineWidget(QWidget* parent)
	: QWidget(parent) {
	ui.setupUi(this);

	connect(ui.cbStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LineWidget::styleChanged);
	connect(ui.kcbColor, &KColorButton::changed, this, &LineWidget::colorChanged);
	connect(ui.sbWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &LineWidget::widthChanged);
	connect(ui.sbOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &LineWidget::opacityChanged);
}

void LineWidget::setLines(QList<Line*> lines) {
	m_lines = lines;
	m_line = m_lines.first();

	load();

	connect(m_line, &Line::penChanged, this, &LineWidget::linePenChanged);
	connect(m_line, &Line::opacityChanged, this, &LineWidget::lineOpacityChanged);

	QTimer::singleShot(100, this, [=]() {
		adjustLayout();
	});
}

void LineWidget::setPrefix(const QString& prefix) {
	m_prefix = prefix;
}

/*!
 * this functions adjusts the width of the first column in the layout of LineWidget
 * to the width of the first column in the layout of the parent widget
 * which LineWidget is being embedded into.
 */
void LineWidget::adjustLayout() {
	auto* parentGridLayout = dynamic_cast<QGridLayout*>(parentWidget()->layout());
	if (!parentGridLayout)
		return;

	auto* parentWidget = parentGridLayout->itemAtPosition(0, 0)->widget();
	auto* gridLayout = static_cast<QGridLayout*>(layout());
	auto* widget = gridLayout->itemAtPosition(2, 0)->widget(); // use the third line, the first two are optional and not always visible

	if (parentWidget->width() >= widget->width()) {
		gridLayout->activate();
		widget->setMinimumWidth(parentWidget->width());
		updateGeometry();
	} else {
		parentGridLayout->activate();
		parentWidget->setMinimumWidth(widget->width());
		this->parentWidget()->updateGeometry();
	}
}

void LineWidget::setEnabled(bool enabled) {
	ui.cbStyle->setEnabled(enabled);
	ui.kcbColor->setEnabled(enabled);
	ui.sbWidth->setEnabled(enabled);
	ui.sbOpacity->setEnabled(enabled);
}

void LineWidget::updateLocale() {
	SET_NUMBER_LOCALE
	ui.sbWidth->setLocale(numberLocale);
}

//*************************************************************
//******** SLOTs for changes triggered in LineWidget ****
//*************************************************************
void LineWidget::styleChanged(int index) const {
	if (m_initializing)
		return;

	auto penStyle = Qt::PenStyle(index);
	QPen pen;
	for (auto* line : m_lines) {
		pen = line->pen();
		pen.setStyle(penStyle);
		line->setPen(pen);
	}
}

void LineWidget::colorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* line : m_lines) {
		pen = line->pen();
		pen.setColor(color);
		line->setPen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbStyle, color);
	m_initializing = false;
}

void LineWidget::widthChanged(double value) const {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* line : m_lines) {
		pen = line->pen();
		pen.setWidthF(Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point));
		line->setPen(pen);
	}
}

void LineWidget::opacityChanged(int value) const {
	if (m_initializing)
		return;

	double opacity = static_cast<double>(value) / 100.;
	for (auto* line : m_lines)
		line->setOpacity(opacity);
}

//*************************************************************
//********* SLOTs for changes triggered in Line *********
//*************************************************************
void LineWidget::linePenChanged(QPen& pen) {
	Lock lock(m_initializing);
	if (ui.cbStyle->currentIndex() != pen.style())
		ui.cbStyle->setCurrentIndex(pen.style());
	if (ui.kcbColor->color() != pen.color())
		ui.kcbColor->setColor(pen.color());
	if (ui.sbWidth->value() != pen.widthF())
		ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point));
}

void LineWidget::lineOpacityChanged(double value) {
	Lock lock(m_initializing);
	double v = (double)value * 100.;
	ui.sbOpacity->setValue(v);
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void LineWidget::load() {
	const Lock lock(m_initializing);
	const QPen& penBorder = m_line->pen();
	ui.cbStyle->setCurrentIndex(static_cast<int>(penBorder.style()));
	ui.kcbColor->setColor(penBorder.color());
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(penBorder.widthF(), Worksheet::Unit::Point));
	ui.sbOpacity->setValue(m_line->opacity() * 100);
	GuiTools::updatePenStyles(ui.cbStyle, ui.kcbColor->color());
}

void LineWidget::loadConfig(const KConfigGroup& group) {
	const Lock lock(m_initializing);
	const QPen& penBorder = m_line->pen();
	ui.cbStyle->setCurrentIndex(group.readEntry(m_prefix + "Style", static_cast<int>(penBorder.style())));
	ui.kcbColor->setColor(group.readEntry(m_prefix + "Color", penBorder.color()));
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(group.readEntry(m_prefix + "Width", penBorder.widthF()), Worksheet::Unit::Point));
	ui.sbOpacity->setValue(group.readEntry(m_prefix + "Opacity", m_line->opacity()) * 100);
	GuiTools::updatePenStyles(ui.cbStyle, ui.kcbColor->color());
}

void LineWidget::saveConfig(KConfigGroup& group) const {
	group.writeEntry(m_prefix + "Style", ui.cbStyle->currentIndex());
	group.writeEntry(m_prefix + "Color", ui.kcbColor->color());
	group.writeEntry(m_prefix + "Width", Worksheet::convertToSceneUnits(ui.sbWidth->value(), Worksheet::Unit::Point));
	group.writeEntry(m_prefix + "Opacity", ui.sbOpacity->value() / 100.0);
}
