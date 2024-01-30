/*
	File                 : ErrorBarStyleWidget.cpp
	Project              : LabPlot
	Description          : error bar style widget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ErrorBarStyleWidget.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/dockwidgets/BaseDock.h"
#include "kdefrontend/widgets/LineWidget.h"

#include <KConfigGroup>
#include <QPainter>
#include <QTimer>

/*!
	\class ErrorBarStyleWidget
	\brief Widget for editing the properties of a Line object, mostly used in an appropriate dock widget.

	\ingroup kdefrontend
 */
ErrorBarStyleWidget::ErrorBarStyleWidget(QWidget* parent)
	: QWidget(parent) {
	ui.setupUi(this);

	auto* gridLayout = qobject_cast<QGridLayout*>(ui.gridLayout);
	lineWidget = new LineWidget(this);
	gridLayout->addWidget(lineWidget, 2, 0, 1, 3);

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

	connect(ui.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ErrorBarStyleWidget::typeChanged);
	connect(ui.sbCapSize, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &ErrorBarStyleWidget::capSizeChanged);
}

void ErrorBarStyleWidget::setErrorBarStyles(const QList<ErrorBarStyle*>& styles) {
	CONDITIONAL_LOCK_RETURN;
	m_styles = styles;
	m_style = m_styles.first();

	load();

	QList<Line*> lines;
	for (auto* style : styles)
		lines << style->line();
	lineWidget->setLines(lines);

	connect(m_style, &ErrorBarStyle::typeChanged, this, &ErrorBarStyleWidget::errorBarStyleTypeChanged);
	connect(m_style, &ErrorBarStyle::capSizeChanged, this, &ErrorBarStyleWidget::errorBarStyleCapSizeChanged);
}

void ErrorBarStyleWidget::showEvent(QShowEvent* event) {
	QWidget::showEvent(event);
	adjustLayout();
}

/*!
 * this functions adjusts the width of the first column in the layout of ErrorBarStyleWidget
 * to the width of the first column in the layout of the parent widget
 * which ErrorBarStyleWidget is being embedded into.
 */
void ErrorBarStyleWidget::adjustLayout() {
	auto* parentGridLayout = dynamic_cast<QGridLayout*>(parentWidget()->layout());
	if (!parentGridLayout)
		return;

	auto* parentWidget = parentGridLayout->itemAtPosition(0, 0)->widget();
	if (!parentWidget)
		return;

	auto* widget = ui.gridLayout->itemAtPosition(0, 0)->widget();

	if (parentWidget->width() >= widget->width()) {
		ui.gridLayout->activate();
		widget->setMinimumWidth(parentWidget->width());
		updateGeometry();
	} else {
		parentGridLayout->activate();
		parentWidget->setMinimumWidth(widget->width());
		this->parentWidget()->updateGeometry();
	}
}

void ErrorBarStyleWidget::setEnabled(bool enabled) {
	ui.cbType->setEnabled(enabled);
	ui.sbCapSize->setEnabled(enabled);
}

void ErrorBarStyleWidget::updateLocale() {
	const auto numberLocale = QLocale();
	ui.sbCapSize->setLocale(numberLocale);
}

//*************************************************************
//******** SLOTs for changes triggered in ErrorBarStyleWidget **********
//*************************************************************
void ErrorBarStyleWidget::typeChanged(int index) {
	// bool enabled = true;

	auto type = ErrorBarStyle::Type(index);
	bool b = (type == ErrorBarStyle::Type::WithEnds);
	ui.lCapSize->setVisible(b);
	ui.sbCapSize->setVisible(b);

	if (!m_initializing) {
		for (auto* style : m_styles)
			style->setType(type);
	}
}

void ErrorBarStyleWidget::capSizeChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	const double size = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* style : m_styles)
		style->setCapSize(size);
}

//*************************************************************
//*********** SLOTs for changes triggered in Line *************
//*************************************************************
void ErrorBarStyleWidget::errorBarStyleTypeChanged(ErrorBarStyle::Type type) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbType->setCurrentIndex(static_cast<int>(type));
}

void ErrorBarStyleWidget::errorBarStyleCapSizeChanged(double size) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbCapSize->setValue(Worksheet::convertFromSceneUnits(size, Worksheet::Unit::Point));
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void ErrorBarStyleWidget::load() {
	ui.cbType->setCurrentIndex(static_cast<int>(m_style->type()));
	const double size = Worksheet::convertFromSceneUnits(m_style->capSize(), Worksheet::Unit::Point);
	ui.sbCapSize->setValue(size);
}

void ErrorBarStyleWidget::loadConfig(const KConfigGroup& group) {
	ui.cbType->setCurrentIndex(group.readEntry(QStringLiteral("ErrorBarsType"), static_cast<int>(m_style->type())));
	const double size = Worksheet::convertFromSceneUnits(group.readEntry(QStringLiteral("ErrorBarsCapSize"), m_style->capSize()), Worksheet::Unit::Point);
	ui.sbCapSize->setValue(size);
}

void ErrorBarStyleWidget::saveConfig(KConfigGroup& group) const {
	group.writeEntry(QStringLiteral("ErrorBarsType"), ui.cbType->currentIndex());
	group.writeEntry(QStringLiteral("ErrorBarsCapSize"), Worksheet::convertToSceneUnits(ui.sbCapSize->value(), Worksheet::Unit::Point));
}
