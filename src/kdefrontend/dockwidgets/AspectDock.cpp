/***************************************************************************
    File                 : AspectDock.h
    Project              : LabPlot
    Description          : widget for aspect properties showing name and comments only
    --------------------------------------------------------------------
    Copyright            : (C) 2021 Alexander Semke (alexander.semke@web.de)

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

#include "AspectDock.h"


/*!
  \class AspectDock
  \brief Provides a widget for editing the properties of the aspects where only the name and comments need to be modified.
  Example objects are Folder, Workbook, etc.

  \ingroup kdefrontend
*/

AspectDock::AspectDock(QWidget* parent) : BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_leComment = ui.leComment;

	connect(ui.leName, &QLineEdit::textChanged, this, &AspectDock::nameChanged);
	connect(ui.leComment, &QLineEdit::textChanged, this, &AspectDock::commentChanged);
}

void AspectDock::setAspects(QList<AbstractAspect*> list) {
	m_aspect = list.first();

	const Lock lock(m_initializing);
	if (list.size() == 1) {
		ui.leName->setEnabled(true);
		ui.leComment->setEnabled(true);

		ui.leName->setText(m_aspect->name());
		ui.leComment->setText(m_aspect->comment());
	} else {
		ui.leName->setEnabled(false);
		ui.leComment->setEnabled(false);

		ui.leName->setText(QString());
		ui.leComment->setText(QString());
	}

	// slots
	connect(m_aspect, &AbstractColumn::aspectDescriptionChanged, this, &AspectDock::aspectDescriptionChanged);
}

//*************************************************************
//********* SLOTs for changes triggered in Column *************
//*************************************************************
void AspectDock::aspectDescriptionChanged(const AbstractAspect* aspect) {
	if (m_aspect != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != ui.leName->text())
		ui.leName->setText(aspect->name());
	else if (aspect->comment() != ui.leComment->text())
		ui.leComment->setText(aspect->comment());
	m_initializing = false;
}
