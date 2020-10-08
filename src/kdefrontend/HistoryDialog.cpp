/***************************************************************************
    File                 : HistoryDialog.cpp
    Project              : LabPlot
    Description          : history dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2012-2019 by Alexander Semke (alexander.semke@web.de)

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

#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWindow>
#include <QUndoStack>
#include <QUndoView>

#include <KMessageBox>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KWindowConfig>

/*!
	\class HistoryDialog
	\brief Display the content of project's undo stack.

	\ingroup kdefrontend
 */
HistoryDialog::HistoryDialog(QWidget* parent, QUndoStack* stack, const QString& emptyLabel) : QDialog(parent),
	m_undoStack(stack) {
	auto* undoView = new QUndoView(stack, this);
	undoView->setCleanIcon( QIcon::fromTheme("edit-clear-history") );
	undoView->setEmptyLabel(emptyLabel);
	undoView->setMinimumWidth(350);
	undoView->setWhatsThis(i18n("List of all performed steps/actions.\n"
	                            "Select an item in the list to navigate to the corresponding step."));

	setWindowIcon( QIcon::fromTheme("view-history") );
	setWindowTitle(i18nc("@title:window", "Undo/Redo History"));
	setAttribute(Qt::WA_DeleteOnClose);
	auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	m_okButton = btnBox->button(QDialogButtonBox::Ok);

	connect(btnBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &HistoryDialog::close);
	connect(btnBox, &QDialogButtonBox::accepted, this, &HistoryDialog::accept);
	connect(btnBox, &QDialogButtonBox::rejected, this, &HistoryDialog::reject);

	if (stack->count()) {
		m_clearUndoStackButton = new QPushButton;
		btnBox->addButton(m_clearUndoStackButton, QDialogButtonBox::ActionRole);
		m_clearUndoStackButton->setText(i18n("&Clear"));
		m_clearUndoStackButton->setToolTip(i18n("Clears the undo history. Commands are not undone or redone; the state of the project remains unchanged."));
		m_clearUndoStackButton->setIcon(QIcon::fromTheme("edit-clear"));
		connect(m_clearUndoStackButton, &QPushButton::clicked, this, &HistoryDialog::clearUndoStack);
	}

	QFrame* line = new QFrame;
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);

	auto* layout = new QVBoxLayout;

	layout->addWidget(undoView);
	layout->addWidget(line);
	layout->addWidget(btnBox);

	setLayout(layout);

	//restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf(KSharedConfig::openConfig(), "HistoryDialog");
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(500, 300).expandedTo(minimumSize()));
}

HistoryDialog::~HistoryDialog() {
	//save dialog size
	KConfigGroup conf(KSharedConfig::openConfig(), "HistoryDialog");
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void HistoryDialog::clearUndoStack() {
	if (KMessageBox::questionYesNo( this,
	                                i18n("Do you really want to clear the undo history?"),
	                                i18n("Clear History")
	                              ) == KMessageBox::Yes)
		m_undoStack->clear();
}
