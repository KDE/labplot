/*
	File                 : BaseDock.cpp
	Project              : LabPlot
	Description          : Base Dock widget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019 Martin Marmsoler (martin.marmsoler@gmail.com)
	SPDX-FileCopyrightText: 2019-2020 Alexander Semke (alexander.semke@web.de)
	SPDX-FileCopyrightText: 2021 Stefan Gerlach (stefan.gerlach@uni.kn)
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "BaseDock.h"
#include "AxisDock.h"
#include "backend/core/AbstractAspect.h"
#include "backend/core/Project.h"


#include <KLocalizedString>
#include <KConfigGroup>
#include <KSharedConfig>

#include <QComboBox>

BaseDock::BaseDock(QWidget* parent) : QWidget(parent) {
	const KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Settings_General"));
	m_units = (Units)group.readEntry("Units", static_cast<int>(Units::Metric));

	if (m_units == Units::Imperial)
		m_worksheetUnit = Worksheet::Unit::Inch;
}

BaseDock::~BaseDock() = default;

void BaseDock::updatePlotRangeList(QComboBox* cb) const {
	DEBUG(Q_FUNC_INFO)
	auto* element{ static_cast<WorksheetElement*>(m_aspect) };
	if (!element) {
		DEBUG(Q_FUNC_INFO << ", WARNING: no worksheet element!")
		return;
	}
	const int cSystemCount{ element->coordinateSystemCount() };
	const int cSystemIndex{ element->coordinateSystemIndex() };

	if (cSystemCount == 0) {
		DEBUG(Q_FUNC_INFO << ", WARNING: no plot range yet")
		return;
	}
	DEBUG(Q_FUNC_INFO << ", plot ranges count: " << cSystemCount)
	DEBUG(Q_FUNC_INFO << ", current plot range: " << cSystemIndex+1)

	if (!cb) {
		DEBUG(Q_FUNC_INFO << ", ERROR: no plot range combo box")
		return;
	}
	// fill ui.cbPlotRanges
	cb->clear();
	for (int i{0}; i < cSystemCount; i++)
		cb->addItem( QString::number(i+1) + QLatin1String(" : ") + element->coordinateSystemInfo(i) );
	cb->setCurrentIndex(cSystemIndex);
	// disable when there is only on plot range
	cb->setEnabled(cSystemCount == 1 ? false : true);
}

void BaseDock::plotRangeChanged(int index) {
	DEBUG(Q_FUNC_INFO << ", index = " << index)

	auto* element{ static_cast<WorksheetElement*>(m_aspect) };
	const CartesianPlot* plot;
	if (element->plot()) {
		plot = element->plot();
	} else {
		plot = dynamic_cast<const CartesianPlot*>(m_aspect->parentAspect());
	}

	if (!plot)
		return;

	if (index < 0 || index > plot->coordinateSystemCount()) {
		DEBUG(Q_FUNC_INFO << ", index " << index << " out of range")
		return;
	}

	if (index != element->coordinateSystemIndex()) {
		element->setCoordinateSystemIndex(index);
		if (dynamic_cast<Axis*>(element))
			dynamic_cast<AxisDock*>(this)->updateAutoScale();
		updateLocale();		// update line edits
		element->retransform();	// redraw
		element->project()->setChanged(true);
	}
}

void BaseDock::nameChanged() {
	if (m_initializing || !m_aspect)
		return;

	if (!m_aspect->setName(m_leName->text(), false)) {
		m_leName->setStyleSheet("background:red;");
		m_leName->setToolTip(i18n("Please choose another name, because this is already in use."));
		return;
	}

	m_leName->setStyleSheet("");
	m_leName->setToolTip("");
}

void BaseDock::commentChanged() {
	if (m_initializing || !m_aspect)
		return;

	m_aspect->setComment(m_teComment->text());
}

void BaseDock::aspectDescriptionChanged(const AbstractAspect* aspect) {
	if (m_aspect != aspect)
		return;

	Lock lock(m_initializing);
	if (aspect->name() != m_leName->text())
		m_leName->setText(aspect->name());
	else if (aspect->comment() != m_teComment->text())
		m_teComment->document()->setPlainText(aspect->comment());
}

void BaseDock::spinBoxCalculateMinMax(QDoubleSpinBox* spinbox, Range<double> range, double newValue) {
    double min, max;
    if (range.start() > range.end()) {
        min = range.end();
        max = range.start();
    } else {
        min = range.start();
        max = range.end();
    }

    if (newValue != NAN) {
        if (newValue < min)
            min = newValue;
        if (newValue > max)
            max = newValue;
    }
    spinbox->setMinimum(min);
    spinbox->setMaximum(max);
    auto singlestep = abs(min - max)/100;
    spinbox->setSingleStep(singlestep);
    spinbox->setDecimals(nsl_math_decimal_places(singlestep));
}
