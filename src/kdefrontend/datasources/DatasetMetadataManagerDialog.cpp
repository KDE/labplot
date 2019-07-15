/***************************************************************************
File                 : DatasetMetadataManagerDialog.cpp
Project              : LabPlot
Description          : Dialog for managing a metadata file of a dataset
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

#include "src/kdefrontend/datasources/DatasetMetadataManagerDialog.h"
#include "src/kdefrontend/datasources/DatasetMetadataManagerWidget.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KWindowConfig>
#include <QDialogButtonBox>
#include <QWindow>
#include <QPushButton>

/*!
	\class DatasetMetadataManagerDialog
	\brief Dialog for adding a new dataset to LabPlot's current collection. Embeds \c DatasetMetadataManagerWidget and provides the standard buttons.

	\ingroup kdefrontend
 */
DatasetMetadataManagerDialog::DatasetMetadataManagerDialog(QWidget* parent, const QMap< QString, QMap<QString, QMap<QString, QVector<QString>>>>& datasetMap) : QDialog(parent),
	m_mainWidget(new DatasetMetadataManagerWidget(this, datasetMap)),
	m_buttonBox(nullptr),
	m_okButton(nullptr) {
	connect(m_mainWidget, &DatasetMetadataManagerWidget::checkOk, this, &DatasetMetadataManagerDialog::checkOkButton);

	setWindowTitle(i18nc("@title:window", "Dataset metadata manager"));

	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	m_okButton = m_buttonBox->button(QDialogButtonBox::Ok);
	m_okButton->setEnabled(false);

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

	checkOkButton();
}

DatasetMetadataManagerDialog::~DatasetMetadataManagerDialog() {
	//save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "DatasetMetadataManagerDialog");
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

/**
 * @brief Checks whether the OK button of the dialog can be pressed or not
 */
void DatasetMetadataManagerDialog::checkOkButton() {
	bool enable = m_mainWidget->checkDataValidity();
	m_okButton->setEnabled(enable);
}

/**
 * @brief Triggers updating the metadata file containing the categories, subcategories and datasets.
 * @param fileName the name of the metadata file (path)
 */
void DatasetMetadataManagerDialog::updateDocument(const QString& fileName) {
	m_mainWidget->updateDocument(fileName);
}

/**
 * @brief returns the path to the new metadata file of the new dataset.
 */
QString DatasetMetadataManagerDialog::getMetadataFilePath() const {
	return m_mainWidget->getMetadataFilePath();
}
