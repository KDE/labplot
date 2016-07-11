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
#include <QDebug>
FITSHeaderEditNewKeywordDialog::FITSHeaderEditNewKeywordDialog(QWidget *parent) :
    KDialog(parent) {
    QWidget* mainWidget = new QWidget(this);
    ui.setupUi(mainWidget);
    setMainWidget( mainWidget );
    setWindowTitle(i18n("Specify the new keyword"));
    setButtons( KDialog::Ok | KDialog::Cancel );
    setButtonText(KDialog::Ok, i18n("&Add keyword"));

    KCompletion* keyCompletion = new KCompletion;
    keyCompletion->setItems(FITSFilter::standardKeywords());
    ui.kleKey->setCompletionObject(keyCompletion);
    ui.kleKey->setAutoDeleteCompletionObject(true);
}

FITSHeaderEditNewKeywordDialog::~FITSHeaderEditNewKeywordDialog() {
}

int FITSHeaderEditNewKeywordDialog::okClicked() {
    if (!ui.kleKey->text().isEmpty()) {
        m_newKeyword.key = ui.kleKey->text();
        m_newKeyword.value = ui.kleValue->text();
        m_newKeyword.comment = ui.kleComment->text();
        return KDialog::Ok;
    } else {
        int yesNo = KMessageBox::warningYesNo(this, i18n("Can't add new keyword without key, would you like to try again?"),
                                  i18n("Cannot add empty key"));
        if (yesNo == KMessageBox::No) {
            return KDialog::Cancel;
        }
        return yesNo;
    }
}

FITSFilter::Keyword FITSHeaderEditNewKeywordDialog::newKeyword() const {
    return m_newKeyword;
}

void FITSHeaderEditNewKeywordDialog::slotButtonClicked(int button) {
    if (button == KDialog::Ok)
    {
        int okClickedBtn = okClicked();
        if (okClickedBtn == KDialog::Ok) {
            accept();
        } else if (okClickedBtn == KDialog::Cancel) {
            reject();
        }
    }
    else {
        KDialog::slotButtonClicked(button);
    }
}
