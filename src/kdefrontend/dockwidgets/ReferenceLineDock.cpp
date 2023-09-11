/*
	File                 : ReferenceLineDock.cpp
	Project              : LabPlot
	Description          : Dock widget for the reference line on the plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ReferenceLineDock.h"
#include "backend/worksheet/plots/cartesian/ReferenceLine.h"
#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/widgets/LineWidget.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

ReferenceLineDock::ReferenceLineDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_teComment = ui.teComment;
	m_teComment->setFixedHeight(1.2 * m_leName->height());

	ui.cbOrientation->addItem(i18n("Horizontal"));
	ui.cbOrientation->addItem(i18n("Vertical"));

	auto* layout = static_cast<QHBoxLayout*>(ui.tabLine->layout());
	lineWidget = new LineWidget(ui.tabLine);
	layout->insertWidget(0, lineWidget);

	// SLOTS
	// General
	connect(ui.leName, &QLineEdit::textChanged, this, &ReferenceLineDock::nameChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &ReferenceLineDock::commentChanged);

	connect(ui.cbOrientation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ReferenceLineDock::orientationChanged);
	connect(ui.sbPosition, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &ReferenceLineDock::positionLogicalChanged);
	connect(ui.dtePosition, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &ReferenceLineDock::positionLogicalDateTimeChanged);
	connect(ui.cbPlotRanges, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ReferenceLineDock::plotRangeChanged);
	connect(ui.chkVisible, &QCheckBox::clicked, this, &ReferenceLineDock::visibilityChanged);

	// Template handler
	auto* frame = new QFrame(this);
	auto* hlayout = new QHBoxLayout(frame);
	hlayout->setContentsMargins(0, 11, 0, 11);

	auto* templateHandler = new TemplateHandler(this, QLatin1String("ReferenceLine"));
	hlayout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &ReferenceLineDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &ReferenceLineDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &ReferenceLineDock::info);

	ui.verticalLayout->addWidget(frame);
}

void ReferenceLineDock::setReferenceLines(QList<ReferenceLine*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_linesList = list;
	m_line = list.first();
	setAspects(list);
	Q_ASSERT(m_line);

	// show the properties of the first reference line
	this->load();

	QList<Line*> lines;
	for (auto* line : m_linesList)
		lines << line->line();
	lineWidget->setLines(lines);

	updatePlotRanges();

	// SIGNALs/SLOTs
	connect(m_line, &AbstractAspect::aspectDescriptionChanged, this, &ReferenceLineDock::aspectDescriptionChanged);
	connect(m_line, &WorksheetElement::plotRangeListChanged, this, &ReferenceLineDock::updatePlotRanges);
	connect(m_line, &ReferenceLine::visibleChanged, this, &ReferenceLineDock::lineVisibilityChanged);

	// position
	connect(m_line, &ReferenceLine::orientationChanged, this, &ReferenceLineDock::lineOrientationChanged);
	connect(m_line, &ReferenceLine::positionLogicalChanged, this, &ReferenceLineDock::linePositionLogicalChanged);
}

/*
 * updates the locale in the widgets. called when the application settings are changed.
 */
void ReferenceLineDock::updateLocale() {
	CONDITIONAL_LOCK_RETURN;
	const auto* plot = static_cast<const CartesianPlot*>(m_line->plot());
	if (m_line->orientation() == ReferenceLine::Orientation::Horizontal) {
		if (plot->yRangeFormatDefault() == RangeT::Format::Numeric)
			ui.sbPosition->setValue(m_line->positionLogical().y());
	} else {
		if (plot->xRangeFormatDefault() == RangeT::Format::Numeric)
			ui.sbPosition->setValue(m_line->positionLogical().x());
	}

	lineWidget->updateLocale();
}

void ReferenceLineDock::updatePlotRanges() {
	updatePlotRangeList(ui.cbPlotRanges);
}

void ReferenceLineDock::updateWidgetsOrientation(ReferenceLine::Orientation orientation) {
	const auto* plot = static_cast<const CartesianPlot*>(m_line->plot());
	bool numeric;
	if (orientation == ReferenceLine::Orientation::Horizontal) {
		ui.lPosition->setText(QStringLiteral("y:"));
		ui.lPositionDateTime->setText(QStringLiteral("y:"));
		numeric = (plot->yRangeFormatDefault() == RangeT::Format::Numeric);
	} else {
		ui.lPosition->setText(QStringLiteral("x:"));
		ui.lPositionDateTime->setText(QStringLiteral("x:"));
		numeric = (plot->xRangeFormatDefault() == RangeT::Format::Numeric);
	}

	ui.lPosition->setVisible(numeric);
	ui.sbPosition->setVisible(numeric);
	ui.lPositionDateTime->setVisible(!numeric);
	ui.dtePosition->setVisible(!numeric);
}

//**********************************************************
//*** SLOTs for changes triggered in ReferenceLineDock *****
//**********************************************************
// Position
void ReferenceLineDock::orientationChanged(int index) {
	auto orientation{ReferenceLine::Orientation(index)};
	updateWidgetsOrientation(orientation);

	CONDITIONAL_LOCK_RETURN;

	for (auto* line : m_linesList)
		line->setOrientation(orientation);

	// call this slot to show the x or y value depending on the new orientation
	linePositionLogicalChanged(m_line->positionLogical());
}

void ReferenceLineDock::positionLogicalChanged(double pos) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* line : m_linesList) {
		auto positionLogical = line->positionLogical();
		if (line->orientation() == ReferenceLine::Orientation::Horizontal)
			positionLogical.setY(pos);
		else
			positionLogical.setX(pos);
		line->setPositionLogical(positionLogical);
	}
}

void ReferenceLineDock::positionLogicalDateTimeChanged(qint64 pos) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* line : m_linesList) {
		auto positionLogical = line->positionLogical();
		if (line->orientation() == ReferenceLine::Orientation::Horizontal)
			positionLogical.setY(pos);
		else
			positionLogical.setX(pos);
		line->setPositionLogical(positionLogical);
	}
}

void ReferenceLineDock::visibilityChanged(bool state) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* line : m_linesList)
		line->setVisible(state);
}

//*************************************************************
//******* SLOTs for changes triggered in ReferenceLine ********
//*************************************************************
void ReferenceLineDock::linePositionLogicalChanged(const QPointF& positionLogical) {
	CONDITIONAL_LOCK_RETURN;
	if (m_line->orientation() == ReferenceLine::Orientation::Horizontal) {
		ui.sbPosition->setValue(positionLogical.y());
		ui.dtePosition->setMSecsSinceEpochUTC(positionLogical.y());
	} else {
		ui.sbPosition->setValue(positionLogical.x());
		ui.dtePosition->setMSecsSinceEpochUTC(positionLogical.x());
	}
}

void ReferenceLineDock::lineOrientationChanged(ReferenceLine::Orientation orientation) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbOrientation->setCurrentIndex(static_cast<int>(orientation));
}

void ReferenceLineDock::lineVisibilityChanged(bool on) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkVisible->setChecked(on);
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void ReferenceLineDock::load() {
	if (!m_line)
		return;

	// No lock!

	auto orientation = m_line->orientation();
	ui.cbOrientation->setCurrentIndex(static_cast<int>(orientation));
	updateWidgetsOrientation(orientation);

	// position
	const auto* plot = static_cast<const CartesianPlot*>(m_line->plot());
	if (orientation == ReferenceLine::Orientation::Horizontal) {
		if (plot->yRangeFormatDefault() == RangeT::Format::Numeric)
			ui.sbPosition->setValue(m_line->positionLogical().y());
		else { // DateTime
			ui.dtePosition->setDisplayFormat(plot->rangeDateTimeFormat(Dimension::Y));
			ui.dtePosition->setMSecsSinceEpochUTC(m_line->positionLogical().y());
		}
	} else {
		if (plot->xRangeFormatDefault() == RangeT::Format::Numeric)
			ui.sbPosition->setValue(m_line->positionLogical().x());
		else { // DateTime
			ui.dtePosition->setDisplayFormat(plot->rangeDateTimeFormat(Dimension::X));
			ui.dtePosition->setMSecsSinceEpochUTC(m_line->positionLogical().x());
		}
	}

	ui.chkVisible->setChecked(m_line->isVisible());
}

void ReferenceLineDock::loadConfigFromTemplate(KConfig& config) {
	auto name = TemplateHandler::templateName(config);
	int size = m_linesList.size();
	if (size > 1)
		m_line->beginMacro(i18n("%1 reference lines: template \"%2\" loaded", size, name));
	else
		m_line->beginMacro(i18n("%1: template \"%2\" loaded", m_line->name(), name));

	lineWidget->loadConfig(config.group("ReferenceLine"));

	m_line->endMacro();
}

void ReferenceLineDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("ReferenceLine"));
	lineWidget->saveConfig(group);
}
