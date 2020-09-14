/***************************************************************************
	File                 : BaseDock.cpp
	Project              : LabPlot
	Description          : Base Dock widget
	--------------------------------------------------------------------
	Copyright            : (C) 2019 Martin Marmsoler (martin.marmsoler@gmail.com)
	Copyright            : (C) 2019-2020 Alexander Semke (alexander.semke@web.de)

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
#include "backend/core/AbstractAspect.h"

#include <KLocalizedString>
#include <KConfigGroup>
#include <KSharedConfig>

BaseDock::BaseDock(QWidget* parent) : QWidget(parent) {
	const KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Settings_General"));
	m_units = (Units)group.readEntry("Units", static_cast<int>(Units::Metric));

	if (m_units == Units::Imperial)
		m_worksheetUnit = Worksheet::Unit::Inch;
}

BaseDock::~BaseDock() = default;

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
