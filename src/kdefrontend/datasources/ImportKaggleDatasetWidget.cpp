/*
	File                 : ImportKaggleDatasetWidget.cpp
	Project              : LabPlot
	Description          : import kaggle dataset widget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Israel Galadima <izzygaladima@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kdefrontend/datasources/ImportKaggleDatasetWidget.h"
#include "backend/core/Settings.h"
#include "kdefrontend/datasources/ImportFileWidget.h"

#include <QCompleter>
#include <QDirIterator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QTableWidget>

#include <KConfigGroup>
#include <KUrlComboBox>

ImportKaggleDatasetWidget::ImportKaggleDatasetWidget(QWidget* parent)
	: QWidget(parent)
	, m_importFileWidget(new ImportFileWidget(this, false, QString(), true))
	, m_kaggleCli(new QProcess(this))
	, m_cbDatasetFiles(new QComboBox(this)) {
	ui.setupUi(this);
	ui.bottomContainer->hide();

	prepareImportKaggleDatasetWidget();
	prepareImportFileWidget();

	auto* bottomContainerLayout = new QVBoxLayout();
	bottomContainerLayout->addWidget(m_importFileWidget);
	ui.bottomContainer->setLayout(bottomContainerLayout);

	const auto group = Settings::group(QStringLiteral("Settings_Datasets"));
	m_kaggleCli->setProgram(group.readEntry(QLatin1String("KaggleCLIPath"), QString()));

	connect(ui.leSearch, &TimedLineEdit::textChanged, [&] {
		Q_EMIT clearError();
		m_resultPage = 1;
		searchKaggleDatasets();
	});
	connect(ui.bPrev, &QToolButton::clicked, [&] {
		Q_EMIT clearError();
		m_resultPage -= 1;
		searchKaggleDatasets();
	});
	connect(ui.bNext, &QToolButton::clicked, [&] {
		Q_EMIT clearError();
		m_resultPage += 1;
		searchKaggleDatasets();
	});
	connect(ui.lwDatasets, &QListWidget::currentRowChanged, [&](int index) {
		if (index != -1) {
			Q_EMIT clearError();
			m_cbDatasetFiles->clear();
			ui.bottomContainer->hide();
			m_importFileWidget->ui.tePreview->clear();
			m_importFileWidget->m_twPreview->clear();
			ui.lInfo->clear();
			ui.bDownload->setEnabled(true);
			fetchKaggleDatasetMetadata(index);
			Q_EMIT toggleOkBtn(false);
			Q_EMIT toggleOptionsBtn(false);
		}
	});
	connect(ui.bDownload, &QToolButton::clicked, [&] {
		Q_EMIT clearError();
		downloadKaggleDataset();
		ui.bDownload->setEnabled(false);
	});
	connect(m_kaggleCli, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [&](int exitCode, QProcess::ExitStatus exitStatus) {
		RESET_CURSOR;
		if (exitStatus == QProcess::NormalExit && exitCode == 0) {
			if (m_kaggleCli->arguments().at(1) == QLatin1String("list"))
				listKaggleDatasets();
			else if (m_kaggleCli->arguments().at(1) == QLatin1String("metadata"))
				displayKaggleDatasetMetadata();
			else if (m_kaggleCli->arguments().at(1) == QLatin1String("download"))
				listDownloadedKaggleDatasetFiles(m_kaggleCli->arguments().at(4));
		}
	});
	connect(m_kaggleCli, &QProcess::errorOccurred, [&] {
		RESET_CURSOR;
	});
	connect(m_cbDatasetFiles, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index) {
		if (index != -1) {
			Q_EMIT clearError();
			m_importFileWidget->m_cbFileName->clear();
			QString datasetFilePath = m_currentDatasetDirPath + QLatin1String("/") + m_cbDatasetFiles->itemText(index);
			m_importFileWidget->m_cbFileName->setUrl(QUrl(datasetFilePath));
			m_importFileWidget->fileNameChanged(datasetFilePath);
		}
	});
	connect(m_importFileWidget, &ImportFileWidget::previewReady, [&] {
		Q_EMIT toggleOkBtn(true);
	});

	Q_EMIT ui.leSearch->textChanged();
}

ImportKaggleDatasetWidget::~ImportKaggleDatasetWidget() {
}

void ImportKaggleDatasetWidget::prepareImportFileWidget() {
	m_importFileWidget->ui.gbOptions->layout()->addWidget(m_cbDatasetFiles);
	m_importFileWidget->initSlots();
}

void ImportKaggleDatasetWidget::prepareImportKaggleDatasetWidget() {
	const int size = ui.leSearch->height();
	ui.lSearch->setPixmap(QIcon::fromTheme(QStringLiteral("edit-find")).pixmap(size, size));

	QString info = i18n("Enter the keyword you want to search for");
	ui.lSearch->setToolTip(info);
	ui.leSearch->setToolTip(info);
	ui.leSearch->setPlaceholderText(i18n("Search..."));
	ui.leSearch->setFocus();

	ui.lCategories->setText(i18n("Dataset:"));
	ui.bPrev->setIcon(QIcon::fromTheme(QStringLiteral("go-previous")));
	ui.bNext->setIcon(QIcon::fromTheme(QStringLiteral("go-next")));
	ui.bDownload->setIcon(QIcon::fromTheme(QStringLiteral("download")));
}

void ImportKaggleDatasetWidget::searchKaggleDatasets() {
	QString filter = ui.leSearch->text();

	QStringList arguments{QLatin1String("datasets"),
						  QLatin1String("list"),
						  QLatin1String("--csv"),
						  QLatin1String("--file-type"),
						  QLatin1String("csv"),
						  QLatin1String("--max-size"),
						  QLatin1String("10000000"), // 10mb
						  QLatin1String("--page"),
						  QString::number(m_resultPage)};

	if (!filter.isEmpty())
		arguments << QLatin1String("--search") << filter;

	if (m_kaggleCli->state() != QProcess::NotRunning) {
		RESET_CURSOR;
		m_kaggleCli->kill();
	}

	m_kaggleCli->setArguments(arguments);
	m_kaggleCli->start();
	WAIT_CURSOR;
	DEBUG(Q_FUNC_INFO);
	DEBUG(m_kaggleCli->program().toStdString() + " " + arguments.join(QStringLiteral(" ")).toStdString());
}

void ImportKaggleDatasetWidget::listKaggleDatasets() {
	DEBUG(Q_FUNC_INFO);

	if (m_resultPage == 1)
		ui.bPrev->setEnabled(false);
	else
		ui.bPrev->setEnabled(true);

	QLatin1String headerRow("ref,title,size,lastUpdated,downloadCount,voteCount,usabilityRating");
	QLatin1String noDatasetsRow("No datasets found");

	m_kaggleCli->setReadChannel(QProcess::StandardOutput);

	ui.lwDatasets->clear();

	while (m_kaggleCli->canReadLine()) {
		QString row = QLatin1String(m_kaggleCli->readLine()).trimmed();

		if (row.startsWith(QLatin1String("Warning: ")) || row == headerRow || row == noDatasetsRow)
			continue;

		QString datasetName = row.section(QLatin1String(","), 1, 1);
		QString refName = row.section(QLatin1String(","), 0, 0);

		auto* listItem = new QListWidgetItem(datasetName);
		listItem->setData(Qt::ItemDataRole::UserRole, refName);

		ui.lwDatasets->addItem(listItem);
	}

	if (ui.lwDatasets->count() == 0) {
		ui.bNext->setEnabled(false);
		ui.bDownload->setEnabled(false);
	} else {
		if (ui.lwDatasets->count() == RESULTS_PER_PAGE)
			ui.bNext->setEnabled(true);
		else
			ui.bNext->setEnabled(false);

		ui.lwDatasets->setCurrentRow(0);
	}
}

void ImportKaggleDatasetWidget::fetchKaggleDatasetMetadata(int index) {
	QString refName = ui.lwDatasets->item(index)->data(Qt::ItemDataRole::UserRole).toString();

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
	DEBUG(m_kaggleCli->program().toStdString() + " " + arguments.join(QStringLiteral(" ")).toStdString());
}

void ImportKaggleDatasetWidget::displayKaggleDatasetMetadata() {
	DEBUG(Q_FUNC_INFO);

	ui.bDownload->setEnabled(true);
	QString datasetInfo;

	QString metadataFilePath = QStandardPaths::locate(QStandardPaths::TempLocation, QStringLiteral("dataset-metadata.json"), QStandardPaths::LocateFile);
	QFile metadataFile(metadataFilePath);
	if (metadataFile.open(QIODevice::ReadOnly)) {
		QJsonDocument metadataFileJsondocument = QJsonDocument::fromJson(metadataFile.readAll());
		metadataFile.close();

		if (!metadataFileJsondocument.isObject()) {
			Q_EMIT showError(i18n("Could not read %1", metadataFilePath));
			ui.lInfo->setText(datasetInfo);
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
	ui.lInfo->setText(datasetInfo);
}

void ImportKaggleDatasetWidget::downloadKaggleDataset() {
	QString refName = ui.lwDatasets->selectedItems().at(0)->data(Qt::ItemDataRole::UserRole).toString();

	QDir datasetDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/datasets_local/")
					+ QString(refName).replace(QLatin1String("/"), QLatin1String("_")));

	if (datasetDir.exists()) {
		listDownloadedKaggleDatasetFiles(datasetDir.path());
		return;
	}

	if (!datasetDir.mkpath(datasetDir.path())) {
		Q_EMIT showError(i18n("Could not create %1", datasetDir.path()));
		return;
	}

	QStringList arguments{QLatin1String("datasets"), QLatin1String("download"), QLatin1String("--unzip"), QLatin1String("--path"), datasetDir.path(), refName};
	if (m_kaggleCli->state() != QProcess::NotRunning) {
		RESET_CURSOR;
		m_kaggleCli->kill();
	}
	m_kaggleCli->setArguments(arguments);
	m_kaggleCli->start();
	WAIT_CURSOR;
	DEBUG(Q_FUNC_INFO);
	DEBUG(m_kaggleCli->program().toStdString() + " " + arguments.join(QStringLiteral(" ")).toStdString());
}

void ImportKaggleDatasetWidget::listDownloadedKaggleDatasetFiles(const QString& datasetDirPath) {
	DEBUG(Q_FUNC_INFO);

	m_currentDatasetDirPath = datasetDirPath;
	m_cbDatasetFiles->clear();
	QStringList datasetFiles;

	QDirIterator datasetDirIterator(datasetDirPath, QStringList() << QStringLiteral("*.csv"), QDir::Files, QDirIterator::Subdirectories);
	while (datasetDirIterator.hasNext()) {
		datasetDirIterator.next();
		datasetFiles.append(datasetDirIterator.fileName());
	}

	if (datasetFiles.isEmpty()) {
		Q_EMIT showError(i18n("Could not find .csv files in %1", datasetDirPath));
	} else {
		ui.bottomContainer->show();
		m_importFileWidget->ui.gbOptions->show();
		m_importFileWidget->ui.gbDataSource->hide();
		m_importFileWidget->ui.gbUpdateOptions->hide();
		m_cbDatasetFiles->addItems(datasetFiles);
		Q_EMIT toggleOptionsBtn(true);
	}
}

void ImportKaggleDatasetWidget::importToSpreadsheet(Spreadsheet* spreadsheet) const {
	DEBUG(Q_FUNC_INFO);
	spreadsheet->setName(ui.lwDatasets->selectedItems().at(0)->text());
	spreadsheet->setComment(ui.lInfo->text());
	m_importFileWidget->currentFileFilter()->readDataFromFile(m_currentDatasetDirPath + QLatin1String("/") + m_cbDatasetFiles->currentText(), spreadsheet);
}

void ImportKaggleDatasetWidget::toggleOptionsVisibility() {
	if (ui.bottomContainer->isVisible()) {
		ui.bottomContainer->hide();
	} else {
		ui.bottomContainer->show();
		m_importFileWidget->ui.gbOptions->show();
		m_importFileWidget->ui.gbDataSource->hide();
		m_importFileWidget->ui.gbUpdateOptions->hide();
	}
}