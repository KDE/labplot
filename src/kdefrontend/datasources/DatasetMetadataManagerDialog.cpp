/*
    File                 : DatasetMetadataManagerDialog.cpp
    Project              : LabPlot
    Description          : Dialog for managing a metadata file of a dataset
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Ferencz Kovacs <kferike98@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

	auto* layout = new QVBoxLayout(this);
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
	} else {
		resize(QSize(0, 0).expandedTo(minimumSize()));
	}

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

/**
 * @brief Sets the collection name in the DatasetMetadataManagerWidget
 */
void DatasetMetadataManagerDialog::setCollection(const QString& collection) {
	m_mainWidget->setCollection(collection);
}

/**
 * @brief Sets the category name in the DatasetMetadataManagerWidget
 */
void DatasetMetadataManagerDialog::setCategory(const QString& category) {
	m_mainWidget->setCategory(category);
}

/**
 * @brief Sets the subcategory name in the DatasetMetadataManagerWidget
 */
void DatasetMetadataManagerDialog::setSubcategory(const QString& subcategory) {
	m_mainWidget->setSubcategory(subcategory);
}

/**
 * @brief Sets the short name of the dataset in the DatasetMetadataManagerWidget
 */
void DatasetMetadataManagerDialog::setShortName(const QString& name) {
	m_mainWidget->setShortName(name);
}

/**
 * @brief Sets the full name of the dataset in the DatasetMetadataManagerWidget
 */
void DatasetMetadataManagerDialog::setFullName(const QString& name) {
	m_mainWidget->setFullName(name);
}

/**
 * @brief Sets the text of the description in the DatasetMetadataManagerWidget
 */
void DatasetMetadataManagerDialog::setDescription(const QString& description) {
	m_mainWidget->setDescription(description);
}

/**
 * @brief Sets the download url in the DatasetMetadataManagerWidget
 */
void DatasetMetadataManagerDialog::setURL(const QString& url) {
	m_mainWidget->setURL(url);
}
