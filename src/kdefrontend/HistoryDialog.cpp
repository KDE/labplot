/***************************************************************************
    File                 : HistoryDialog.cpp
    Project              : LabPlot
    Description          : history dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2012-2013 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de

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
#include "HistoryDialog.h"
#include <kmessagebox.h>
#include <klocale.h>
#include <QUndoStack>
#include <QUndoView>

/*!
	\class HistoryDialog
	\brief Display the content of the project's undo stack.

	\ingroup kdefrontend
 */
HistoryDialog::HistoryDialog(QWidget* parent, QUndoStack* stack, QString& emptyLabel) : KDialog(parent){
	m_undoStack = stack;
	QUndoView* undoView = new QUndoView(stack, this);
	undoView->setCleanIcon( QIcon::fromTheme("edit-clear-history") );
	undoView->setEmptyLabel(emptyLabel);
	undoView->setMinimumWidth(350);
	undoView->setWhatsThis(i18n("List of all performed steps/actions.\n"
			"Select an item in the list to navigate to the corresponding step."));
	setMainWidget(undoView);

	setWindowIcon( QIcon::fromTheme("view-history") );
	setWindowTitle(i18n("Undo/Redo History"));
	showButtonSeparator(true);
    setAttribute(Qt::WA_DeleteOnClose);
	if (stack->count()) {
		setButtons( KDialog::Ok | KDialog::User1 | KDialog::Cancel );
		setButtonToolTip(KDialog::User1, i18n("Clears the undo history. Commands are not undone or redone; the state of the project remains unchanged."));
		setButtonIcon(KDialog::User1, QIcon::fromTheme("edit-clear"));
		setButtonText(KDialog::User1, i18n("Clear"));
		connect(this,SIGNAL(user1Clicked()), this, SLOT(clearUndoStack()));
	}else{
		setButtons( KDialog::Ok | KDialog::Cancel );
	}
}

void HistoryDialog::clearUndoStack(){
	if (KMessageBox::questionYesNo( this,
									i18n("Do you really want to clear the undo history?"),
									i18n("Clear history")
								  ) == KMessageBox::Yes)
		m_undoStack->clear();
}
