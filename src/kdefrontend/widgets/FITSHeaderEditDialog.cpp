/***************************************************************************
File                 : FITSHeaderEditDialog.h
Project              : LabPlot
Description          : Dialog for listing/editing FITS header keywords
--------------------------------------------------------------------
Copyright            : (C) 2016-2017 by Fabian Kristof (fkristofszabolcs@gmail.com)
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
#include <KWindowConfig>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

/*! \class FITSHeaderEditDialog
 * \brief Dialog class for editing FITS header units.
 * \since 2.4.0
 * \ingroup widgets
 */
FITSHeaderEditDialog::FITSHeaderEditDialog(QWidget* parent) : QDialog(parent), m_saved(false) {
	m_headerEditWidget = new FITSHeaderEditWidget(this);

	QDialogButtonBox* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	QVBoxLayout* layout = new QVBoxLayout;

	layout->addWidget(m_headerEditWidget);
	layout->addWidget(btnBox);

	setLayout(layout);

	m_okButton = btnBox->button(QDialogButtonBox::Ok);
	m_okButton->setText(i18n("&Save"));
	m_okButton->setEnabled(false);

	connect(btnBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &FITSHeaderEditDialog::reject);
	connect(btnBox, &QDialogButtonBox::accepted, this, &FITSHeaderEditDialog::accept);
	connect(btnBox, &QDialogButtonBox::rejected, this, &FITSHeaderEditDialog::reject);

	setWindowTitle(i18nc("@title:window", "FITS Metadata Editor"));
	setWindowIcon(QIcon::fromTheme("document-edit"));

	connect(m_okButton, &QPushButton::clicked, this, &FITSHeaderEditDialog::save);
	connect(m_headerEditWidget, &FITSHeaderEditWidget::changed, this, &FITSHeaderEditDialog::headersChanged);

	setAttribute(Qt::WA_DeleteOnClose);

	//restore saved settings if available
	KConfigGroup conf(KSharedConfig::openConfig(), "FITSHeaderEditDialog");
	if (conf.exists())
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
	else
		resize( QSize(400,0).expandedTo(minimumSize()) );
}

/*!
 * \brief FITSHeaderEditDialog::~FITSHeaderEditDialog
 */
FITSHeaderEditDialog::~FITSHeaderEditDialog() {
	KConfigGroup conf(KSharedConfig::openConfig(), "FITSHeaderEditDialog");
	KWindowConfig::saveWindowSize(windowHandle(), conf);
	delete m_headerEditWidget;
}

void FITSHeaderEditDialog::headersChanged(bool changed) {
	if (changed) {
		setWindowTitle(i18nc("@title:window", "FITS Metadata Editor  [Changed]"));
		m_okButton->setEnabled(true);
	} else {
		setWindowTitle(i18nc("@title:window", "FITS Metadata Editor"));
		m_okButton->setEnabled(false);
	}
}

/*!
 * \brief This slot is triggered when the Save button was clicked in the ui.
 */
void FITSHeaderEditDialog::save() {
	m_saved = m_headerEditWidget->save();
}

/*!
 * \brief Returns whether there were changes saved.
 * \return
 */
bool FITSHeaderEditDialog::saved() const {
	return m_saved;
}
