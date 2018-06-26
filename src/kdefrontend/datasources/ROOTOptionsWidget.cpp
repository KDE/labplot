/***************************************************************************
File                 : ROOTOptionsWidget.cpp
Project              : LabPlot
Description          : widget providing options for the import of ROOT data
--------------------------------------------------------------------
Copyright            : (C) 2018 Christoph Roick (chrisito@gmx.de)
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

#include "ROOTOptionsWidget.h"

#include "ImportFileWidget.h"

#include "backend/datasources/filters/ROOTFilter.h"
#include "backend/lib/macros.h"

ROOTOptionsWidget::ROOTOptionsWidget(QWidget* parent, ImportFileWidget* fileWidget) : QWidget(parent), m_fileWidget(fileWidget) {
	ui.setupUi(parent);

	connect(ui.lwContent, &QListWidget::itemSelectionChanged, this, &ROOTOptionsWidget::rootListWidgetSelectionChanged);
	connect(ui.bRefreshPreview, &QPushButton::clicked, fileWidget, &ImportFileWidget::refreshPreview);
}

void ROOTOptionsWidget::clear() {
	ui.lwContent->clear();
	ui.twPreview->clearContents();
}

void ROOTOptionsWidget::updateContent(ROOTFilter *filter, QString fileName) {
	DEBUG("updateContent()");
	ui.lwContent->clear();
	ui.lwContent->addItems(filter->listHistograms(fileName));
}

void ROOTOptionsWidget::rootListWidgetSelectionChanged() {
	DEBUG("rootListWidgetSelectionChanged()");
	auto items = ui.lwContent->selectedItems();
	QDEBUG("SELECTED ITEMS =" << items);

	if (items.isEmpty())
		return;

	m_fileWidget->refreshPreview();
}

const QStringList ROOTOptionsWidget::selectedROOTNames() const {
	QStringList names;

	for (const QListWidgetItem* const item : ui.lwContent->selectedItems())
		names << item->text();

	return names;
}

int ROOTOptionsWidget::columns() const {
	int cols = ui.checkCenter->isChecked() ? ROOTFilter::Center : 0;
	cols |= ui.checkLow->isChecked() ? ROOTFilter::Low : 0;
	cols |= ui.checkContent->isChecked() ? ROOTFilter::Content : 0;
	cols |= ui.checkError->isChecked() ? ROOTFilter::Error : 0;

	return cols;
}

void ROOTOptionsWidget::setNBins(int nbins) {
	// try to retain the range settings:
	// - if nbins was not 0, keep start bin,
	//   else set it to one after underflow
	// - if nbins didn't change, keep end bin,
	//   else set it to one before overflow
	const int max = qMax(nbins - 1, 0);
	int firstval = ui.sbFirst->value();
	if (ui.sbFirst->maximum() == 0)
		firstval = qMin(nbins - 1, 1);
	ui.sbFirst->setMaximum(max);
	ui.sbFirst->setValue(firstval);

	int lastval = max == ui.sbLast->maximum() ? ui.sbLast->value() : qMax(max - 1, 0);
	ui.sbLast->setMaximum(max);
	ui.sbLast->setValue(lastval);
}
