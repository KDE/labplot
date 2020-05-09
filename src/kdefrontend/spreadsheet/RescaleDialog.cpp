/***************************************************************************
    File                 : RescaleDialog.h
    Project              : LabPlot
    Description          : Dialog to provide the rescale interval
    --------------------------------------------------------------------
	Copyright            : (C) 2020 by Alexander Semke (alexander.semke@web.de)

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
#include "RescaleDialog.h"
#include "backend/core/column/Column.h"

#include <QDoubleValidator>
#include <QPushButton>
#include <QWindow>

#include <KConfigGroup>
#include <KSharedConfig>
#include <KWindowConfig>

/*!
	\class RescaleDialog
	\brief Dialog to provide the rescale interval for the select columns in the spreadsheet.

	\ingroup kdefrontend
 */
RescaleDialog::RescaleDialog(QWidget* parent) : QDialog(parent) {
	setWindowIcon(QIcon::fromTheme("view-sort-ascending"));
	setWindowTitle(i18nc("@title:window", "Rescale Interval"));
	setSizeGripEnabled(true);

	ui.setupUi(this);

	ui.buttonBox->button(QDialogButtonBox::Ok)->setText(i18n("Rescale"));
	ui.lInterval->setText(i18n("Interval [a, b]:"));
	ui.lInfo->setText(i18n("More than one column selected. The same interval will be applied to <i>all</i> columns."));

	ui.leMin->setValidator(new QDoubleValidator(ui.leMin));
	ui.leMax->setValidator(new QDoubleValidator(ui.leMax));

	connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &RescaleDialog::reject);
	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &RescaleDialog::accept);
	connect(ui.leMin, &QLineEdit::textChanged, this, &RescaleDialog::validateOkButton);
	connect(ui.leMax, &QLineEdit::textChanged, this, &RescaleDialog::validateOkButton);

	//restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("RescaleDialog"));
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(300, 0).expandedTo(minimumSize()));

	double min = conf.readEntry(QLatin1String("Min"), 0.0);
	double max = conf.readEntry(QLatin1String("Max"), 1.0);
	QLocale locale;
// 	ui.leMin->setText(locale.toString(min, 'f'));
// 	ui.leMax->setText(locale.toString(max, 'f'));
	ui.leMin->setText(QString::number(min));
	ui.leMax->setText(QString::number(max));

	validateOkButton();
}

RescaleDialog::~RescaleDialog() {
	//save the current settings
	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("RescaleDialog"));
	KWindowConfig::saveWindowSize(windowHandle(), conf);

	// general settings
	conf.writeEntry(QLatin1String("Min"), ui.leMin->text().toDouble());
	conf.writeEntry(QLatin1String("Max"), ui.leMax->text().toDouble());
}

void RescaleDialog::setColumns(const QVector<Column*>& columns) {
	ui.lInfo->setVisible(columns.size() > 1);
}

double RescaleDialog::min() const {
	return ui.leMin->text().toDouble();
}

double RescaleDialog::max() const {
	return ui.leMax->text().toDouble();
}

void RescaleDialog::validateOkButton() {
	const QString& min = ui.leMin->text().simplified();
	const QString& max = ui.leMax->text().simplified();
	bool valid = !min.isEmpty() && !max.isEmpty() && min != max;
	ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(valid);
}
