/*
	File                 : HistoryDialog.cpp
	Project              : LabPlot
	Description          : history dialog
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2012-2019 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "HistoryDialog.h"
#include "backend/core/Settings.h"

#include <KLocalizedString>
#include <KMessageBox>

#include <KWindowConfig>
#include <kcoreaddons_version.h>

#include <QDialogButtonBox>
#include <QPushButton>
#include <QUndoStack>
#include <QUndoView>
#include <QVBoxLayout>
#include <QWindow>

/*!
	\class HistoryDialog
	\brief Display the content of project's undo stack.

	\ingroup kdefrontend
 */
HistoryDialog::HistoryDialog(QWidget* parent, QUndoStack* stack, const QString& emptyLabel)
	: QDialog(parent)
	, m_undoStack(stack) {
	auto* undoView = new QUndoView(stack, this);
	undoView->setCleanIcon(QIcon::fromTheme(QLatin1String("edit-clear-history")));
	undoView->setEmptyLabel(emptyLabel);
	undoView->setMinimumWidth(350);
	undoView->setWhatsThis(
		i18n("List of all performed steps/actions.\n"
			 "Select an item in the list to navigate to the corresponding step."));

	setWindowIcon(QIcon::fromTheme(QLatin1String("view-history")));
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
		m_clearUndoStackButton->setIcon(QIcon::fromTheme(QLatin1String("edit-clear")));
		connect(m_clearUndoStackButton, &QPushButton::clicked, this, &HistoryDialog::clearUndoStack);
	}

	auto* line = new QFrame;
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);

	auto* layout = new QVBoxLayout;

	layout->addWidget(undoView);
	layout->addWidget(line);
	layout->addWidget(btnBox);

	setLayout(layout);

	// restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf = Settings::group(QStringLiteral("HistoryDialog"));
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(500, 300).expandedTo(minimumSize()));
}

HistoryDialog::~HistoryDialog() {
	// save dialog size
	KConfigGroup conf = Settings::group(QStringLiteral("HistoryDialog"));
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void HistoryDialog::clearUndoStack() {
	if (KMessageBox::questionTwoActions(this,
										i18n("Do you really want to clear the undo history?"),
										i18n("Clear History"),
										KStandardGuiItem::clear(),
										KStandardGuiItem::cancel())
		== KMessageBox::PrimaryAction)
		m_undoStack->clear();
}
