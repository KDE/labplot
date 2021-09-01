/*
File                 : FITSHeaderEditDialog.h
Project              : LabPlot
Description          : Dialog for listing/editing FITS header keywords
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2016-2017 Fabian Kristof (fkristofszabolcs@gmail.com)
SPDX-FileCopyrightText: 2016-2019 Alexander Semke (alexander.semke@web.de)
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "FITSHeaderEditDialog.h"


#include <QDialogButtonBox>
#include <QPushButton>
#include <QWindow>
#include <QVBoxLayout>

#include <KSharedConfig>
#include <KWindowConfig>

/*! \class FITSHeaderEditDialog
 * \brief Dialog class for editing FITS header units.
 * \since 2.4.0
 * \ingroup widgets
 */
FITSHeaderEditDialog::FITSHeaderEditDialog(QWidget* parent) : QDialog(parent),
	m_headerEditWidget(new FITSHeaderEditWidget(this)) {

	auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	auto* layout = new QVBoxLayout;

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
	create(); // ensure there's a window created
	KConfigGroup conf(KSharedConfig::openConfig(), "FITSHeaderEditDialog");
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(300, 0).expandedTo(minimumSize()));
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
