/***************************************************************************
File                 : ImportDatasetWidget.cpp
Project              : LabPlot
Description          : import online dataset widget
--------------------------------------------------------------------
Copyright            : (C) 2019 Kovacs Ferencz (kferike98@gmail.com)

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

#include "src/backend/datasources/DatasetHandler.h"
#include "src/kdefrontend/datasources/ImportDatasetWidget.h"
#include "src/kdefrontend/datasources/DatasetMetadataManagerDialog.h"
#include "src/kdefrontend/DatasetModel.h"

#include <QCompleter>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QStandardPaths>
#include <QFile>
#include <QDebug>
#include <QTreeWidget>
#include <QMessageBox>
#include <QDir>
#include <KLocalizedString>
#include <KMessageBox>
#include <KNS3/DownloadDialog>
#include <KNewStuff3/KNS3/DownloadDialog>
#include <KNS3/DownloadManager>
#include <KNS3/UploadDialog>

/*!
	\class ImportDatasetWidget
	\brief Widget for importing data from a dataset.

	\ingroup kdefrontend
 */
ImportDatasetWidget::ImportDatasetWidget(QWidget* parent) : QWidget(parent),
	m_categoryCompleter(new QCompleter),
	m_datasetCompleter(new QCompleter),
	m_loadingCategories(false) {
	const QString baseDir = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
	QString containingDir = "labplot_data";
	m_jsonDir = baseDir + QDir::separator() + containingDir + QDir::separator();
	ui.setupUi(this);

	if(!QFile(m_jsonDir + "DatasetCategories.json").exists())
		downloadCategoryFile();
	loadDatasetCategoriesFromJson();

	ui.lwDatasets->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.twCategories->setSelectionMode(QAbstractItemView::SingleSelection);

	connect(ui.twCategories, &QTreeWidget::itemDoubleClicked, this, &ImportDatasetWidget::listDatasetsForSubcategory);
	connect(ui.twCategories, &QTreeWidget::itemSelectionChanged, [this] {
		if(!m_loadingCategories)
			listDatasetsForSubcategory(ui.twCategories->selectedItems().first());
	});
	connect(ui.leSearchDatasets, &QLineEdit::textChanged, this, &ImportDatasetWidget::scrollToDatasetListItem);
	connect(ui.bClearCache, &QPushButton::clicked, this, &ImportDatasetWidget::clearCache);
	connect(ui.leSearchCategories, &QLineEdit::textChanged, this, &ImportDatasetWidget::scrollToCategoryTreeItem);
	connect(ui.bRefresh, &QPushButton::clicked, this, &ImportDatasetWidget::refreshCategories);
	connect(ui.bNewDataset, &QPushButton::clicked, this, &ImportDatasetWidget::showDatasetMetadataManager);
	connect(ui.lwDatasets, &QListWidget::itemSelectionChanged, [this]() {
		emit datasetSelected();
	});
}

ImportDatasetWidget::~ImportDatasetWidget() {
	if(m_categoryCompleter != nullptr)
		delete m_categoryCompleter;
	if(m_datasetCompleter != nullptr)
		delete m_datasetCompleter;

	//TODO: Save selected state
}

/**
 * @brief Locates in the file system the json metadata file that contains the list of categories and subcategories.
 * @return The location of the file
 */
QString ImportDatasetWidget::locateCategoryJsonFile() const {
	qDebug() << "Locating category file" << QStandardPaths::locate(QStandardPaths::AppDataLocation, "datasets/DatasetCategories.json");
	return QStandardPaths::locate(QStandardPaths::AppDataLocation, "datasets/DatasetCategories.json");
}

/**
 * @brief Processes the json metadata file that contains the list of categories and subcategories and their datasets.
 */
void ImportDatasetWidget::loadDatasetCategoriesFromJson() {
	qDebug() << "loading Categories";

	QString filePath = m_jsonDir + "DatasetCategories.json";
	QFile file(filePath);

	if (file.open(QIODevice::ReadOnly)) {
		m_loadingCategories = true;
		ui.lwDatasets->clear();
		ui.twCategories->clear();
		m_datasetsMap.clear();

		QJsonDocument document = QJsonDocument::fromJson(file.readAll());
		QJsonArray categoryArray = document.object().value("categories").toArray();

		//processing categories
		for(int i = 0 ; i < categoryArray.size(); ++i) {
			const QJsonObject currentCategory = categoryArray[i].toObject();
			const QString categoryName = currentCategory.value("category_name").toString();
			QTreeWidgetItem* const currentCategoryItem = new QTreeWidgetItem(QStringList(categoryName));
			const QJsonArray subcategories = currentCategory.value("subcategories").toArray();
			ui.twCategories->addTopLevelItem(currentCategoryItem);

			//processing subcategories
			for(int j = 0; j < subcategories.size(); ++j) {
				QJsonObject currentSubCategory = subcategories[j].toObject();
				QString subcategoryName = currentSubCategory.value("subcategory_name").toString();
				currentCategoryItem->addChild(new QTreeWidgetItem(QStringList(subcategoryName)));
				const QJsonArray datasetArray = currentSubCategory.value("datasets").toArray();

				//processing the datasets o the actual subcategory
				for (const auto& dataset : datasetArray) {
					m_datasetsMap[categoryName][subcategoryName].push_back(dataset.toString());
				}
			}
		}

		if(m_datasetModel != nullptr)
			delete m_datasetModel;

		m_datasetModel = new DatasetModel(m_datasetsMap);
		updateCategoryCompleter();
		m_loadingCategories = false;
		restoreSelectedSubcategory();
		file.close();
	} else {
		qDebug("Couldn't open dataset category file");
	}
}

/**
 * @brief Restores the lastly selected subcategory making it the selected QTreeWidgetItem and also lists the datasets belonigng to it/
 */
void ImportDatasetWidget::restoreSelectedSubcategory() {
	if(m_datasetModel->categories().toStringList().contains(m_selectedCategory)) {
		const QTreeWidgetItem* const categoryItem = ui.twCategories->findItems(m_selectedCategory, Qt::MatchExactly).first();

		if(m_datasetModel->subcategories(m_selectedCategory).toStringList().contains(m_selectedSubcategory)) {
			for(int i = 0; i < categoryItem->childCount(); ++i)	{
				if(categoryItem->child(i)->text(0).compare(m_selectedSubcategory) == 0) {
					QTreeWidgetItem* const subcategoryItem = categoryItem->child(i);
					ui.twCategories->setCurrentItem(subcategoryItem);
					subcategoryItem->setSelected(true);
					m_selectedSubcategory.clear();
					listDatasetsForSubcategory(subcategoryItem);
					break;
				}
			}
		}
	}
}

/**
 * @brief Populates lwDatasets with the datasets of the selected subcategory.
 * @param item the selected subcategory
 */
void ImportDatasetWidget::listDatasetsForSubcategory(QTreeWidgetItem* item) {
	if(item->childCount() == 0) {
		if(m_selectedSubcategory.compare(item->text(0)) != 0) {
			m_selectedSubcategory = item->text(0);
			m_selectedCategory = item->parent()->text(0);
			QString categoryName = item->parent()->text(0);

			ui.lwDatasets->clear();
			for(QString dataset :  m_datasetModel->datasets(categoryName, m_selectedSubcategory).toStringList()) {
				ui.lwDatasets->addItem(new QListWidgetItem(dataset));
			}

			updateDatasetCompleter();
			highlightLocalMetadataFiles();
		}
	} else {
		if(item->text(0).compare(m_selectedCategory) != 0) {
			m_selectedCategory = item->text(0);
			m_selectedSubcategory = "";
			ui.lwDatasets->clear();
			item->setExpanded(true);
		}
	}
}

/**
 * @brief Updates the completer used for searching among datasets.
 */
void ImportDatasetWidget::updateDatasetCompleter() {
	QStringList datasetList;
	for(int i = 0; i <ui.lwDatasets->count(); ++i) {
		datasetList.append(ui.lwDatasets->item(i)->text());
	}

	if(!datasetList.isEmpty()) {
		if(m_datasetCompleter != nullptr)
			delete m_datasetCompleter;

		m_datasetCompleter = new QCompleter(datasetList);
		m_datasetCompleter->setCompletionMode(QCompleter::PopupCompletion);
		m_datasetCompleter->setCaseSensitivity(Qt::CaseSensitive);
		ui.leSearchDatasets->setCompleter(m_datasetCompleter);
	} else
		ui.leSearchDatasets->setCompleter(nullptr);
}

/**
 * @brief Updates the completer used for searching among categories and subcategories.
 */
void ImportDatasetWidget::updateCategoryCompleter() {
	QStringList categoryList;
	for (int i = 0; i < ui.twCategories->topLevelItemCount(); ++i) {
		categoryList.append(ui.twCategories->topLevelItem(i)->text(0));
		for(int j = 0; j < ui.twCategories->topLevelItem(i)->childCount(); ++j) {
			categoryList.append(ui.twCategories->topLevelItem(i)->text(0) + QLatin1Char(':') + ui.twCategories->topLevelItem(i)->child(j)->text(0));
		}
	}

	if(!categoryList.isEmpty()) {
		if(m_categoryCompleter != nullptr)
			delete m_categoryCompleter;

		m_categoryCompleter = new QCompleter(categoryList);
		m_categoryCompleter->setCompletionMode(QCompleter::PopupCompletion);
		m_categoryCompleter->setCaseSensitivity(Qt::CaseSensitive);
		ui.leSearchCategories->setCompleter(m_categoryCompleter);
	} else
		ui.leSearchCategories->setCompleter(nullptr);
}

/**
 * @brief Scrolls the twCategories to the given category or subcategory
 * @param rootName the name of the category or category+subcategory
 */
void ImportDatasetWidget::scrollToCategoryTreeItem(const QString& rootName) {
	int topItemIdx = -1;
	for (int i = 0; i < ui.twCategories->topLevelItemCount(); ++i)
		if (rootName.startsWith(ui.twCategories->topLevelItem(i)->text(0))) {
			topItemIdx = i;
			break;
		}

	if (topItemIdx >= 0) {
		if(!rootName.contains(QLatin1Char(':'))) {
			ui.twCategories->scrollToItem(ui.twCategories->topLevelItem(topItemIdx),
										  QAbstractItemView::ScrollHint::PositionAtTop);
		} else {
			int childIdx = -1;
			for(int j = 0; j < ui.twCategories->topLevelItem(topItemIdx)->childCount(); ++j) {
				if(rootName.endsWith(ui.twCategories->topLevelItem(topItemIdx)->child(j)->text(0))) {
					childIdx = j;
					break;
				}
			}

			if(childIdx >= 0) {
				ui.twCategories->scrollToItem(ui.twCategories->topLevelItem(topItemIdx)->child(childIdx),
											  QAbstractItemView::ScrollHint::PositionAtTop);
			} else {
				ui.twCategories->scrollToItem(ui.twCategories->topLevelItem(topItemIdx),
											  QAbstractItemView::ScrollHint::PositionAtTop);
			}
		}
	}
}

/**
 * @brief Scrolls the lwDatasets to the given dataset name.
 * @param rootName the name of the dataset
 */
void ImportDatasetWidget::scrollToDatasetListItem(const QString& rootName) {
	int itemIdx = -1;
	for (int i = 0; i < ui.lwDatasets->count(); ++i)
		if (ui.lwDatasets->item(i)->text() == rootName) {
			itemIdx = i;
			break;
		}

	if (itemIdx >= 0)
		ui.lwDatasets->scrollToItem(ui.lwDatasets->item(itemIdx),
									QAbstractItemView::ScrollHint::PositionAtTop);
}

/**
 * @brief Returns the name of the selected dataset
 */
QString ImportDatasetWidget::getSelectedDataset() const {
	if (ui.lwDatasets->selectedItems().count() > 0) {
		return ui.lwDatasets->selectedItems().at(0)->text();
	} else
		return QString();
}

/**
 * @brief Initiates the processing of the dataset's metadata file and of the dataset itself.
 * @param datasetHandler the DatasetHanlder that downloads processes the dataset
 */
void ImportDatasetWidget::loadDatasetToProcess(DatasetHandler* datasetHandler) {
	const QString fileName = getSelectedDataset() + QLatin1String(".json");
	downloadDatasetFile(fileName);

	QString filePath = m_jsonDir + m_selectedCategory + QDir::separator() + m_selectedSubcategory + QDir::separator() + fileName;

	if(QFile::exists(filePath)) {
		datasetHandler->processMetadata(filePath);
	} else  {
		QMessageBox::critical(this, i18n("Can't locate file"), i18n("The metadata file for the choosen dataset can't be located"));
	}
}

/**
 * @brief Opens the DatasetMetadataManagerDialog when the user wants to add a new dataset.
 */
void ImportDatasetWidget::showDatasetMetadataManager() {
	DatasetMetadataManagerDialog* dlg = new DatasetMetadataManagerDialog(this, m_datasetsMap);

	if (dlg->exec() == QDialog::Accepted) {
		const QString pathToJson =  m_jsonDir + QLatin1String("DatasetCategories.json");
		const QString dirPath = QFileInfo(pathToJson).dir().absolutePath();
		dlg->updateDocument(pathToJson);
		dlg->createNewMetadata(dirPath);
		uploadCategoryFile();
		uploadDatasetFile(dlg->getMetadataFilePath());
		loadDatasetCategoriesFromJson();
	}
	delete dlg;
}

/**
 * @brief Places the metadata file containing the categories and subcategories into a specific directory.
 */
void ImportDatasetWidget::downloadCategoryFile() {
	qDebug() << "Downloading category file";
	const QString fileNameOld = QStandardPaths::locate(QStandardPaths::AppDataLocation, "datasets/DatasetCategories.json");
	const QString fileNameNew =m_jsonDir + QLatin1String("DatasetCategories.json");
	const QString parentDir  = m_jsonDir.left(m_jsonDir.left(m_jsonDir.length() - 1).lastIndexOf(QDir::separator()));

	if(!QDir(m_jsonDir).exists()) {
		qDebug() << parentDir;
		QDir(parentDir).mkdir(QLatin1String("labplot_data"));
	}

	QFile::copy(fileNameOld, fileNameNew);
}

/**
 * @brief Places the metadata file of the given dataset into a specific directory.
 * @param datasetName the name of the dataset
 */
void ImportDatasetWidget::downloadDatasetFile(const QString& datasetName) {
	const QString fileNameOld = QStandardPaths::locate(QStandardPaths::AppDataLocation, QLatin1String("datasets") + QDir::separator() + datasetName);
	QString pathToNewFile = m_jsonDir + m_selectedCategory;

	//Create directory structure based on category and subcategory name
	if(!QDir(m_jsonDir + m_selectedCategory).exists()) {
		qDebug() <<m_jsonDir + m_selectedCategory;
		QDir(m_jsonDir).mkdir(m_selectedCategory);
	}

	if(!QDir(pathToNewFile + QDir::separator() + m_selectedSubcategory).exists()) {
		qDebug() <<pathToNewFile + QDir::separator() + m_selectedSubcategory;
		QDir(pathToNewFile).mkdir(m_selectedSubcategory);
	}

	pathToNewFile = pathToNewFile + QDir::separator() + m_selectedSubcategory;
	const QString fileNameNew = pathToNewFile + QDir::separator() + datasetName;
	QFile::copy(fileNameOld, fileNameNew);
}

/**
 * @brief Refreshes the categories, subcategories and datasets.
 */
void ImportDatasetWidget::refreshCategories() {
	qDebug() << "Refresh categories";
	QString fileNameNew = m_jsonDir + QLatin1String("DatasetCategories.json");
	qDebug() << fileNameNew;

	QFile existingCategoriesFile(fileNameNew);
	if(existingCategoriesFile.exists()) {
		qDebug() << "Creating backup";
		QFile oldBackup(m_jsonDir + QLatin1String("DatasetCategories_backup.json"));
		if(oldBackup.exists()) {
			oldBackup.remove();
			qDebug() << "Old backup removed";
		}
		oldBackup.close();

		if(existingCategoriesFile.rename(m_jsonDir + QLatin1String("DatasetCategories_backup.json")))
			qDebug() << "Creating backup done";
		else {
			qDebug() << " Couldn't create backup because " << existingCategoriesFile.errorString();
		}
	}

	downloadCategoryFile();
	loadDatasetCategoriesFromJson();
}

/**
 * @brief Clears the content of the directory in which the download of metadata files was done.
 */
void ImportDatasetWidget::clearCache() {
	QDir dir(m_jsonDir);

	if(dir.exists()) {
		for(const auto& entry : dir.entryList()) {
			if(!(entry.startsWith(QLatin1String("DatasetCategories")) || entry.startsWith(QLatin1Char('.')))) {
				QDir deleteDir(m_jsonDir + entry);
				if(deleteDir.exists()) {
					deleteDir.removeRecursively();
					qDebug() << "Delete " << deleteDir.absolutePath();
				}
			}
		}
	} else {
		qDebug("Couldn't clear cache, containing folder doesn't exist!");
	}

	highlightLocalMetadataFiles();
}

/**
 * @brief Highlights the name of the locally available metadata files in lwDatasets.
 */
void ImportDatasetWidget::highlightLocalMetadataFiles() {
	const QString filePath = m_jsonDir + m_selectedCategory + QDir::separator() + m_selectedSubcategory + QDir::separator();

	for(int i = 0 ; i < ui.lwDatasets->count(); ++i) {
		QListWidgetItem* const currentItem = ui.lwDatasets->item(i);
		const QFile file(filePath + currentItem->text() + QLatin1String(".json"));

		if(file.exists())
			currentItem->setBackgroundColor(Qt::yellow);
		else
			currentItem->setBackgroundColor(Qt::white);
	}
}

/**
 * @brief TODO: uploads the metadata file that contains the categories to store.kde.org -- Library doesn't work for indefinite time.
 */
void ImportDatasetWidget::uploadCategoryFile() {
	/*KNS3::UploadDialog dialog("labplot2_datasets.knsrc", this);

	QFile file(m_jsonDir + "DatasetCategories.json");
	qDebug() << "file " << m_jsonDir + "DatasetCategories.json "<< file.exists();
	qDebug() << "file can be opened: " << file.open(QIODevice::ReadOnly) << "  " << file.errorString();
	file.close();

	QUrl payloadFile ="file:" + m_jsonDir + "DatasetCategories.json";
	QFile file2(payloadFile.toLocalFile());
	qDebug() << "Local file: " << payloadFile.toLocalFile();
		 if (!file2.open(QIODevice::ReadOnly)) {
			 qDebug() << i18n("File not found: %1  ", payloadFile.url());
		 } else {
			 qDebug() << i18n("File found: %1  ", payloadFile.url());
		 }
	file2.close();

	dialog.setUploadFile("file:" + m_jsonDir + "DatasetCategories.json");
	qDebug("Upload file set!");
	dialog.setUploadName("Dataset Categories");
	qDebug() << "Upload name set: ";

	dialog.exec();*/
}

/**
 * @brief TODO: uploads the metadata file of a dataset to store.kde.org -- Library doesn't work for indefinite time.
 */
void ImportDatasetWidget::uploadDatasetFile(const QString& filePath) {
	/*KNS3::UploadDialog dialog("labplot2_datasets.knsrc", this);

	QFile file(filePath);
	qDebug() << filePath + "  " << file.exists();
	qDebug() << "file can be opened: " << file.open(QIODevice::ReadOnly) << "  " << file.errorString();
	file.close();

	QUrl payloadFile ="file:" + filePath;
	QFile file2(payloadFile.toLocalFile());
	qDebug() << "Local file: " << payloadFile.toLocalFile();
		 if (!file2.open(QIODevice::ReadOnly)) {
			 qDebug() << i18n("File not found: %1  ", payloadFile.url());
		 } else {
			 qDebug() << i18n("File found: %1  ", payloadFile.url());
		 }
	file2.close();

	dialog.setUploadFile("file:" + filePath);
	qDebug("Upload file set!");
	dialog.setUploadName("Dataset Categories");
	qDebug() << "Upload name set: ";

	dialog.exec();*/
}

/**
 * @brief Returns the structure containing the categories, subcategories and datasets.
 * @return the structure containing the categories, subcategories and datasets
 */
const QMap<QString, QMap<QString, QVector<QString>>>& ImportDatasetWidget::getDatasetsMap() {
	return m_datasetsMap;
}

/**
 * @brief Sets the currently selected category
 * @param category the name of the category
 */
void ImportDatasetWidget::setCategory(const QString &category) {
	for(int i = 0; i < ui.twCategories->topLevelItemCount(); i++) {
		if (ui.twCategories->topLevelItem(i)->text(0).compare(category) == 0) {
			listDatasetsForSubcategory(ui.twCategories->topLevelItem(i));
			break;
		}
	}
}

/**
 * @brief Sets the currently selected subcategory
 * @param subcategory the name of the subcategory
 */
void ImportDatasetWidget::setSubcategory(const QString &subcategory) {
	for(int i = 0; i < ui.twCategories->topLevelItemCount(); i++) {
		if (ui.twCategories->topLevelItem(i)->text(0).compare(m_selectedCategory) == 0) {
			QTreeWidgetItem* categoryItem = ui.twCategories->topLevelItem(i);
			for(int j = 0; j <categoryItem->childCount(); j++) {
				if(categoryItem->child(j)->text(0).compare(subcategory) == 0) {
					listDatasetsForSubcategory(categoryItem->child(j));
					break;
				}
			}
			break;
		}
	}
}

/**
 * @brief  Sets the currently selected dataset
 * @param the currently selected dataset
 */
void ImportDatasetWidget::setDataset(const QString &datasetName) {
	for(int i = 0; i < ui.lwDatasets->count() ; i++) {
		if(ui.lwDatasets->item(i)->text().compare(datasetName) == 0) {
			ui.lwDatasets->item(i)->setSelected(true);
			break;
		}
	}
}
