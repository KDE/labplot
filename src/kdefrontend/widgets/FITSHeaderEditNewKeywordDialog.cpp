/***************************************************************************
File                 : FITSHeaderEditNewKeywordDialog.cpp
Project              : LabPlot
Description          : Widget for adding new keyword in the FITS edit widget
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
#include "FITSHeaderEditNewKeywordDialog.h"
// #include <QDebug>
#include <KDialog>
#include <KLineEdit>
#include <KCompletion>
#include <KMessageBox>

#define FLEN_KEYWORD   75  /* max length of a keyword (HIERARCH convention) */
#define FLEN_VALUE     71  /* max length of a keyword value string */
#define FLEN_COMMENT   73  /* max length of a keyword comment string */

/*! \class FITSHeaderEditNewKeywordDialog
 * \brief Dialog class for adding new keywords to the FITSHeaderEditDialog's table.
 * \since 2.4.0
 * \ingroup widgets
 */
FITSHeaderEditNewKeywordDialog::FITSHeaderEditNewKeywordDialog(QWidget *parent) : KDialog(parent) {
	QWidget* mainWidget = new QWidget(this);
	ui.setupUi(mainWidget);
	setMainWidget(mainWidget);

	setWindowTitle(i18n("Specify the new keyword"));
	setWindowIcon(KIcon("document-new"));
	setButtons(KDialog::Ok | KDialog::Cancel);
	setButtonText(KDialog::Ok, i18n("&Add keyword"));

	KCompletion* keyCompletion = new KCompletion;
	keyCompletion->setItems(FITSFilter::standardKeywords());
	ui.kleKey->setCompletionObject(keyCompletion);
	ui.kleKey->setAutoDeleteCompletionObject(true);

	ui.kleKey->setPlaceholderText(i18n("Specify the name"));
	ui.kleValue->setPlaceholderText(i18n("Specify the value"));
	ui.kleComment->setPlaceholderText(i18n("Specify the comment"));

	ui.kleKey->setMaxLength(FLEN_KEYWORD);
	ui.kleValue->setMaxLength(FLEN_VALUE);
	ui.kleComment->setMaxLength(FLEN_COMMENT);
}

/*!
 * \brief Decides whether the keyword can be used, messagebox pops up if the keywords key is empty.
 * \return Whether the keyword was "Ok" or not.
 */
int FITSHeaderEditNewKeywordDialog::okClicked() {
	if (!ui.kleKey->text().isEmpty()) {
		m_newKeyword = FITSFilter::Keyword(ui.kleKey->text(), ui.kleValue->text(), ui.kleComment->text());
		return KDialog::Ok;
	} else {
		const int yesNo = KMessageBox::warningYesNo(this, i18n("Can't add new keyword without key, would you like to try again?"),
		                  i18n("Cannot add empty key"));
		if (yesNo == KMessageBox::No)
			return KDialog::Cancel;
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
 * \brief Overrides KDialog's slotButtonClicked slot.
 *        Decides whether the dialog should move in an accepted state or canceled.
 * \param button the code of the button clicked
 */
void FITSHeaderEditNewKeywordDialog::slotButtonClicked(int button) {
	if (button == KDialog::Ok) {
		int okClickedBtn = okClicked();
		if (okClickedBtn == KDialog::Ok)
			accept();
		else if (okClickedBtn == KDialog::Cancel)
			reject();
	} else
		KDialog::slotButtonClicked(button);
}
