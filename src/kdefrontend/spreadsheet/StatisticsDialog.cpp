/***************************************************************************
    File                 : StatisticsDialog.cpp
    Project              : LabPlot
    Description          : Dialog showing statistics for column values
    --------------------------------------------------------------------
    Copyright            : (C) 2016-2017 by Fabian Kristof (fkristofszabolcs@gmail.com))
	Copyright            : (C) 2016-2021 by Alexander Semke (alexander.semke@web.de)

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

#include "StatisticsDialog.h"
#include "StatisticsColumnWidget.h"
#include "backend/core/column/Column.h"
#include "backend/lib/macros.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QWindow>

#include <KLocalizedString>
#include <KWindowConfig>
#include <KSharedConfig>

#include <cmath>

StatisticsDialog::StatisticsDialog(const QString& title, const QVector<Column*>& columns, QWidget* parent) : QDialog(parent),
	m_twStatistics(new QTabWidget) {

	auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok);

	QPushButton* btnOk = btnBox->button(QDialogButtonBox::Ok);
	btnOk->setFocus();

	connect(btnOk, &QPushButton::clicked, this, &StatisticsDialog::close);
	connect(btnBox, &QDialogButtonBox::accepted, this, &StatisticsDialog::accept);

	auto* layout = new QVBoxLayout;
	layout->addWidget(m_twStatistics);
	layout->addWidget(btnBox);

	setLayout(layout);

	setWindowTitle(title);
	setWindowIcon(QIcon::fromTheme("view-statistics"));
	setAttribute(Qt::WA_DeleteOnClose);

	m_columns = columns;

	//create tab widgets for every column and show the initial text with the placeholders
	for (auto* col : m_columns)
		m_twStatistics->addTab(new StatisticsColumnWidget(col, this), col->name());

	connect(m_twStatistics, &QTabWidget::currentChanged, this, &StatisticsDialog::currentTabChanged);

	//restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf(KSharedConfig::openConfig(), "StatisticsDialog");
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(490, 520));
}

StatisticsDialog::~StatisticsDialog() {
	KConfigGroup conf(KSharedConfig::openConfig(), "StatisticsDialog");
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void StatisticsDialog::showStatistics() {
	QApplication::processEvents(QEventLoop::AllEvents, 0);
	QTimer::singleShot(0, this, [=] () {currentTabChanged(0);});
}

void StatisticsDialog::currentTabChanged(int) {
	auto* const w = static_cast<StatisticsColumnWidget*>(m_twStatistics->currentWidget());
	if (!w)
		return;

	w->showStatistics();
}
