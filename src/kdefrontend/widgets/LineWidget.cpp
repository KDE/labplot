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

	connect(ui.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LineWidget::typeChanged);
	connect(ui.sbErrorBarsCapSize, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &LineWidget::capSizeChanged);

	connect(ui.cbStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LineWidget::styleChanged);
	connect(ui.kcbColor, &KColorButton::changed, this, &LineWidget::colorChanged);
	connect(ui.sbWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &LineWidget::widthChanged);
	connect(ui.sbOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &LineWidget::opacityChanged);
}

void LineWidget::setLines(const QList<Line*>& lines) {
	m_lines = lines;
	m_line = m_lines.first();

	if (m_line->histogramLineTypeAvailable()) {
		ui.lType->show();
		ui.cbType->show();
		ui.lErrorBarsCapSize->hide();
		ui.sbErrorBarsCapSize->hide();

		if (ui.cbType->count() == 0) {
			Lock lock(m_initializing);
			ui.cbType->addItem(i18n("None"));
			ui.cbType->addItem(i18n("Bars"));
			ui.cbType->addItem(i18n("Envelope"));
			ui.cbType->addItem(i18n("Drop Lines"));
			ui.cbType->addItem(i18n("Half-Bars"));
		}
	} else if (m_line->errorBarsTypeAvailable()) {
		ui.lType->show();
		ui.cbType->show();
		ui.lErrorBarsCapSize->show();
		ui.sbErrorBarsCapSize->show();

		if (ui.cbType->count() == 0) {
			Lock lock(m_initializing);
			QPainter pa;
			int iconSize = 20;
			QPixmap pm(iconSize, iconSize);
			pm.fill(Qt::transparent);
			pa.begin(&pm);
			pa.setRenderHint(QPainter::Antialiasing);
			pa.drawLine(3, 10, 17, 10); // vert. line
			pa.drawLine(10, 3, 10, 17); // hor. line
			pa.end();
			ui.cbType->addItem(i18n("Bars"));
			ui.cbType->setItemIcon(0, pm);

			pm.fill(Qt::transparent);
			pa.begin(&pm);
			pa.setRenderHint(QPainter::Antialiasing);
			pa.setBrush(Qt::SolidPattern);
			pa.drawLine(3, 10, 17, 10); // vert. line
			pa.drawLine(10, 3, 10, 17); // hor. line
			pa.drawLine(7, 3, 13, 3); // upper cap
			pa.drawLine(7, 17, 13, 17); // bottom cap
			pa.drawLine(3, 7, 3, 13); // left cap
			pa.drawLine(17, 7, 17, 13); // right cap
			pa.end();
			ui.cbType->addItem(i18n("Bars with Ends"));
			ui.cbType->setItemIcon(1, pm);
		}
	} else if (m_prefix == QLatin1String("DropLine")) {
		ui.lType->show();
		ui.cbType->show();
		ui.lErrorBarsCapSize->hide();
		ui.sbErrorBarsCapSize->hide();

		if (ui.cbType->count() == 0) {
			Lock lock(m_initializing);
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
		ui.lErrorBarsCapSize->hide();
		ui.sbErrorBarsCapSize->hide();
	}

	load();

	connect(m_line, &Line::histogramLineTypeChanged, this, &LineWidget::histogramLineTypeChanged);
	connect(m_line, &Line::errorBarsTypeChanged, this, &LineWidget::errorBarsTypeChanged);
	connect(m_line, &Line::errorBarsCapSizeChanged, this, &LineWidget::errorBarsCapSizeChanged);
	connect(m_line, &Line::dropLineTypeChanged, this, &LineWidget::dropLineTypeChanged);

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
	SET_NUMBER_LOCALE
	ui.sbErrorBarsCapSize->setLocale(numberLocale);
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
	} else if (m_line->errorBarsTypeAvailable()) {
		auto type = XYCurve::ErrorBarsType(index);
		bool b = (type == XYCurve::ErrorBarsType::WithEnds);
		ui.lErrorBarsCapSize->setVisible(b);
		ui.sbErrorBarsCapSize->setVisible(b);

		if (!m_initializing) {
			for (auto* line : m_lines)
				line->setErrorBarsType(type);
		}
	} else if (m_prefix == QLatin1String("DropLine")) {
		auto type = XYCurve::DropLineType(index);
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

void LineWidget::capSizeChanged(double value) const {
	if (m_initializing)
		return;

	const double size = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* line : m_lines)
		line->setErrorBarsCapSize(size);
}

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
	const double width = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* line : m_lines) {
		pen = line->pen();
		pen.setWidthF(width);
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
//*********** SLOTs for changes triggered in Line *************
//*************************************************************
void LineWidget::histogramLineTypeChanged(Histogram::LineType type) {
	m_initializing = true;
	ui.cbType->setCurrentIndex(static_cast<int>(type));
	m_initializing = false;
}

void LineWidget::errorBarsTypeChanged(XYCurve::ErrorBarsType type) {
	m_initializing = true;
	ui.cbType->setCurrentIndex(static_cast<int>(type));
	m_initializing = false;
}

void LineWidget::errorBarsCapSizeChanged(double size) {
	m_initializing = true;
	ui.sbErrorBarsCapSize->setValue(Worksheet::convertFromSceneUnits(size, Worksheet::Unit::Point));
	m_initializing = false;
}

void LineWidget::dropLineTypeChanged(XYCurve::DropLineType type) {
	m_initializing = true;
	ui.cbType->setCurrentIndex(static_cast<int>(type));
	m_initializing = false;
}

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

	if (m_line->histogramLineTypeAvailable())
		ui.cbType->setCurrentIndex(static_cast<int>(m_line->histogramLineType()));
	else if (m_line->errorBarsTypeAvailable()) {
		ui.cbType->setCurrentIndex(static_cast<int>(m_line->errorBarsType()));
		const double size = Worksheet::convertFromSceneUnits(m_line->errorBarsCapSize(), Worksheet::Unit::Point);
		ui.sbErrorBarsCapSize->setValue(size);
	} else if (m_prefix == QLatin1String("DropLine"))
		ui.cbType->setCurrentIndex(static_cast<int>(m_line->dropLineType()));

	const QPen& pen = m_line->pen();
	ui.kcbColor->setColor(pen.color());
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point));
	ui.sbOpacity->setValue(m_line->opacity() * 100);
	GuiTools::updatePenStyles(ui.cbStyle, ui.kcbColor->color());
	ui.cbStyle->setCurrentIndex(static_cast<int>(pen.style()));
}

void LineWidget::loadConfig(const KConfigGroup& group) {
	const Lock lock(m_initializing);

	if (m_line->histogramLineTypeAvailable())
		ui.cbType->setCurrentIndex(group.readEntry(m_prefix + QStringLiteral("Type"), static_cast<int>(m_line->histogramLineType())));
	else if (m_line->errorBarsTypeAvailable()) {
		ui.cbType->setCurrentIndex(group.readEntry(m_prefix + QStringLiteral("Type"), static_cast<int>(m_line->errorBarsType())));
		const double size =
			Worksheet::convertFromSceneUnits(group.readEntry(m_prefix + QStringLiteral("CapSize"), m_line->errorBarsCapSize()), Worksheet::Unit::Point);
		ui.sbErrorBarsCapSize->setValue(size);
	} else if (m_prefix == QLatin1String("DropLine"))
		ui.cbType->setCurrentIndex(group.readEntry("DropLineType", static_cast<int>(m_line->dropLineType())));

	const QPen& pen = m_line->pen();
	ui.cbStyle->setCurrentIndex(group.readEntry(m_prefix + QStringLiteral("Style"), static_cast<int>(pen.style())));
	ui.kcbColor->setColor(group.readEntry(m_prefix + QStringLiteral("Color"), pen.color()));
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(group.readEntry(m_prefix + QStringLiteral("Width"), pen.widthF()), Worksheet::Unit::Point));
	ui.sbOpacity->setValue(group.readEntry(m_prefix + QStringLiteral("Opacity"), m_line->opacity()) * 100);
	GuiTools::updatePenStyles(ui.cbStyle, ui.kcbColor->color());
}

void LineWidget::saveConfig(KConfigGroup& group) const {
	if (m_line->histogramLineTypeAvailable() || m_prefix == QLatin1String("DropLine"))
		group.writeEntry(m_prefix + QStringLiteral("Type"), ui.cbType->currentIndex());
	else if (m_line->errorBarsTypeAvailable()) {
		group.writeEntry(m_prefix + QStringLiteral("Type"), ui.cbType->currentIndex());
		group.writeEntry(m_prefix + QStringLiteral("CapSize"), Worksheet::convertToSceneUnits(ui.sbErrorBarsCapSize->value(), Worksheet::Unit::Point));
	}

	group.writeEntry(m_prefix + QStringLiteral("Style"), ui.cbStyle->currentIndex());
	group.writeEntry(m_prefix + QStringLiteral("Color"), ui.kcbColor->color());
	group.writeEntry(m_prefix + QStringLiteral("Width"), Worksheet::convertToSceneUnits(ui.sbWidth->value(), Worksheet::Unit::Point));
	group.writeEntry(m_prefix + QStringLiteral("Opacity"), ui.sbOpacity->value() / 100.0);
}
