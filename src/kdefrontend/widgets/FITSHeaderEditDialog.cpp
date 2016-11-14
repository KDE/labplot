/***************************************************************************
File                 : FITSHeaderEditDialog.h
Project              : LabPlot
Description          : Dialog for listing/editing FITS header keywords
--------------------------------------------------------------------
Copyright            : (C) 2016 by Fabian Kristof (fkristofszabolcs@gmail.com)
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

#include "FITSHeaderEditDialog.h"
#include <KSharedConfig>

/*! \class FITSHeaderEditDialog
 * \brief Dialog class for editing FITS header units.
 * \since 2.4.0
 * \ingroup widgets
 */
FITSHeaderEditDialog::FITSHeaderEditDialog(QWidget* parent) : KDialog(parent), m_saved(false) {
	m_HeaderEditWidget = new FITSHeaderEditWidget(this);
	setMainWidget(m_HeaderEditWidget);

	setWindowTitle(i18n("FITS header editor"));
	setButtons( KDialog::Ok | KDialog::Cancel );
	setButtonText(KDialog::Ok, i18n("&Save"));
	connect(this, SIGNAL(okClicked()), this, SLOT(save()));
	setAttribute(Qt::WA_DeleteOnClose);

	//restore saved settings if available
	KConfigGroup conf(KSharedConfig::openConfig(), "FITSHeaderEditDialog");
	if (conf.exists())
		restoreDialogSize(conf);
	else
		resize( QSize(400,0).expandedTo(minimumSize()) );
}

/*!
 * \brief FITSHeaderEditDialog::~FITSHeaderEditDialog
 */
FITSHeaderEditDialog::~FITSHeaderEditDialog() {
	KConfigGroup conf(KSharedConfig::openConfig(), "FITSHeaderEditDialog");
	saveDialogSize(conf);
	delete m_HeaderEditWidget;
}

/*!
 * \brief This slot is triggered when the Save button was clicked in the ui.
 */
void FITSHeaderEditDialog::save() {
	m_saved = m_HeaderEditWidget->save();
}

/*!
 * \brief Returns whether there were changes saved.
 * \return
 */
bool FITSHeaderEditDialog::saved() const {
	return m_saved;
}
