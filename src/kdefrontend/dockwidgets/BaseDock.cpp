/***************************************************************************
	File                 : BaseDock.cpp
	Project              : LabPlot
	Description          : Base Dock widget
	--------------------------------------------------------------------
	Copyright            : (C) 2019 Martin Marmsoler (martin.marmsoler@gmail.com)
	Copyright            : (C) 2019-2020 Alexander Semke (alexander.semke@web.de)
	Copyright            : (C) 2021 Stefan Gerlach (stefan.gerlach@uni.kn)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

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
	const int cSystemCount{ element->coordinateSystemCount() };
	const int cSystemIndex{ element->coordinateSystemIndex() };

	if (cSystemCount == 0) {
		DEBUG(Q_FUNC_INFO << ", WARNING: no plot range yet")
		return;
	}
	DEBUG(Q_FUNC_INFO << ", plot ranges count: " << cSystemCount)
	DEBUG(Q_FUNC_INFO << ", current plot range: " << cSystemIndex+1)

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
	const auto* plot = dynamic_cast<const CartesianPlot*>(m_aspect->parentAspect());
	if (index < 0 || index > plot->coordinateSystemCount()) {
		DEBUG(Q_FUNC_INFO << ", index " << index << " out of range")
		return;
	}

	auto* element{ static_cast<WorksheetElement*>(m_aspect) };
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

	m_aspect->setComment(m_leComment->text());
}
