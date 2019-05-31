/***************************************************************************
File                 : DatasetMetadataManagerDialog.cpp
Project              : LabPlot
Description          : dialog for managing a metadata file of a dataset
--------------------------------------------------------------------
Copyright            : (C) 2019 Ferencz Kovacs (kferike98@gmail.com)

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

#include "DatasetMetadataManagerDialog.h"
#include "DatasetMetadataManagerWidget.h"
#include "KLocalizedString"
#include "QDialogButtonBox"
#include "KConfigGroup"
#include "KSharedConfig"
#include "KWindowConfig"
#include "QWindow"
#include "QPushButton"

DatasetMetadataManagerDialog::DatasetMetadataManagerDialog(QWidget* parent, const QMap<QString, QMap<QString, QVector<QString>>>& datasetMap) : QDialog(parent),
	m_mainWidget(new DatasetMetadataManagerWidget(this, datasetMap))
{
	connect(m_mainWidget, &DatasetMetadataManagerWidget::checkOk, this, &DatasetMetadataManagerDialog::checkOkButton);

	setWindowTitle(i18nc("@title:window", "Dataset metadata manager"));

	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	okButton = m_buttonBox->button(QDialogButtonBox::Ok);
	okButton->setEnabled(false);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(m_mainWidget);
	layout->addWidget(m_buttonBox);

	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	//restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf(KSharedConfig::openConfig(), "DatasetMetadataManagerDialog");
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(0, 0).expandedTo(minimumSize()));
}

DatasetMetadataManagerDialog::~DatasetMetadataManagerDialog() {
	//save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "DatasetMetadataManagerDialog");
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void DatasetMetadataManagerDialog::checkOkButton() {
	bool enable = m_mainWidget->checkDataValidity();
	okButton->setEnabled(enable);
}

void DatasetMetadataManagerDialog::updateDocument(const QString &fileName) {
	m_mainWidget->updateDocument(fileName);
}

void DatasetMetadataManagerDialog::createNewMetadata(const QString& dirPath) {
	m_mainWidget->createNewMetadata(dirPath);
}
