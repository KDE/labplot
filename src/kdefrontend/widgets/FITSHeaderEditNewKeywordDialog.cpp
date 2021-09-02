/*
    File                 : FITSHeaderEditNewKeywordDialog.cpp
    Project              : LabPlot
    Description          : Widget for adding new keyword in the FITS edit widget
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016-2017 Fabian Kristof <fkristofszabolcs@gmail.com>
    SPDX-FileCopyrightText: 2016-2019 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "FITSHeaderEditNewKeywordDialog.h"

#include <QCompleter>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QWindow>

#include <KMessageBox>
#include <KSharedConfig>
#include <KWindowConfig>

#define FLEN_KEYWORD   75  /* max length of a keyword (HIERARCH convention) */
#define FLEN_VALUE     71  /* max length of a keyword value string */
#define FLEN_COMMENT   73  /* max length of a keyword comment string */

/*! \class FITSHeaderEditNewKeywordDialog
 * \brief Dialog class for adding new keywords to the FITSHeaderEditDialog's table.
 * \since 2.4.0
 * \ingroup widgets
 */
FITSHeaderEditNewKeywordDialog::FITSHeaderEditNewKeywordDialog(QWidget *parent) : QDialog(parent) {
	ui.setupUi(this);

	auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	ui.gridLayout->addWidget(btnBox, 3, 1, 1, 2);
	m_okButton = btnBox->button(QDialogButtonBox::Ok);
	m_cancelButton = btnBox->button(QDialogButtonBox::Cancel);

	m_okButton->setText(i18n("&Add Keyword"));

	connect(btnBox, &QDialogButtonBox::clicked, this, &FITSHeaderEditNewKeywordDialog::slotButtonClicked);

	setWindowTitle(i18nc("@title:window", "Specify the New Keyword"));
	setWindowIcon(QIcon::fromTheme("document-new"));

	QCompleter* keyCompleter = new QCompleter(FITSFilter::standardKeywords(), this);
	keyCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	ui.leKey->setCompleter(keyCompleter);

	ui.leKey->setPlaceholderText(i18n("Specify the name"));
	ui.leValue->setPlaceholderText(i18n("Specify the value"));
	ui.leComment->setPlaceholderText(i18n("Specify the comment"));

	ui.leKey->setMaxLength(FLEN_KEYWORD);
	ui.leValue->setMaxLength(FLEN_VALUE);
	ui.leComment->setMaxLength(FLEN_COMMENT);


	//restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf(KSharedConfig::openConfig(), "FITSHeaderEditNewKeywordDialog");
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(300, 0).expandedTo(minimumSize()));
}

FITSHeaderEditNewKeywordDialog::~FITSHeaderEditNewKeywordDialog() {
	KConfigGroup conf(KSharedConfig::openConfig(), "FITSHeaderEditNewKeywordDialog");
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

/*!
 * \brief Decides whether the keyword can be used, messagebox pops up if the keywords key is empty.
 * \return Whether the keyword was "Ok" or not.
 */
int FITSHeaderEditNewKeywordDialog::okClicked() {
	if (!ui.leKey->text().isEmpty()) {
		m_newKeyword = FITSFilter::Keyword(ui.leKey->text(), ui.leValue->text(), ui.leComment->text());
		return QMessageBox::Ok;
	} else {
		const int yesNo = KMessageBox::warningYesNo(this, i18n("Cannot add new keyword without key, would you like to try again?"),
		                  i18n("Cannot add empty key"));
		if (yesNo == KMessageBox::No)
			return QMessageBox::Cancel;
		return yesNo;
	}
}

/*!
 * \brief Returns the new keyword.
 * \return The newly constructed keyword from the line edits.
 */
FITSFilter::Keyword FITSHeaderEditNewKeywordDialog::newKeyword() const {
	return m_newKeyword;
}

/*!
 * \brief Decides whether the dialog should move in an accepted state or canceled.
 * \param button the button clicked
 */
void FITSHeaderEditNewKeywordDialog::slotButtonClicked(QAbstractButton* button) {
	if (button == m_okButton) {
		int okClickedBtn = okClicked();
		if (okClickedBtn == QMessageBox::Ok)
			accept();
		else if (okClickedBtn == QMessageBox::Cancel)
			reject();
	} else if (button == m_cancelButton)
		reject();
}
