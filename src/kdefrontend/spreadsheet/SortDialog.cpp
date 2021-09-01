/*
    File                 : SortDialog.h
    Project              : LabPlot
    Description          : Sorting options dialog
    --------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2020 Alexander Semke (alexander.semke@web.de)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/
#include "SortDialog.h"
#include "backend/core/column/Column.h"

#include <QPushButton>
#include <QWindow>

#include <KConfigGroup>
#include <KSharedConfig>
#include <KWindowConfig>

/*!
	\class SortDialog
	\brief Dialog for sorting the columns in a spreadsheet.

	\ingroup kdefrontend
 */
SortDialog::SortDialog(QWidget* parent) : QDialog(parent) {
	setWindowIcon(QIcon::fromTheme("view-sort-ascending"));
	setWindowTitle(i18nc("@title:window", "Sort Columns"));
	setSizeGripEnabled(true);
	setAttribute(Qt::WA_DeleteOnClose);

	ui.setupUi(this);

	ui.buttonBox->button(QDialogButtonBox::Ok)->setText(i18n("Sort"));

	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &SortDialog::sortColumns);
	connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &SortDialog::reject);
	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &SortDialog::accept);
	connect(ui.cbSorting, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged),
			this, &SortDialog::changeType);

	//restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("SortDialog"));
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(300, 0).expandedTo(minimumSize()));

	ui.cbOrdering->setCurrentIndex(conf.readEntry(QLatin1String("Ordering"), 0));
	ui.cbSorting->setCurrentIndex(conf.readEntry(QLatin1String("Sorting"), 0));
	changeType(ui.cbSorting->currentIndex());
}

SortDialog::~SortDialog() {
	//save the current settings
	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("SortDialog"));
	KWindowConfig::saveWindowSize(windowHandle(), conf);

	// general settings
	conf.writeEntry(QLatin1String("Ordering"), ui.cbOrdering->currentIndex());
	conf.writeEntry(QLatin1String("Sorting"), ui.cbSorting->currentIndex());
}

void SortDialog::sortColumns() {
	Column* leading{nullptr};
	if (ui.cbSorting->currentIndex() == Together)
		leading = m_columns.at(ui.cbColumns->currentIndex());

	emit sort(leading, m_columns, ui.cbOrdering->currentIndex() == Qt::AscendingOrder);
}

void SortDialog::setColumns(const QVector<Column*>& columns) {
	m_columns = columns;

	for (auto* col : m_columns)
		ui.cbColumns->addItem(col->name());

	ui.cbColumns->setCurrentIndex(0);

	if (m_columns.size() == 1) {
		ui.lSorting->hide();
		ui.cbSorting->hide();
		ui.lColumns->hide();
		ui.cbColumns->hide();
	}
}

void SortDialog::changeType(int Type) {
	if (Type == Together)
		ui.cbColumns->setEnabled(true);
	else
		ui.cbColumns->setEnabled(false);
}
