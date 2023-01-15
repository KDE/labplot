/*
	File                 : ReferenceRangeDock.cpp
	Project              : LabPlot
	Description          : Dock widget for the reference range on the plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ReferenceRangeDock.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/ReferenceRange.h"

#include "kdefrontend/GuiTools.h"
#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/widgets/BackgroundWidget.h"
#include "kdefrontend/widgets/LineWidget.h"

#include <KConfig>
#include <KLocalizedString>
#include <QFile>

ReferenceRangeDock::ReferenceRangeDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_teComment = ui.teComment;
	m_teComment->setFixedHeight(1.2 * m_leName->height());

	ui.cbOrientation->addItem(i18n("Horizontal"));
	ui.cbOrientation->addItem(i18n("Vertical"));

	ui.lePositionStart->setValidator(new QDoubleValidator(ui.lePositionStart));
	ui.lePositionEnd->setValidator(new QDoubleValidator(ui.lePositionEnd));

	// background
	auto* layout = static_cast<QGridLayout*>(ui.tabGeneral->layout());
	backgroundWidget = new BackgroundWidget(ui.tabGeneral);
	layout->addWidget(backgroundWidget, 12, 0, 1, 3);

	// border line
	lineWidget = new LineWidget(ui.tabGeneral);
	layout->addWidget(lineWidget, 15, 0, 1, 3);

	// SLOTS
	// General
	connect(ui.leName, &QLineEdit::textChanged, this, &ReferenceRangeDock::nameChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &ReferenceRangeDock::commentChanged);

	connect(ui.cbOrientation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ReferenceRangeDock::orientationChanged);
	connect(ui.lePositionStart, &QLineEdit::textChanged, this, &ReferenceRangeDock::positionLogicalStartChanged);
	connect(ui.lePositionEnd, &QLineEdit::textChanged, this, &ReferenceRangeDock::positionLogicalEndChanged);
	connect(ui.dtePositionStart, &QDateTimeEdit::dateTimeChanged, this, &ReferenceRangeDock::positionLogicalDateTimeStartChanged);
	connect(ui.dtePositionEnd, &QDateTimeEdit::dateTimeChanged, this, &ReferenceRangeDock::positionLogicalDateTimeEndChanged);
	connect(ui.cbPlotRanges, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ReferenceRangeDock::plotRangeChanged);
	connect(ui.chkVisible, &QCheckBox::clicked, this, &ReferenceRangeDock::visibilityChanged);
}

void ReferenceRangeDock::setReferenceRanges(QList<ReferenceRange*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_rangeList = list;
	m_range = list.first();
	setAspects(list);
	Q_ASSERT(m_range);

	// if there is more then one point in the list, disable the comment and name widgets in "general"
	if (list.size() == 1) {
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.teComment->setEnabled(true);
		ui.leName->setText(m_range->name());
		ui.teComment->setText(m_range->comment());
	} else {
		ui.lName->setEnabled(false);
		ui.leName->setEnabled(false);
		ui.lComment->setEnabled(false);
		ui.teComment->setEnabled(false);
		ui.leName->setText(QString());
		ui.teComment->setText(QString());
	}
	ui.leName->setStyleSheet(QString());
	ui.leName->setToolTip(QString());

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

	updatePlotRanges();

	// SIGNALs/SLOTs
	connect(m_range, &AbstractAspect::aspectDescriptionChanged, this, &ReferenceRangeDock::aspectDescriptionChanged);
	connect(m_range, &WorksheetElement::plotRangeListChanged, this, &ReferenceRangeDock::updatePlotRanges);
	connect(m_range, &ReferenceRange::visibleChanged, this, &ReferenceRangeDock::rangeVisibilityChanged);

	// position
	connect(m_range, &ReferenceRange::orientationChanged, this, &ReferenceRangeDock::rangeOrientationChanged);
	connect(m_range, &ReferenceRange::positionLogicalStartChanged, this, &ReferenceRangeDock::rangePositionLogicalStartChanged);
	connect(m_range, &ReferenceRange::positionLogicalEndChanged, this, &ReferenceRangeDock::rangePositionLogicalEndChanged);
}

/*
 * updates the locale in the widgets. called when the application settings are changed.
 */
void ReferenceRangeDock::updateLocale() {
	CONDITIONAL_LOCK_RETURN;
	const auto numberLocale = QLocale();
	const auto* plot = static_cast<const CartesianPlot*>(m_range->plot());
	if (m_range->orientation() == ReferenceRange::Orientation::Horizontal) {
		if (plot->yRangeFormatDefault() == RangeT::Format::Numeric) {
			ui.lePositionStart->setText(numberLocale.toString(m_range->positionLogicalStart().y()));
			ui.lePositionStart->setText(numberLocale.toString(m_range->positionLogicalEnd().y()));
		}
		// TODO datetime
	} else {
		if (plot->xRangeFormatDefault() == RangeT::Format::Numeric) {
			ui.lePositionStart->setText(numberLocale.toString(m_range->positionLogicalStart().x()));
			ui.lePositionEnd->setText(numberLocale.toString(m_range->positionLogicalEnd().x()));
		}
		// TODO datetime
	}
}

void ReferenceRangeDock::updatePlotRanges() {
	updatePlotRangeList(ui.cbPlotRanges);
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
	ui.lePositionStart->setVisible(numeric);
	ui.lPositionEnd->setVisible(numeric);
	ui.lePositionEnd->setVisible(numeric);
	ui.lPositionDateTimeStart->setVisible(!numeric);
	ui.dtePositionStart->setVisible(!numeric);
	ui.lPositionDateTimeEnd->setVisible(!numeric);
	ui.dtePositionEnd->setVisible(!numeric);

	CONDITIONAL_LOCK_RETURN;

	for (auto* range : m_rangeList)
		range->setOrientation(orientation);

	// call these slots to show the start and end values depending on the new orientation
	rangePositionLogicalStartChanged(m_range->positionLogicalStart());
	rangePositionLogicalEndChanged(m_range->positionLogicalEnd());
}

void ReferenceRangeDock::positionLogicalStartChanged(const QString& value) {
	CONDITIONAL_LOCK_RETURN;

	bool ok;
	const auto numberLocale = QLocale();
	const double pos = numberLocale.toDouble(value, &ok);
	if (ok) {
		for (auto* range : m_rangeList) {
			auto positionLogical = range->positionLogicalStart();
			if (range->orientation() == ReferenceRange::Orientation::Horizontal)
				positionLogical.setY(pos);
			else
				positionLogical.setX(pos);
			range->setPositionLogicalStart(positionLogical);
		}
	}
}

void ReferenceRangeDock::positionLogicalEndChanged(const QString& value) {
	CONDITIONAL_LOCK_RETURN;

	bool ok;
	const auto numberLocale = QLocale();
	const double pos = numberLocale.toDouble(value, &ok);
	if (ok) {
		for (auto* range : m_rangeList) {
			auto positionLogical = range->positionLogicalEnd();
			if (range->orientation() == ReferenceRange::Orientation::Horizontal)
				positionLogical.setY(pos);
			else
				positionLogical.setX(pos);
			range->setPositionLogicalEnd(positionLogical);
		}
	}
}

void ReferenceRangeDock::positionLogicalDateTimeStartChanged(const QDateTime& dateTime) {
	CONDITIONAL_LOCK_RETURN;

	quint64 pos = dateTime.toMSecsSinceEpoch();
	for (auto* range : m_rangeList) {
		auto positionLogical = range->positionLogicalStart();
		if (range->orientation() == ReferenceRange::Orientation::Horizontal)
			positionLogical.setY(pos);
		else
			positionLogical.setX(pos);
		range->setPositionLogicalStart(positionLogical);
	}
}

void ReferenceRangeDock::positionLogicalDateTimeEndChanged(const QDateTime& dateTime) {
	CONDITIONAL_LOCK_RETURN;

	quint64 pos = dateTime.toMSecsSinceEpoch();
	for (auto* range : m_rangeList) {
		auto positionLogical = range->positionLogicalEnd();
		if (range->orientation() == ReferenceRange::Orientation::Horizontal)
			positionLogical.setY(pos);
		else
			positionLogical.setX(pos);
		range->setPositionLogicalEnd(positionLogical);
	}
}

void ReferenceRangeDock::visibilityChanged(bool state) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* range : m_rangeList)
		range->setVisible(state);
}

//*************************************************************
//******* SLOTs for changes triggered in ReferenceRange ********
//*************************************************************
void ReferenceRangeDock::rangePositionLogicalStartChanged(const QPointF& positionLogical) {
	CONDITIONAL_LOCK_RETURN;
	const auto numberLocale = QLocale();
	if (m_range->orientation() == ReferenceRange::Orientation::Horizontal) {
		ui.lePositionStart->setText(numberLocale.toString(positionLogical.y()));
		ui.dtePositionStart->setDateTime(QDateTime::fromMSecsSinceEpoch(positionLogical.y()));
	} else {
		ui.lePositionStart->setText(numberLocale.toString(positionLogical.x()));
		ui.dtePositionStart->setDateTime(QDateTime::fromMSecsSinceEpoch(positionLogical.x()));
	}
}

void ReferenceRangeDock::rangePositionLogicalEndChanged(const QPointF& positionLogical) {
	CONDITIONAL_LOCK_RETURN;
	const auto numberLocale = QLocale();
	if (m_range->orientation() == ReferenceRange::Orientation::Horizontal) {
		ui.lePositionEnd->setText(numberLocale.toString(positionLogical.y()));
		ui.dtePositionEnd->setDateTime(QDateTime::fromMSecsSinceEpoch(positionLogical.y()));
	} else {
		ui.lePositionEnd->setText(numberLocale.toString(positionLogical.x()));
		ui.dtePositionEnd->setDateTime(QDateTime::fromMSecsSinceEpoch(positionLogical.x()));
	}
}

void ReferenceRangeDock::rangeOrientationChanged(ReferenceRange::Orientation orientation) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbOrientation->setCurrentIndex(static_cast<int>(orientation));
}

void ReferenceRangeDock::rangeVisibilityChanged(bool on) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkVisible->setChecked(on);
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void ReferenceRangeDock::load() {
	if (!m_range)
		return;

	const auto numberLocale = QLocale();
	auto orientation = m_range->orientation();
	ui.cbOrientation->setCurrentIndex(static_cast<int>(orientation));
	orientationChanged(ui.cbOrientation->currentIndex()); // call this to update the position widgets that depend on the orientation

	// position
	const auto* plot = static_cast<const CartesianPlot*>(m_range->plot());
	if (orientation == ReferenceRange::Orientation::Horizontal) {
		if (plot->yRangeFormatDefault() == RangeT::Format::Numeric) {
			ui.lePositionStart->setText(numberLocale.toString(m_range->positionLogicalStart().y()));
			ui.lePositionEnd->setText(numberLocale.toString(m_range->positionLogicalEnd().y()));
		} else { // DateTime
			ui.dtePositionStart->setDisplayFormat(plot->rangeDateTimeFormat(Dimension::Y));
			ui.dtePositionStart->setDateTime(QDateTime::fromMSecsSinceEpoch(m_range->positionLogicalStart().y()));
			ui.dtePositionEnd->setDisplayFormat(plot->rangeDateTimeFormat(Dimension::Y));
			ui.dtePositionEnd->setDateTime(QDateTime::fromMSecsSinceEpoch(m_range->positionLogicalEnd().y()));
		}
	} else {
		if (plot->xRangeFormatDefault() == RangeT::Format::Numeric) {
			ui.lePositionStart->setText(numberLocale.toString(m_range->positionLogicalStart().x()));
			ui.lePositionEnd->setText(numberLocale.toString(m_range->positionLogicalEnd().x()));
		} else { // DateTime
			ui.dtePositionStart->setDisplayFormat(plot->rangeDateTimeFormat(Dimension::X));
			ui.dtePositionStart->setDateTime(QDateTime::fromMSecsSinceEpoch(m_range->positionLogicalStart().x()));
			ui.dtePositionEnd->setDisplayFormat(plot->rangeDateTimeFormat(Dimension::X));
			ui.dtePositionEnd->setDateTime(QDateTime::fromMSecsSinceEpoch(m_range->positionLogicalEnd().x()));
		}
	}
}
