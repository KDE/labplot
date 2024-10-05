/*
	File                 : ReferenceRangeDock.cpp
	Project              : LabPlot
	Description          : Dock widget for the reference range on the plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ReferenceRangeDock.h"
#include "backend/worksheet/plots/cartesian/ReferenceRange.h"

#include "frontend/widgets/BackgroundWidget.h"
#include "frontend/widgets/LineWidget.h"

#include <KConfig>
#include <KLocalizedString>
#include <QFile>

ReferenceRangeDock::ReferenceRangeDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setPlotRangeCombobox(ui.cbPlotRanges);
	setBaseWidgets(ui.leName, ui.teComment);
	setVisibilityWidgets(ui.chkVisible);

	ui.cbOrientation->addItem(i18n("Horizontal"));
	ui.cbOrientation->addItem(i18n("Vertical"));

	// background
	auto* layout = static_cast<QGridLayout*>(ui.tabGeneral->layout());
	backgroundWidget = new BackgroundWidget(ui.tabGeneral);
	layout->addWidget(backgroundWidget, 12, 0, 1, 3);

	// border line
	lineWidget = new LineWidget(ui.tabGeneral);
	layout->addWidget(lineWidget, 15, 0, 1, 3);

	// SLOTS
	// General
	connect(ui.cbOrientation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ReferenceRangeDock::orientationChanged);
	connect(ui.sbPositionStart, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &ReferenceRangeDock::positionLogicalStartChanged);
	connect(ui.sbPositionEnd, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &ReferenceRangeDock::positionLogicalEndChanged);
	connect(ui.dtePositionStart, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &ReferenceRangeDock::positionLogicalDateTimeStartChanged);
	connect(ui.dtePositionEnd, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &ReferenceRangeDock::positionLogicalDateTimeEndChanged);
}

void ReferenceRangeDock::setReferenceRanges(QList<ReferenceRange*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_rangeList = list;
	m_range = list.first();
	setAspects(list);
	Q_ASSERT(m_range);

	// initialize widgets for common properties
	QList<Line*> lines;
	QList<Background*> backgrounds;
	for (auto* range : m_rangeList) {
		lines << range->line();
		backgrounds << range->background();
	}
	lineWidget->setLines(lines);
	backgroundWidget->setBackgrounds(backgrounds);

	// show the properties of the first reference range
	this->load();

	updatePlotRangeList();

	// SIGNALs/SLOTs
	connect(m_range, &ReferenceRange::orientationChanged, this, &ReferenceRangeDock::rangeOrientationChanged);
	connect(m_range, &ReferenceRange::positionLogicalStartChanged, this, &ReferenceRangeDock::rangePositionLogicalStartChanged);
	connect(m_range, &ReferenceRange::positionLogicalEndChanged, this, &ReferenceRangeDock::rangePositionLogicalEndChanged);
}

/*
 * updates the locale in the widgets. called when the application settings are changed.
 */
void ReferenceRangeDock::updateLocale() {
	const auto numberLocale = QLocale();
	ui.sbPositionStart->setLocale(numberLocale);
	ui.sbPositionEnd->setLocale(numberLocale);

	// TODO datetime
}

//**********************************************************
//*** SLOTs for changes triggered in ReferenceRangeDock *****
//**********************************************************
// Position
void ReferenceRangeDock::orientationChanged(int index) {
	auto orientation{ReferenceRange::Orientation(index)};
	const auto* plot = static_cast<const CartesianPlot*>(m_range->plot());
	bool numeric;
	if (orientation == ReferenceRange::Orientation::Horizontal) {
		ui.lPositionStart->setText(QLatin1String("Start y:"));
		ui.lPositionEnd->setText(QLatin1String("End y:"));
		ui.lPositionDateTimeStart->setText(QLatin1String("Start y:"));
		ui.lPositionDateTimeEnd->setText(QLatin1String("End y:"));
		numeric = (plot->yRangeFormatDefault() == RangeT::Format::Numeric);
	} else {
		ui.lPositionStart->setText(QLatin1String("Start x:"));
		ui.lPositionEnd->setText(QLatin1String("End x:"));
		ui.lPositionDateTimeStart->setText(QLatin1String("Start x:"));
		ui.lPositionDateTimeEnd->setText(QLatin1String("End x:"));
		numeric = (plot->xRangeFormatDefault() == RangeT::Format::Numeric);
	}

	ui.lPositionStart->setVisible(numeric);
	ui.sbPositionStart->setVisible(numeric);
	ui.lPositionEnd->setVisible(numeric);
	ui.sbPositionEnd->setVisible(numeric);
	ui.lPositionDateTimeStart->setVisible(!numeric);
	ui.dtePositionStart->setVisible(!numeric);
	ui.lPositionDateTimeEnd->setVisible(!numeric);
	ui.dtePositionEnd->setVisible(!numeric);

	CONDITIONAL_LOCK_RETURN;

	for (auto* range : m_rangeList)
		range->setOrientation(orientation);

	if (m_range->orientation() == ReferenceRange::Orientation::Horizontal) {
		ui.sbPositionStart->setValue(m_range->positionLogicalStart().y());
		ui.dtePositionStart->setMSecsSinceEpochUTC(m_range->positionLogicalStart().y());
	} else {
		ui.sbPositionStart->setValue(m_range->positionLogicalStart().x());
		ui.dtePositionStart->setMSecsSinceEpochUTC(m_range->positionLogicalStart().x());
	}

	if (m_range->orientation() == ReferenceRange::Orientation::Horizontal) {
		ui.sbPositionEnd->setValue(m_range->positionLogicalEnd().y());
		ui.dtePositionEnd->setMSecsSinceEpochUTC(m_range->positionLogicalEnd().y());
	} else {
		ui.sbPositionEnd->setValue(m_range->positionLogicalEnd().x());
		ui.dtePositionEnd->setMSecsSinceEpochUTC(m_range->positionLogicalEnd().x());
	}
}

void ReferenceRangeDock::positionLogicalStartChanged(double pos) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* range : m_rangeList) {
		auto positionLogical = range->positionLogicalStart();
		if (range->orientation() == ReferenceRange::Orientation::Horizontal)
			positionLogical.setY(pos);
		else
			positionLogical.setX(pos);
		range->setPositionLogicalStart(positionLogical);
	}
}

void ReferenceRangeDock::positionLogicalEndChanged(double pos) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* range : m_rangeList) {
		auto positionLogical = range->positionLogicalEnd();
		if (range->orientation() == ReferenceRange::Orientation::Horizontal)
			positionLogical.setY(pos);
		else
			positionLogical.setX(pos);
		range->setPositionLogicalEnd(positionLogical);
	}
}

void ReferenceRangeDock::positionLogicalDateTimeStartChanged(qint64 pos) {
	CONDITIONAL_RETURN_NO_LOCK; // Feedback needed

	for (auto* range : m_rangeList) {
		auto positionLogical = range->positionLogicalStart();
		if (range->orientation() == ReferenceRange::Orientation::Horizontal)
			positionLogical.setY(pos);
		else
			positionLogical.setX(pos);
		range->setPositionLogicalStart(positionLogical);
	}
}

void ReferenceRangeDock::positionLogicalDateTimeEndChanged(qint64 pos) {
	CONDITIONAL_RETURN_NO_LOCK; // Feedback needed

	for (auto* range : m_rangeList) {
		auto positionLogical = range->positionLogicalEnd();
		if (range->orientation() == ReferenceRange::Orientation::Horizontal)
			positionLogical.setY(pos);
		else
			positionLogical.setX(pos);
		range->setPositionLogicalEnd(positionLogical);
	}
}

//*************************************************************
//******* SLOTs for changes triggered in ReferenceRange ********
//*************************************************************
void ReferenceRangeDock::rangePositionLogicalStartChanged(const QPointF& positionLogical) {
	CONDITIONAL_LOCK_RETURN;
	if (m_range->orientation() == ReferenceRange::Orientation::Horizontal) {
		ui.sbPositionStart->setValue(positionLogical.y());
		ui.dtePositionStart->setMSecsSinceEpochUTC(positionLogical.y());
	} else {
		ui.sbPositionStart->setValue(positionLogical.x());
		ui.dtePositionStart->setMSecsSinceEpochUTC(positionLogical.x());
	}
}

void ReferenceRangeDock::rangePositionLogicalEndChanged(const QPointF& positionLogical) {
	CONDITIONAL_LOCK_RETURN;
	if (m_range->orientation() == ReferenceRange::Orientation::Horizontal) {
		ui.sbPositionEnd->setValue(positionLogical.y());
		ui.dtePositionEnd->setMSecsSinceEpochUTC(positionLogical.y());
	} else {
		ui.sbPositionEnd->setValue(positionLogical.x());
		ui.dtePositionEnd->setMSecsSinceEpochUTC(positionLogical.x());
	}
}

void ReferenceRangeDock::rangeOrientationChanged(ReferenceRange::Orientation orientation) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbOrientation->setCurrentIndex(static_cast<int>(orientation));
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void ReferenceRangeDock::load() {
	if (!m_range)
		return;

	auto orientation = m_range->orientation();
	ui.cbOrientation->setCurrentIndex(static_cast<int>(orientation));
	orientationChanged(ui.cbOrientation->currentIndex()); // call this to update the position widgets that depend on the orientation

	// position
	const auto* plot = static_cast<const CartesianPlot*>(m_range->plot());
	if (orientation == ReferenceRange::Orientation::Horizontal) {
		if (plot->yRangeFormatDefault() == RangeT::Format::Numeric) {
			ui.sbPositionStart->setValue(m_range->positionLogicalStart().y());
			ui.sbPositionEnd->setValue(m_range->positionLogicalEnd().y());
		} else { // DateTime
			ui.dtePositionStart->setDisplayFormat(plot->rangeDateTimeFormat(Dimension::Y));
			ui.dtePositionStart->setMSecsSinceEpochUTC(m_range->positionLogicalStart().y());
			ui.dtePositionEnd->setDisplayFormat(plot->rangeDateTimeFormat(Dimension::Y));
			ui.dtePositionEnd->setMSecsSinceEpochUTC(m_range->positionLogicalEnd().y());
		}
	} else {
		if (plot->xRangeFormatDefault() == RangeT::Format::Numeric) {
			ui.sbPositionStart->setValue(m_range->positionLogicalStart().x());
			ui.sbPositionEnd->setValue(m_range->positionLogicalEnd().x());
		} else { // DateTime
			ui.dtePositionStart->setDisplayFormat(plot->rangeDateTimeFormat(Dimension::X));
			ui.dtePositionStart->setMSecsSinceEpochUTC(m_range->positionLogicalStart().x());
			ui.dtePositionEnd->setDisplayFormat(plot->rangeDateTimeFormat(Dimension::X));
			ui.dtePositionEnd->setMSecsSinceEpochUTC(m_range->positionLogicalEnd().x());
		}
	}

	ui.chkVisible->setChecked(m_range->isVisible());
}
