/*
	File                 : ImportKaggleWidget.cpp
	Project              : LabPlot
	Description          : import kaggle dataset widget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Israel Galadima <izzygaladima@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kdefrontend/datasources/ImportKaggleWidget.h"
#include "backend/core/Settings.h"
#include "backend/lib/macros.h"
#include "kdefrontend/datasources/ImportDatasetWidget.h"
#include "kdefrontend/datasources/ImportFileWidget.h"
#include "kdefrontend/datasources/ImportKaggleWidget.h"
#include <KConfigGroup>
#include <KUrlComboBox>
#include <QComboBox>
#include <QCompleter>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QGridLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QListWidgetItem>
#include <QNetworkAccessManager>
#include <QStandardPaths>
#include <QTableWidget>

ImportKaggleWidget::ImportKaggleWidget(QWidget* parent)
	: QWidget(parent)
	, m_importDatasetWidget(new ImportDatasetWidget(this))
	, m_importFileWidget(new ImportFileWidget(this, false))
	, m_kaggleCli(new QProcess(this))
	, m_vLayout(new QVBoxLayout(this))
	, m_bPrevDatasets(new QToolButton(this))
	, m_bNextDatasets(new QToolButton(this))
	, m_bDownloadDataset(new QToolButton(this))
	, m_cbDatasetFiles(new QComboBox(this)) {
	prepareImportDatasetWidget();
	prepareImportFileWidget();
	const auto group = Settings::group(QStringLiteral("Settings_Datasets"));
	m_kaggleCli->setProgram(group.readEntry(QLatin1String("KaggleCLIPath"), QString()));

	connect(m_importDatasetWidget->ui.leSearch, &TimedLineEdit::textChanged, [&] {
		Q_EMIT clearError();
		m_resultPage = 1;
		searchKaggleDatasets();
	});
	connect(m_importDatasetWidget->ui.lwDatasets, &QListWidget::currentRowChanged, [&](int index) {
		Q_EMIT clearError();
		m_cbDatasetFiles->clear();
		m_importFileWidget->ui.gbOptions->setEnabled(false);
		m_importFileWidget->ui.tePreview->clear();
		m_importFileWidget->m_twPreview->clear();
		m_importDatasetWidget->ui.lInfo->clear();
		m_bDownloadDataset->setEnabled(true);
		if (index != -1) {
			fetchKaggleDatasetMetadata(index);
		}
		Q_EMIT toggleOkBtn(false);
	});
	connect(m_bPrevDatasets, &QToolButton::clicked, [&] {
		Q_EMIT clearError();
		m_resultPage -= 1;
		searchKaggleDatasets();
	});
	connect(m_bNextDatasets, &QToolButton::clicked, [&] {
		Q_EMIT clearError();
		m_resultPage += 1;
		searchKaggleDatasets();
	});
	connect(m_bDownloadDataset, &QToolButton::clicked, [&] {
		Q_EMIT clearError();
		downloadKaggleDataset();
		m_bDownloadDataset->setEnabled(false);
	});
	connect(m_kaggleCli, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [&](int exitCode, QProcess::ExitStatus exitStatus) {
		RESET_CURSOR;
		if (exitStatus == QProcess::NormalExit && exitCode == 0) {
			if (m_kaggleCli->arguments().at(1) == QLatin1String("list")) {
				listKaggleDatasets();
			} else if (m_kaggleCli->arguments().at(1) == QLatin1String("metadata")) {
				displayKaggleDatasetMetadata();
			} else if (m_kaggleCli->arguments().at(1) == QLatin1String("download")) {
				listDownloadedKaggleDatasetFiles(m_kaggleCli->arguments().at(4));
			}
		}
	});
	connect(m_kaggleCli, &QProcess::errorOccurred, [&] {
		RESET_CURSOR;
	});
	connect(m_cbDatasetFiles, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index) {
		Q_EMIT clearError();
		if (index != -1) {
			m_importFileWidget->m_cbFileName->clear();
			m_importFileWidget->m_cbFileName->setUrl(QUrl(m_cbDatasetFiles->itemText(index)));
			m_importFileWidget->fileNameChanged(m_cbDatasetFiles->itemText(index));
		}
	});
	connect(m_importFileWidget, &ImportFileWidget::previewReady, [&] {
		Q_EMIT toggleOkBtn(true);
	});

	addWidgetsToUi();
	Q_EMIT m_importDatasetWidget->ui.leSearch->textChanged();
}

ImportKaggleWidget::~ImportKaggleWidget() {
}

void ImportKaggleWidget::prepareImportDatasetWidget() {
	m_importDatasetWidget->ui.cbCollections->hide();
	m_importDatasetWidget->ui.lCollections->hide();
	m_importDatasetWidget->ui.twCategories->hide();
	m_importDatasetWidget->ui.lCategories->hide();

	disconnect(m_importDatasetWidget->ui.cbCollections, QOverload<int>::of(&QComboBox::currentIndexChanged), nullptr, nullptr);
	disconnect(m_importDatasetWidget->ui.twCategories, &QTreeWidget::itemDoubleClicked, nullptr, nullptr);
	disconnect(m_importDatasetWidget->ui.twCategories, &QTreeWidget::itemSelectionChanged, nullptr, nullptr);
	disconnect(m_importDatasetWidget->m_networkManager, &QNetworkAccessManager::finished, nullptr, nullptr);

	disconnect(m_importDatasetWidget->ui.leSearch, &TimedLineEdit::textChanged, nullptr, nullptr);
	disconnect(m_importDatasetWidget->ui.lwDatasets, &QListWidget::itemSelectionChanged, nullptr, nullptr);
	disconnect(m_importDatasetWidget->ui.lwDatasets, &QListWidget::doubleClicked, nullptr, nullptr);

	m_importDatasetWidget->ui.lwDatasets->clear();
	m_importDatasetWidget->ui.lInfo->clear();
	m_importDatasetWidget->ui.leSearch->setCompleter(nullptr);
	delete m_importDatasetWidget->m_completer;
}

void ImportKaggleWidget::prepareImportFileWidget() {
	m_importFileWidget->ui.gbDataSource->hide();
	m_importFileWidget->ui.gbUpdateOptions->hide();
	m_importFileWidget->ui.gbOptions->layout()->addWidget(m_cbDatasetFiles);
	m_importFileWidget->ui.gbOptions->setEnabled(false);
	m_importFileWidget->ui.gbOptions->show();
	m_importFileWidget->initSlots();
}

void ImportKaggleWidget::addWidgetsToUi() {
	m_bPrevDatasets->setText(QStringLiteral("Previous"));
	m_bPrevDatasets->setEnabled(false);
	m_importDatasetWidget->layout()->addWidget(m_bPrevDatasets);
	m_bNextDatasets->setText(QStringLiteral("Next"));
	m_bNextDatasets->setEnabled(false);
	m_importDatasetWidget->layout()->addWidget(m_bNextDatasets);
	m_bDownloadDataset->setText(QStringLiteral("Download"));
	m_bDownloadDataset->setEnabled(false);
	m_importDatasetWidget->layout()->addWidget(m_bDownloadDataset);

	m_vLayout->addWidget(m_importDatasetWidget);
	m_vLayout->addWidget(m_importFileWidget);
	this->setLayout(m_vLayout);
}

void ImportKaggleWidget::searchKaggleDatasets() {
	QString filter = m_importDatasetWidget->ui.leSearch->text();

	QStringList arguments{QLatin1String("datasets"),
						  QLatin1String("list"),
						  QLatin1String("--csv"),
						  QLatin1String("--file-type"),
						  QLatin1String("csv"),
						  QLatin1String("--max-size"),
						  QLatin1String("10000000"), // 10mb
						  QLatin1String("--page"),
						  QString::number(m_resultPage)};

	if (!filter.isEmpty()) {
		arguments << QLatin1String("--search") << filter;
	}
	if (m_kaggleCli->state() != QProcess::NotRunning) {
		RESET_CURSOR;
		m_kaggleCli->kill();
	}
	m_kaggleCli->setArguments(arguments);
	m_kaggleCli->start();
	WAIT_CURSOR;
	DEBUG(Q_FUNC_INFO);
}

void ImportKaggleWidget::listKaggleDatasets() {
	DEBUG(Q_FUNC_INFO);

	if (m_resultPage == 1) {
		m_bPrevDatasets->setEnabled(false);
	} else {
		m_bPrevDatasets->setEnabled(true);
	}

	QLatin1String headerRow("ref,title,size,lastUpdated,downloadCount,voteCount,usabilityRating");
	QLatin1String noDatasetsRow("No datasets found");

	m_kaggleCli->setReadChannel(QProcess::StandardOutput);

	m_importDatasetWidget->ui.lwDatasets->clear();

	while (m_kaggleCli->canReadLine()) {
		QString row = QLatin1String(m_kaggleCli->readLine()).trimmed();

		if (row.startsWith(QLatin1String("Warning: ")) || row == headerRow || row == noDatasetsRow) {
			continue;
		}

		QString datasetName = row.section(QLatin1String(","), 1, 1);
		QString refName = row.section(QLatin1String(","), 0, 0);

		QListWidgetItem* listItem = new QListWidgetItem(datasetName);
		listItem->setData(Qt::ItemDataRole::UserRole, refName);

		m_importDatasetWidget->ui.lwDatasets->addItem(listItem);
	}

	if (m_importDatasetWidget->ui.lwDatasets->count() == 0) {
		m_bNextDatasets->setEnabled(false);
		m_bDownloadDataset->setEnabled(false);
	} else {
		if (m_importDatasetWidget->ui.lwDatasets->count() == 20) {
			m_bNextDatasets->setEnabled(true);
		} else {
			m_bNextDatasets->setEnabled(false);
		}
		m_importDatasetWidget->ui.lwDatasets->setCurrentRow(0);
	}
}

void ImportKaggleWidget::fetchKaggleDatasetMetadata(int index) {
	QString refName = m_importDatasetWidget->ui.lwDatasets->item(index)->data(Qt::ItemDataRole::UserRole).toString();

	QStringList arguments{QLatin1String("datasets"),
						  QLatin1String("metadata"),
						  QLatin1String("--path"),
						  QStandardPaths::writableLocation(QStandardPaths::TempLocation),
						  refName};
	if (m_kaggleCli->state() != QProcess::NotRunning) {
		RESET_CURSOR;
		m_kaggleCli->kill();
	}
	m_kaggleCli->setArguments(arguments);
	m_kaggleCli->start();
	WAIT_CURSOR;
	DEBUG(Q_FUNC_INFO);
}

void ImportKaggleWidget::displayKaggleDatasetMetadata() {
	DEBUG(Q_FUNC_INFO);

	m_bDownloadDataset->setEnabled(true);
	QString datasetInfo;

	QString metadataFilePath = QStandardPaths::locate(QStandardPaths::TempLocation, QStringLiteral("dataset-metadata.json"), QStandardPaths::LocateFile);
	QFile metadataFile(metadataFilePath);
	if (metadataFile.open(QIODevice::ReadOnly)) {
		QJsonDocument metadataFileJsondocument = QJsonDocument::fromJson(metadataFile.readAll());
		metadataFile.close();

		if (!metadataFileJsondocument.isObject()) {
			Q_EMIT showError(i18n("Could not read %1", metadataFilePath));
			m_importDatasetWidget->ui.lInfo->setText(datasetInfo);
			return;
		}

		QJsonObject metadataObject = metadataFileJsondocument.object();
		datasetInfo += QStringLiteral("<b>") + i18n("Dataset") + QStringLiteral(":</b><br>");
		datasetInfo += metadataObject[QLatin1String("title")].toString();
		datasetInfo += QStringLiteral("<br><br>");
		datasetInfo += QStringLiteral("<b>") + i18n("Description") + QStringLiteral(":</b><br>");
		datasetInfo += metadataObject[QLatin1String("description")].toString();
	} else {
		Q_EMIT showError(i18n("Could not open %1", metadataFilePath));
	}
	m_importDatasetWidget->ui.lInfo->setText(datasetInfo);
}

void ImportKaggleWidget::downloadKaggleDataset() {
	QString refName = m_importDatasetWidget->ui.lwDatasets->selectedItems().at(0)->data(Qt::ItemDataRole::UserRole).toString();

	QString datasetDirPath =
		QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QStringLiteral("/") + QString(refName).replace(QLatin1String("/"), QLatin1String("_"));
	QDir datasetDir(datasetDirPath);

	if (datasetDir.exists()) {
		if (!datasetDir.removeRecursively()) {
			Q_EMIT showError(i18n("Could not remove %1", datasetDirPath));
			return;
		}
	}

	if (!datasetDir.mkpath(datasetDirPath)) {
		Q_EMIT showError(i18n("Could not create %1", datasetDirPath));
		return;
	}

	QStringList arguments{QLatin1String("datasets"), QLatin1String("download"), QLatin1String("--unzip"), QLatin1String("--path"), datasetDirPath, refName};
	if (m_kaggleCli->state() != QProcess::NotRunning) {
		RESET_CURSOR;
		m_kaggleCli->kill();
	}
	m_kaggleCli->setArguments(arguments);
	m_kaggleCli->start();
	WAIT_CURSOR;
	DEBUG(Q_FUNC_INFO);
}

void ImportKaggleWidget::listDownloadedKaggleDatasetFiles(const QString& datasetDirPath) {
	DEBUG(Q_FUNC_INFO);

	m_cbDatasetFiles->clear();

	QDirIterator datasetDirIterator(datasetDirPath, QStringList() << QStringLiteral("*.csv"), QDir::Files, QDirIterator::Subdirectories);
	while (datasetDirIterator.hasNext()) {
		QFile datasetFile(datasetDirIterator.next());
		m_cbDatasetFiles->addItem(datasetFile.fileName());
	}

	if (m_cbDatasetFiles->count() != 0) {
		m_importFileWidget->ui.gbOptions->setEnabled(true);
	}
}

void ImportKaggleWidget::importToSpreadsheet(Spreadsheet* spreadsheet) const {
	DEBUG(Q_FUNC_INFO);
	spreadsheet->setName(m_importDatasetWidget->ui.lwDatasets->selectedItems().at(0)->text());
	spreadsheet->setComment(m_importDatasetWidget->ui.lInfo->text());
	m_importFileWidget->currentFileFilter()->readDataFromFile(m_cbDatasetFiles->currentText(), spreadsheet);
}