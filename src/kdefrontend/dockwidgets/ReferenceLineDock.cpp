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
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/ReferenceLine.h"

#include "kdefrontend/GuiTools.h"
#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/widgets/LineWidget.h"

#include <KConfig>
#include <KLocalizedString>

ReferenceLineDock::ReferenceLineDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_teComment = ui.teComment;
	m_teComment->setFixedHeight(1.2 * m_leName->height());

	ui.cbOrientation->addItem(i18n("Horizontal"));
	ui.cbOrientation->addItem(i18n("Vertical"));

	ui.lePosition->setValidator(new QDoubleValidator(ui.lePosition));

	auto* gridLayout = qobject_cast<QGridLayout*>(ui.tabGeneral->layout());
	lineWidget = new LineWidget(ui.tabGeneral);
	gridLayout->addWidget(lineWidget, 9, 0, 1, 3);

	// SLOTS
	// General
	connect(ui.leName, &QLineEdit::textChanged, this, &ReferenceLineDock::nameChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &ReferenceLineDock::commentChanged);

	connect(ui.cbOrientation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ReferenceLineDock::orientationChanged);
	connect(ui.lePosition, &QLineEdit::textChanged, this, &ReferenceLineDock::positionLogicalChanged);
	connect(ui.dtePosition, &QDateTimeEdit::dateTimeChanged, this, &ReferenceLineDock::positionLogicalDateTimeChanged);
	connect(ui.cbPlotRanges, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ReferenceLineDock::plotRangeChanged);
	connect(ui.chkVisible, &QCheckBox::clicked, this, &ReferenceLineDock::visibilityChanged);
}

void ReferenceLineDock::setReferenceLines(QList<ReferenceLine*> list) {
	m_initializing = true;
	m_linesList = list;
	m_line = list.first();
	setAspects(list);
	Q_ASSERT(m_line);

	// if there is more then one point in the list, disable the comment and name widgets in "general"
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
	SET_NUMBER_LOCALE
	Lock lock(m_initializing);
	const auto* plot = static_cast<const CartesianPlot*>(m_line->plot());
	if (m_line->orientation() == ReferenceLine::Orientation::Horizontal) {
		if (plot->yRangeFormatDefault() == RangeT::Format::Numeric)
			ui.lePosition->setText(numberLocale.toString(m_line->positionLogical().y()));
	} else {
		if (plot->xRangeFormatDefault() == RangeT::Format::Numeric)
			ui.lePosition->setText(numberLocale.toString(m_line->positionLogical().x()));
	}

	lineWidget->updateLocale();
}

void ReferenceLineDock::updatePlotRanges() {
	updatePlotRangeList(ui.cbPlotRanges);
}

//**********************************************************
//*** SLOTs for changes triggered in ReferenceLineDock *****
//**********************************************************
// Position
void ReferenceLineDock::orientationChanged(int index) {
	auto orientation{ReferenceLine::Orientation(index)};
	const auto* plot = static_cast<const CartesianPlot*>(m_line->plot());
	bool numeric;
	if (orientation == ReferenceLine::Orientation::Horizontal) {
		ui.lPosition->setText(QLatin1String("y:"));
		ui.lPositionDateTime->setText(QLatin1String("y:"));
		numeric = (plot->yRangeFormatDefault() == RangeT::Format::Numeric);
	} else {
		ui.lPosition->setText(QLatin1String("x:"));
		ui.lPositionDateTime->setText(QLatin1String("x:"));
		numeric = (plot->xRangeFormatDefault() == RangeT::Format::Numeric);
	}

	ui.lPosition->setVisible(numeric);
	ui.lePosition->setVisible(numeric);
	ui.lPositionDateTime->setVisible(!numeric);
	ui.dtePosition->setVisible(!numeric);

	if (m_initializing)
		return;

	for (auto* line : m_linesList)
		line->setOrientation(orientation);

	// call this slot to show the x or y value depending on the new orientation
	linePositionLogicalChanged(m_line->positionLogical());
}

void ReferenceLineDock::positionLogicalChanged(const QString& value) {
	if (m_initializing)
		return;

	bool ok;
	SET_NUMBER_LOCALE
	const double pos = numberLocale.toDouble(value, &ok);
	if (ok) {
		for (auto* line : m_linesList) {
			auto positionLogical = line->positionLogical();
			if (line->orientation() == ReferenceLine::Orientation::Horizontal)
				positionLogical.setY(pos);
			else
				positionLogical.setX(pos);
			line->setPositionLogical(positionLogical);
		}
	}
}

void ReferenceLineDock::positionLogicalDateTimeChanged(const QDateTime& dateTime) {
	if (m_initializing)
		return;

	quint64 pos = dateTime.toMSecsSinceEpoch();
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
	if (m_initializing)
		return;

	for (auto* line : m_linesList)
		line->setVisible(state);
}

//*************************************************************
//******* SLOTs for changes triggered in ReferenceLine ********
//*************************************************************
void ReferenceLineDock::linePositionLogicalChanged(const QPointF& positionLogical) {
	const Lock lock(m_initializing);
	SET_NUMBER_LOCALE
	if (m_line->orientation() == ReferenceLine::Orientation::Horizontal) {
		ui.lePosition->setText(numberLocale.toString(positionLogical.y()));
		ui.dtePosition->setDateTime(QDateTime::fromMSecsSinceEpoch(positionLogical.y()));
	} else {
		ui.lePosition->setText(numberLocale.toString(positionLogical.x()));
		ui.dtePosition->setDateTime(QDateTime::fromMSecsSinceEpoch(positionLogical.x()));
	}
}

void ReferenceLineDock::lineOrientationChanged(ReferenceLine::Orientation orientation) {
	m_initializing = true;
	ui.cbOrientation->setCurrentIndex(static_cast<int>(orientation));
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

	const Lock lock(m_initializing);

	SET_NUMBER_LOCALE
	auto orientation = m_line->orientation();
	ui.cbOrientation->setCurrentIndex(static_cast<int>(orientation));
	orientationChanged(ui.cbOrientation->currentIndex()); // call this to update the position widgets that depend on the orientation

	// position
	const auto* plot = static_cast<const CartesianPlot*>(m_line->plot());
	if (orientation == ReferenceLine::Orientation::Horizontal) {
		if (plot->yRangeFormatDefault() == RangeT::Format::Numeric)
			ui.lePosition->setText(numberLocale.toString(m_line->positionLogical().y()));
		else { // DateTime
			ui.dtePosition->setDisplayFormat(plot->rangeDateTimeFormat(Dimension::Y));
			ui.dtePosition->setDateTime(QDateTime::fromMSecsSinceEpoch(m_line->positionLogical().y()));
		}
	} else {
		if (plot->xRangeFormatDefault() == RangeT::Format::Numeric)
			ui.lePosition->setText(numberLocale.toString(m_line->positionLogical().x()));
		else { // DateTime
			ui.dtePosition->setDisplayFormat(plot->rangeDateTimeFormat(Dimension::X));
			ui.dtePosition->setDateTime(QDateTime::fromMSecsSinceEpoch(m_line->positionLogical().x()));
		}
	}
}
