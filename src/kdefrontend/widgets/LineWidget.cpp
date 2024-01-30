/*
	File                 : LineWidget.cpp
	Project              : LabPlot
	Description          : line settings widget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "LineWidget.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/dockwidgets/BaseDock.h"

#include <KConfigGroup>
#include <QPainter>
#include <QTimer>

/*!
	\class LineWidget
	\brief Widget for editing the properties of a Line object, mostly used in an appropriate dock widget.

	\ingroup kdefrontend
 */
LineWidget::LineWidget(QWidget* parent)
	: QWidget(parent) {
	ui.setupUi(this);

	ui.sbWidth->setMinimum(0);

	connect(ui.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LineWidget::typeChanged);
	connect(ui.cbStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LineWidget::styleChanged);
	connect(ui.kcbColor, &KColorButton::changed, this, &LineWidget::colorChangedSlot);
	connect(ui.sbWidth, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &LineWidget::widthChanged);
	connect(ui.sbOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &LineWidget::opacityChanged);
}

void LineWidget::setLines(const QList<Line*>& lines) {
	CONDITIONAL_LOCK_RETURN;
	m_lines = lines;
	m_line = m_lines.first();
	m_prefix = m_line->prefix();

	if (m_line->histogramLineTypeAvailable()) {
		ui.lType->show();
		ui.cbType->show();


		if (ui.cbType->count() == 0) {
			ui.cbType->addItem(i18n("None"));
			ui.cbType->addItem(i18n("Bars"));
			ui.cbType->addItem(i18n("Envelope"));
			ui.cbType->addItem(i18n("Drop Lines"));
			ui.cbType->addItem(i18n("Half-Bars"));
		}
	} else if (m_prefix == QLatin1String("DropLine")) {
		ui.lType->show();
		ui.cbType->show();

		if (ui.cbType->count() == 0) {
			ui.cbType->addItem(i18n("No Drop Lines"));
			ui.cbType->addItem(i18n("Drop Lines, X"));
			ui.cbType->addItem(i18n("Drop Lines, Y"));
			ui.cbType->addItem(i18n("Drop Lines, XY"));
			ui.cbType->addItem(i18n("Drop Lines, X, Zero Baseline"));
			ui.cbType->addItem(i18n("Drop Lines, X, Min Baseline"));
			ui.cbType->addItem(i18n("Drop Lines, X, Max Baseline"));
		}
	} else {
		ui.lType->hide();
		ui.cbType->hide();
	}

	load();

	connect(m_line, &Line::histogramLineTypeChanged, this, &LineWidget::histogramLineTypeChanged);
	connect(m_line, &Line::dropLineTypeChanged, this, &LineWidget::dropLineTypeChanged);
	connect(m_line, &Line::styleChanged, this, &LineWidget::lineStyleChanged);
	connect(m_line, &Line::colorChanged, this, &LineWidget::lineColorChanged);
	connect(m_line, &Line::widthChanged, this, &LineWidget::lineWidthChanged);
	connect(m_line, &Line::opacityChanged, this, &LineWidget::lineOpacityChanged);
}

void LineWidget::showEvent(QShowEvent* event) {
	QWidget::showEvent(event);
	adjustLayout();
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
	if (!parentWidget)
		return;

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
	const auto numberLocale = QLocale();
	ui.sbWidth->setLocale(numberLocale);
}

//*************************************************************
//******** SLOTs for changes triggered in LineWidget **********
//*************************************************************
void LineWidget::typeChanged(int index) {
	bool enabled = true;
	if (m_line->histogramLineTypeAvailable()) {
		const auto type = Histogram::LineType(index);
		enabled = (type != Histogram::NoLine);

		if (!m_initializing) {
			for (auto* line : m_lines)
				line->setHistogramLineType(type);
		}
	} else if (m_prefix == QLatin1String("DropLine")) {
		auto type = static_cast<XYCurve::DropLineType>(index);
		enabled = (type != XYCurve::DropLineType::NoDropLine);

		if (!m_initializing) {
			for (auto* line : m_lines)
				line->setDropLineType(type);
		}
	}

	ui.cbStyle->setEnabled(enabled);
	ui.kcbColor->setEnabled(enabled);
	ui.sbWidth->setEnabled(enabled);
	ui.sbOpacity->setEnabled(enabled);

	// TODO
	// const bool fillingEnabled = (lineType == Histogram::LineType::Bars || lineType == Histogram::LineType::Envelope);
	// backgroundWidget->setEnabled(fillingEnabled);
}

void LineWidget::styleChanged(int index) {
	CONDITIONAL_LOCK_RETURN;
	auto style = static_cast<Qt::PenStyle>(index);
	for (auto* line : m_lines)
		line->setStyle(style);
}

void LineWidget::colorChangedSlot(const QColor& color) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* line : m_lines)
		line->setColor(color);

	GuiTools::updatePenStyles(ui.cbStyle, color);
}

void LineWidget::widthChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	const double width = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* line : m_lines)
		line->setWidth(width);
}

void LineWidget::opacityChanged(int value) {
	CONDITIONAL_LOCK_RETURN;
	double opacity = static_cast<double>(value) / 100.;
	for (auto* line : m_lines)
		line->setOpacity(opacity);
}

//*************************************************************
//*********** SLOTs for changes triggered in Line *************
//*************************************************************
void LineWidget::histogramLineTypeChanged(Histogram::LineType type) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbType->setCurrentIndex(static_cast<int>(type));
}

void LineWidget::dropLineTypeChanged(XYCurve::DropLineType type) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbType->setCurrentIndex(static_cast<int>(type));
}

void LineWidget::lineStyleChanged(Qt::PenStyle style) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbStyle->setCurrentIndex(static_cast<int>(style));
}

void LineWidget::lineColorChanged(const QColor& color) {
	Q_EMIT colorChanged(color);

	CONDITIONAL_LOCK_RETURN;
	ui.kcbColor->setColor(color);
}

void LineWidget::lineWidthChanged(double width) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(width, Worksheet::Unit::Point));
}

void LineWidget::lineOpacityChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbOpacity->setValue(round(value * 100));
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void LineWidget::load() {
	if (m_line->histogramLineTypeAvailable())
		ui.cbType->setCurrentIndex(static_cast<int>(m_line->histogramLineType()));
	else if (m_prefix == QLatin1String("DropLine"))
		ui.cbType->setCurrentIndex(static_cast<int>(m_line->dropLineType()));

	setColor(m_line->color());
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(m_line->width(), Worksheet::Unit::Point));
	ui.sbOpacity->setValue(round(m_line->opacity() * 100));
	GuiTools::updatePenStyles(ui.cbStyle, ui.kcbColor->color());
	ui.cbStyle->setCurrentIndex(static_cast<int>(m_line->style()));
}

void LineWidget::loadConfig(const KConfigGroup& group) {
	if (m_line->histogramLineTypeAvailable())
		ui.cbType->setCurrentIndex(group.readEntry(m_prefix + QStringLiteral("Type"), static_cast<int>(m_line->histogramLineType())));
	else if (m_prefix == QLatin1String("DropLine"))
		ui.cbType->setCurrentIndex(group.readEntry("DropLineType", static_cast<int>(m_line->dropLineType())));

	ui.cbStyle->setCurrentIndex(group.readEntry(m_prefix + QStringLiteral("Style"), static_cast<int>(m_line->style())));
	setColor(group.readEntry(m_prefix + QStringLiteral("Color"), m_line->color()));
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(group.readEntry(m_prefix + QStringLiteral("Width"), m_line->width()), Worksheet::Unit::Point));
	ui.sbOpacity->setValue(group.readEntry(m_prefix + QStringLiteral("Opacity"), m_line->opacity()) * 100);
	GuiTools::updatePenStyles(ui.cbStyle, ui.kcbColor->color());
}

void LineWidget::saveConfig(KConfigGroup& group) const {
	if (m_line->histogramLineTypeAvailable() || m_prefix == QLatin1String("DropLine"))
		group.writeEntry(m_prefix + QStringLiteral("Type"), ui.cbType->currentIndex());

	group.writeEntry(m_prefix + QStringLiteral("Style"), ui.cbStyle->currentIndex());
	group.writeEntry(m_prefix + QStringLiteral("Color"), ui.kcbColor->color());
	group.writeEntry(m_prefix + QStringLiteral("Width"), Worksheet::convertToSceneUnits(ui.sbWidth->value(), Worksheet::Unit::Point));
	group.writeEntry(m_prefix + QStringLiteral("Opacity"), ui.sbOpacity->value() / 100.0);
}

void LineWidget::setColor(const QColor& color) {
	ui.kcbColor->setColor(color);
}
