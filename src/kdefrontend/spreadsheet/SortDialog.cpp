/*
	File                 : SortDialog.h
	Project              : LabPlot
	Description          : Sorting options dialog
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SortDialog.h"
#include "backend/core/Settings.h"
#include "backend/core/column/Column.h"

#include <QPushButton>
#include <QWindow>

#include <KConfigGroup>
#include <KWindowConfig>

/*!
	\class SortDialog
	\brief Dialog for sorting the columns in a spreadsheet.

	\ingroup kdefrontend
 */
SortDialog::SortDialog(QWidget* parent, bool sortAll)
	: QDialog(parent) {
	setWindowIcon(QIcon::fromTheme(QStringLiteral("view-sort-ascending")));
	setSizeGripEnabled(true);
	setAttribute(Qt::WA_DeleteOnClose);

	if (sortAll)
		setWindowTitle(i18nc("@title:window", "Sort All Columns"));
	else
		setWindowTitle(i18nc("@title:window", "Sort Selected Columns"));

	ui.setupUi(this);

	ui.buttonBox->button(QDialogButtonBox::Ok)->setText(i18n("Sort"));

	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &SortDialog::sortColumns);
	connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &SortDialog::reject);
	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &SortDialog::accept);

	// restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf = Settings::group(QLatin1String("SortDialog"));
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(300, 0).expandedTo(minimumSize()));

	ui.cbOrdering->setCurrentIndex(conf.readEntry(QLatin1String("Ordering"), 0));
}

SortDialog::~SortDialog() {
	// save the current settings
	KConfigGroup conf = Settings::group(QLatin1String("SortDialog"));
	KWindowConfig::saveWindowSize(windowHandle(), conf);

	// general settings
	conf.writeEntry(QLatin1String("Ordering"), ui.cbOrdering->currentIndex());
}

void SortDialog::sortColumns() {
	Column* leading = m_columns.at(ui.cbColumns->currentIndex());
	Q_EMIT sort(leading, m_columns, ui.cbOrdering->currentIndex() == Qt::AscendingOrder);
}

void SortDialog::setColumns(const QVector<Column*>& columns, const Column* leadingColumn) {
	m_columns = columns;

	int index = 0;
	int leadingColumnIndex = 0;
	for (auto* col : m_columns) {
		ui.cbColumns->addItem(col->name());
		if (leadingColumn && col == leadingColumn)
			leadingColumnIndex = index;

		++index;
	}

	ui.cbColumns->setCurrentIndex(leadingColumnIndex);

	if (m_columns.size() == 1) {
		ui.lColumns->hide();
		ui.cbColumns->hide();
	}
}
