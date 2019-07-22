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

	if(!QFile(m_jsonDir + "DatasetCollections.json").exists())
		downloadCollectionsFile();
	loadDatasetCategoriesFromJson();

	ui.lwDatasets->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.twCategories->setSelectionMode(QAbstractItemView::SingleSelection);

	connect(ui.cbCollections, &QComboBox::currentTextChanged, this, &ImportDatasetWidget::updateCategoryTree);
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
	connect(ui.lwDatasets, &QListWidget::doubleClicked, [this]() {
		emit datasetDoubleClicked();
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
	qDebug() << "loading Collections";

	QString filePath = m_jsonDir + "DatasetCollections.json";
	QFile file(filePath);

	if (file.open(QIODevice::ReadOnly)) {
		m_datasetsMap.clear();
		ui.cbCollections->clear();
		ui.cbCollections->addItem("All");

		QJsonDocument document = QJsonDocument::fromJson(file.readAll());
		QJsonArray collections;
		if(document.isArray())
			collections = document.array();
		else {
			qDebug()<< "The DatasetCollections.json file is invalid";
			return;
		}

		for (int collectionIndex = 0; collectionIndex < collections.size(); collectionIndex++) {
			const QString currentCollection = collections[collectionIndex].toString();
			ui.cbCollections->addItem(currentCollection);

			if(!QFile(m_jsonDir + currentCollection + ".json").exists())
				downloadCollectionFile(currentCollection + ".json");

			QFile collectionFile(m_jsonDir + currentCollection + ".json");

			if (collectionFile.open(QIODevice::ReadOnly)) {
				QJsonDocument collectionDocument = QJsonDocument::fromJson(collectionFile.readAll());
				QJsonObject collectionObject;
				//qDebug() << "Collection document: " << collectionDocument;
				//qDebug() << "Is object" << collectionDocument.isObject() << collectionDocument.isArray() << collectionDocument.isEmpty();
				if(collectionDocument.isObject()) {
					collectionObject = collectionDocument.object();
				} else {
					qDebug()<< "The " +  currentCollection + ".json file is invalid";
					return;
				}

				if(collectionObject.value("collection_name").toString().compare(currentCollection) != 0) {
					qDebug()<< "The " +  currentCollection + ".json file name is invalid";
					return;
				}

				QJsonArray categoryArray = collectionObject.value("categories").toArray();

				//processing categories
				for(int i = 0 ; i < categoryArray.size(); ++i) {
					const QJsonObject currentCategory = categoryArray[i].toObject();
					const QString categoryName = currentCategory.value("category_name").toString();
					const QJsonArray subcategories = currentCategory.value("subcategories").toArray();

					//processing subcategories
					for(int j = 0; j < subcategories.size(); ++j) {
						QJsonObject currentSubCategory = subcategories[j].toObject();
						QString subcategoryName = currentSubCategory.value("subcategory_name").toString();
						const QJsonArray datasetArray = currentSubCategory.value("datasets").toArray();

						//processing the datasets o the actual subcategory
						for (const auto& dataset : datasetArray) {
							m_datasetsMap[currentCollection][categoryName][subcategoryName].push_back(dataset.toObject().value("filename").toString());
						}
					}
				}
			}
		}


		if(m_datasetModel != nullptr)
			delete m_datasetModel;
		m_datasetModel = new DatasetModel(m_datasetsMap);
		updateCategoryTree(ui.cbCollections->currentText());
		restoreSelectedSubcategory(ui.cbCollections->currentText());
		file.close();
	} else {
		qDebug("Couldn't open dataset collections file");
	}
}

void ImportDatasetWidget::updateCategoryTree(const QString& collection) {
	m_loadingCategories = true;
	ui.lwDatasets->clear();
	ui.twCategories->clear();
	QStringList categories = (collection.compare("All") == 0) ? m_datasetModel->allCategories().toStringList() : m_datasetModel->categories(collection);

	for(auto category : categories) {
		QTreeWidgetItem* const currentCategoryItem = new QTreeWidgetItem(QStringList(category));
		ui.twCategories->addTopLevelItem(currentCategoryItem);

		QStringList subcategories = (collection.compare("All") == 0) ?
					m_datasetModel->allSubcategories(category).toStringList() : m_datasetModel->subcategories(collection, category);

		for(auto subcategory : subcategories) {
			currentCategoryItem->addChild(new QTreeWidgetItem(QStringList(subcategory)));
		}
	}

	if(m_selectedCollection.compare(collection) == 0) {
		restoreSelectedSubcategory(ui.cbCollections->currentText());
	} else {
		m_selectedCollection = collection;
		m_selectedCategory = "";
		m_selectedSubcategory = "";
	}

	m_loadingCategories = false;
	updateCategoryCompleter();
}

/**
 * @brief Restores the lastly selected subcategory making it the selected QTreeWidgetItem and also lists the datasets belonigng to it/
 */
void ImportDatasetWidget::restoreSelectedSubcategory(const QString& collection) {
	if(m_datasetModel->categories(collection).contains(m_selectedCategory)) {
		const QTreeWidgetItem* const categoryItem = ui.twCategories->findItems(m_selectedCategory, Qt::MatchExactly).first();

		if(m_datasetModel->subcategories(collection, m_selectedCategory).contains(m_selectedSubcategory)) {
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
			for(QString dataset :  m_datasetModel->datasets(m_selectedCollection, categoryName, m_selectedSubcategory)) {
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

	QString filePath = m_jsonDir;// + m_selectedCategory + QDir::separator() + m_selectedSubcategory + QDir::separator();

	QJsonObject datasetObject = loadDatasetObject();
	qDebug()<<"Dataset object" << datasetObject;

	if(!datasetObject.isEmpty()) {
		datasetHandler->processMetadata(datasetObject, filePath);
	} else  {
		QMessageBox::critical(this, i18n("Invalid metadata file"), i18n("The metadata file for the choosen dataset isn't valid"));
	}
}

/**
 * @brief Returns the QJsonObject associated with the currently selected dataset.
 */
QJsonObject ImportDatasetWidget::loadDatasetObject() {
	QString filePath = m_jsonDir + "DatasetCollections.json";
	QFile file(filePath);

	bool allCollections = (m_selectedCollection.compare("All") == 0);

	if (file.open(QIODevice::ReadOnly)) {
		QJsonDocument document = QJsonDocument::fromJson(file.readAll());
		QJsonArray collections;
		if(document.isArray())
			collections = document.array();
		else {
			qDebug()<< "The DatasetCollections.json file is invalid";
			return QJsonObject();
		}

		for (int collectionIndex = 0; collectionIndex < collections.size(); collectionIndex++) {
			const QString currentCollection = collections[collectionIndex].toString();

			if(currentCollection.compare(m_selectedCollection) == 0 || allCollections) {

				QFile collectionFile(m_jsonDir + currentCollection + ".json");

				if (collectionFile.open(QIODevice::ReadOnly)) {
					QJsonDocument collectionDocument = QJsonDocument::fromJson(collectionFile.readAll());
					QJsonObject collectionObject;
					qDebug() << "Collection document: " << collectionDocument;
					if(collectionDocument.isObject()) {
						collectionObject = collectionDocument.object();
					} else {
						qDebug()<< "The " +  currentCollection + ".json file is invalid";
						return QJsonObject();
					}

					if(collectionObject.value("collection_name").toString().compare(currentCollection) != 0) {
						qDebug()<< "The " +  currentCollection + ".json file's name is invalid";
						return QJsonObject();
					}

					QJsonArray categoryArray = collectionObject.value("categories").toArray();

					//processing categories
					for(int i = 0 ; i < categoryArray.size(); ++i) {
						const QJsonObject currentCategory = categoryArray[i].toObject();
						const QString categoryName = currentCategory.value("category_name").toString();
						if(categoryName.compare(m_selectedCategory) == 0) {

							const QJsonArray subcategories = currentCategory.value("subcategories").toArray();

							//processing subcategories
							for(int j = 0; j < subcategories.size(); ++j) {
								QJsonObject currentSubCategory = subcategories[j].toObject();
								QString subcategoryName = currentSubCategory.value("subcategory_name").toString();

								if(subcategoryName.compare(m_selectedSubcategory) == 0) {

									const QJsonArray datasetArray = currentSubCategory.value("datasets").toArray();

									//processing the datasets o the actual subcategory
									for (const auto& dataset : datasetArray) {
										if(getSelectedDataset().compare(dataset.toObject().value("filename").toString()) == 0)
											return dataset.toObject();
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return QJsonObject();
}

/**
 * @brief Opens the DatasetMetadataManagerDialog when the user wants to add a new dataset.
 */
void ImportDatasetWidget::showDatasetMetadataManager() {
	DatasetMetadataManagerDialog* dlg = new DatasetMetadataManagerDialog(this, m_datasetsMap);

	if (dlg->exec() == QDialog::Accepted) {
		const QString pathToJson =  m_jsonDir + QLatin1String("DatasetCategories.json");
		const QString dirPath = QFileInfo(pathToJson).dir().absolutePath();
		dlg->updateDocument(m_jsonDir);
		uploadCategoryFile();
		uploadDatasetFile(dlg->getMetadataFilePath());
		loadDatasetCategoriesFromJson();
	}
	delete dlg;
}

/**
 * @brief Places the metadata file containing the categories and subcategories into a specific directory.
 */
void ImportDatasetWidget::downloadCollectionsFile() {
	qDebug() << "Downloading collections file";
	const QString fileNameOld = QStandardPaths::locate(QStandardPaths::AppDataLocation, "datasets/DatasetCollections.json");
	const QString fileNameNew =m_jsonDir + QLatin1String("DatasetCollections.json");
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
void ImportDatasetWidget::downloadCollectionFile(const QString& collectionName) {
	const QString fileNameOld = QStandardPaths::locate(QStandardPaths::AppDataLocation, QLatin1String("datasets") + QDir::separator() + collectionName);
	const QString fileNameNew =m_jsonDir + collectionName;

	QFile::copy(fileNameOld, fileNameNew);
}

/**
 * @brief Refreshes the categories, subcategories and datasets.
 */
void ImportDatasetWidget::refreshCategories() {
	qDebug() << "Refresh categories";
	QString fileNameNew = m_jsonDir + QLatin1String("DatasetCollections.json");
	qDebug() << fileNameNew;

	QFile existingCategoriesFile(fileNameNew);
	if(existingCategoriesFile.exists()) {
		qDebug() << "Creating backup";
		QFile oldBackup(m_jsonDir + QLatin1String("DatasetCollections_backup.json"));
		if(oldBackup.exists()) {
			oldBackup.remove();
			qDebug() << "Old backup removed";
		}
		oldBackup.close();

		if(existingCategoriesFile.rename(m_jsonDir + QLatin1String("DatasetCollections_backup.json")))
			qDebug() << "Creating backup done " << m_jsonDir + QLatin1String("DatasetCollections_backup.json");
		else {
			qDebug() << " Couldn't create backup because " << existingCategoriesFile.errorString();
		}
	}

	downloadCollectionsFile();

	QString filePath = m_jsonDir + "DatasetCollections.json";
	QFile file(filePath);

	if (file.open(QIODevice::ReadOnly)) {
		m_datasetsMap.clear();

		QJsonDocument document = QJsonDocument::fromJson(file.readAll());
		QJsonArray collections;
		if(document.isArray())
			collections = document.array();
		else {
			qDebug()<< "The DatasetCollections.json file is invalid";
			return;
		}

		for (int collectionIndex = 0; collectionIndex < collections.size(); collectionIndex++) {
			const QString currentCollection = collections[collectionIndex].toString();

			QFile existingCollectionFile(m_jsonDir + currentCollection + ".json");
			if(!existingCollectionFile.exists()) {
				downloadCollectionFile(currentCollection + ".json");
			}else {
				qDebug() << "Creating backup for " << currentCollection;
				QFile oldBackupCollection(m_jsonDir + currentCollection + "_backup.json");
				if(oldBackupCollection.exists()) {
					oldBackupCollection.remove();
					qDebug() << "Old backup removed";
				}
				oldBackupCollection.close();

				if(existingCollectionFile.rename(m_jsonDir + currentCollection + "_backup.json"))
					qDebug() << "Creating backup done";
				else {
					qDebug() << " Couldn't create backup because " << existingCollectionFile.errorString();
				}

				downloadCollectionFile(currentCollection + ".json");
			}
		}
	}

	loadDatasetCategoriesFromJson();
}

/**
 * @brief Clears the content of the directory in which the download of metadata files was done.
 */
void ImportDatasetWidget::clearCache() {
	QDir dir(m_jsonDir);

	if(dir.exists()) {
		for(const auto& entry : dir.entryList()) {
			if(!(entry.endsWith(QLatin1String(".json")) || entry.startsWith(QLatin1Char('.')))) {
				QFile deleteFile (m_jsonDir + entry);
				if(deleteFile.exists()) {
					deleteFile.remove();
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
	QDir dir(m_jsonDir);

	for(int i = 0 ; i < ui.lwDatasets->count(); ++i) {
		QListWidgetItem* const currentItem = ui.lwDatasets->item(i);
		bool found = false;
		for(QString entry : dir.entryList()) {
			if(entry.startsWith(currentItem->text()) && !entry.endsWith(".json")) {
				found = true;
				break;
			}
		}

		if(found)
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
const QMap< QString, QMap<QString, QMap<QString, QVector<QString>>>>& ImportDatasetWidget::getDatasetsMap() {
	return m_datasetsMap;
}

void ImportDatasetWidget::setCollection(const QString& collection) {
	ui.cbCollections->setCurrentText(collection);
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
