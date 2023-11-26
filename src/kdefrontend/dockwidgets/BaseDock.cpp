/*
	File                 : BaseDock.cpp
	Project              : LabPlot
	Description          : Base Dock widget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2019-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "BaseDock.h"
#include "AxisDock.h"
#include "backend/core/AbstractAspect.h"
#include "backend/core/Project.h"
#include "backend/core/Settings.h"
#include "backend/lib/macros.h"

#include "backend/nsl/nsl_math.h"

#include <KConfigGroup>
#include <KLocalizedString>

#include <QComboBox>

BaseDock::BaseDock(QWidget* parent)
	: QWidget(parent) {
	const KConfigGroup group = Settings::group(QStringLiteral("Settings_General"));
	m_units = (Units)group.readEntry("Units", static_cast<int>(Units::Metric));

	if (m_units == Units::Imperial)
		m_worksheetUnit = Worksheet::Unit::Inch;
}

void BaseDock::setPlotRangeCombobox(QComboBox* cb) {
	m_cbPlotRangeList = cb;
}

BaseDock::~BaseDock() {
	if (m_aspectModel)
		delete m_aspectModel;
}

void BaseDock::updatePlotRangeList() {
	auto* element{static_cast<WorksheetElement*>(m_aspect)};
	if (!element) {
		DEBUG(Q_FUNC_INFO << ", WARNING: no worksheet element!")
		return;
	}
	const int cSystemCount{element->coordinateSystemCount()};
	const int cSystemIndex{element->coordinateSystemIndex()};

	if (cSystemCount == 0) {
		DEBUG(Q_FUNC_INFO << ", WARNING: no plot range yet")
		return;
	}
	DEBUG(Q_FUNC_INFO << ", plot ranges count: " << cSystemCount)
	DEBUG(Q_FUNC_INFO << ", current plot range: " << cSystemIndex + 1)

	if (!m_cbPlotRangeList) {
		assert(false);
		DEBUG(Q_FUNC_INFO << ", ERROR: no plot range combo box")
		return;
	}
	// fill ui.cbPlotRanges
	m_suppressPlotRetransform = true;
	m_cbPlotRangeList->clear();
	for (int i{0}; i < cSystemCount; i++)
		m_cbPlotRangeList->addItem(QString::number(i + 1) + QStringLiteral(" : ") + element->coordinateSystemInfo(i));
	m_cbPlotRangeList->setCurrentIndex(cSystemIndex);
	m_suppressPlotRetransform = false;
	// disable when there is only on plot range
	m_cbPlotRangeList->setEnabled(cSystemCount == 1 ? false : true);
}

void BaseDock::plotRangeChanged(int index) {
	if (m_suppressPlotRetransform)
		return;
	DEBUG(Q_FUNC_INFO << ", index = " << index)

	auto* element{static_cast<WorksheetElement*>(m_aspect)};
	CartesianPlot* plot;
	if (element->plot()) {
		plot = element->plot();
	} else {
		plot = dynamic_cast<CartesianPlot*>(m_aspect->parentAspect());
	}

	if (!plot)
		return;

	if (index < 0 || index > plot->coordinateSystemCount()) {
		index = element->coordinateSystemIndex();
		DEBUG(Q_FUNC_INFO << ", using default index " << index)
	}

	const auto xRangeNew = plot->coordinateSystem(index)->range(Dimension::X);
	const auto yRangeNew = plot->coordinateSystem(index)->range(Dimension::Y);

	bool xIndexNewDifferent = false;
	bool yIndexNewDifferent = false;
	QVector<std::shared_ptr<const Range<double>>> xRangesChanged;
	QVector<std::shared_ptr<const Range<double>>> yRangesChanged;
	for (auto aspect : m_aspects) {
		auto* e{static_cast<WorksheetElement*>(aspect)};
		if (index != e->coordinateSystemIndex()) {
			const auto* elementOldCSystem = e->coordinateSystem();
			const auto xRangeOld = elementOldCSystem->range(Dimension::X);
			const auto yRangeOld = elementOldCSystem->range(Dimension::Y);
			// If indices are same, the range will not change, so do not track those
			if (xRangeOld != xRangeNew) {
				xIndexNewDifferent = true;
				if (!xRangesChanged.contains(xRangeOld))
					xRangesChanged.append(xRangeOld);
			}
			if (yRangeOld != yRangeNew) {
				yIndexNewDifferent = true;
				if (!yRangesChanged.contains(yRangeOld))
					yRangesChanged.append(yRangeOld);
			}
			e->setSuppressRetransform(true);
			e->setCoordinateSystemIndex(index);
			e->setSuppressRetransform(false);
			if (dynamic_cast<Axis*>(e) && dynamic_cast<AxisDock*>(this))
				dynamic_cast<AxisDock*>(this)->updateAutoScale();
			updateLocale(); // update line edits
		}
	}

	// Retransform all changed indices and the new indices
	if (!xRangesChanged.contains(xRangeNew) && xIndexNewDifferent)
		xRangesChanged.append(xRangeNew);
	for (const auto& r : xRangesChanged) {
		plot->setRangeDirty(Dimension::X, r, true);
		if (plot->autoScale(Dimension::X, r))
			plot->scaleAuto(Dimension::X, r);
	}

	if (!yRangesChanged.contains(yRangeNew) && yIndexNewDifferent)
		yRangesChanged.append(yRangeNew);
	for (const auto& r : yRangesChanged) {
		plot->setRangeDirty(Dimension::Y, r, true);
		if (plot->autoScale(Dimension::Y, r))
			plot->scaleAuto(Dimension::Y, r);
	}

	plot->WorksheetElementContainer::retransform();
	plot->project()->setChanged(true);
}

void BaseDock::nameChanged() {
	if (m_initializing || !m_aspect)
		return;

	if (!m_aspect->setName(m_leName->text(), AbstractAspect::NameHandling::UniqueRequired)) {
		SET_WARNING_STYLE(m_leName)
		m_leName->setToolTip(i18n("Please choose another name, because this is already in use."));
	} else {
		m_leName->setStyleSheet(QString());
		m_leName->setToolTip(QString());
	}
}

void BaseDock::commentChanged() {
	if (m_initializing || !m_aspect)
		return;

	m_aspect->setComment(m_teComment->text());
}

void BaseDock::aspectDescriptionChanged(const AbstractAspect* aspect) {
	if (m_aspect != aspect)
		return;

	CONDITIONAL_LOCK_RETURN;
	if (aspect->name() != m_leName->text())
		m_leName->setText(aspect->name());
	else if (aspect->comment() != m_teComment->text())
		m_teComment->document()->setPlainText(aspect->comment());
}

AspectTreeModel* BaseDock::aspectModel() {
	if (!m_aspectModel)
		m_aspectModel = new AspectTreeModel(m_aspect->project());

	return m_aspectModel;
}

/*!
 * shows the name and the description of the first selecte aspect,
 * disables the fields "Name" and "Comment" if there are more than one aspects selected.
 */
void BaseDock::updateNameDescriptionWidgets() {
	if (m_aspects.size() == 1) {
		m_leName->setEnabled(true);
		m_teComment->setEnabled(true);
		m_leName->setText(m_aspect->name());
		m_teComment->setText(m_aspect->comment());
	} else {
		m_leName->setEnabled(false);
		m_teComment->setEnabled(false);
		m_leName->setText(QString());
		m_teComment->setText(QString());
	}
	m_leName->setStyleSheet(QString());
	m_leName->setToolTip(QString());
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
	auto singlestep = abs(min - max) / 100;
	spinbox->setSingleStep(singlestep);
	spinbox->setDecimals(nsl_math_decimal_places(singlestep));
}
