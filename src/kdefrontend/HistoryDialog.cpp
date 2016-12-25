/***************************************************************************
    File                 : HistoryDialog.cpp
    Project              : LabPlot
    Description          : history dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2012-2016 by Alexander Semke (alexander.semke@web.de)

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
	\brief Display the content of project's undo stack.

	\ingroup kdefrontend
 */
HistoryDialog::HistoryDialog(QWidget* parent, QUndoStack* stack, const QString& emptyLabel) : KDialog(parent), m_undoStack(stack) {
	QUndoView* undoView = new QUndoView(stack, this);
	undoView->setCleanIcon( KIcon("edit-clear-history") );
	undoView->setEmptyLabel(emptyLabel);
	undoView->setWhatsThis(i18n("List of all performed steps/actions.\n"
	                            "Select an item in the list to navigate to the corresponding step."));
	setMainWidget(undoView);

	setWindowIcon( KIcon("view-history") );
	setWindowTitle(i18n("Undo/Redo History"));
	showButtonSeparator(true);
	setAttribute(Qt::WA_DeleteOnClose);

	if (stack->count()) {
		setButtons( KDialog::Ok | KDialog::User1 | KDialog::Cancel );
		setButtonToolTip(KDialog::User1, i18n("Clears the undo history. Commands are not undone or redone; the state of the project remains unchanged."));
		setButtonIcon(KDialog::User1, KIcon("edit-clear"));
		setButtonText(KDialog::User1, i18n("Clear"));
		connect(this,SIGNAL(user1Clicked()), this, SLOT(clearUndoStack()));
	} else
		setButtons( KDialog::Ok | KDialog::Cancel );

	//restore saved dialog size if available
	KConfigGroup conf(KSharedConfig::openConfig(), "HistoryDialog");
	if (conf.exists())
		restoreDialogSize(conf);
	else
		resize( QSize(500, 300).expandedTo(minimumSize()) );
}

HistoryDialog::~HistoryDialog() {
	//save dialog size
	KConfigGroup conf(KSharedConfig::openConfig(), "HistoryDialog");
	saveDialogSize(conf);
}

void HistoryDialog::clearUndoStack() {
	if (KMessageBox::questionYesNo( this,
	                                i18n("Do you really want to clear the undo history?"),
	                                i18n("Clear history")
	                              ) == KMessageBox::Yes)
		m_undoStack->clear();
}
