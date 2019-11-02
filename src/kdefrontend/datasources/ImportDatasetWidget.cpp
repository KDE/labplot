/***************************************************************************
	File                 : ImportDatasetWidget.cpp
	Project              : LabPlot
	Description          : import online dataset widget
	--------------------------------------------------------------------
	Copyright            : (C) 2019 Kovacs Ferencz (kferike98@gmail.com)
	Copyright            : (C) 2019 by Alexander Semke (alexander.semke@web.de)

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

#include "backend/datasources/DatasetHandler.h"
#include "kdefrontend/datasources/ImportDatasetWidget.h"
#include "kdefrontend/datasources/DatasetMetadataManagerDialog.h"
#include "kdefrontend/DatasetModel.h"

#include <QCompleter>
#include <QJsonDocument>
#include <QJsonValue>
#include <QStandardPaths>
#include <QFile>
#include <QDebug>
#include <QTreeWidget>
#include <QMessageBox>
#include <QDir>

#include <KLocalizedString>
// #include <KNS3/DownloadDialog>
// #include <KNewStuff3/KNS3/DownloadDialog>
// #include <KNS3/DownloadManager>
// #include <KNS3/UploadDialog>

/*!
	\class ImportDatasetWidget
	\brief Widget for importing data from a dataset.

	\ingroup kdefrontend
 */
ImportDatasetWidget::ImportDatasetWidget(QWidget* parent) : QWidget(parent),
	m_categoryCompleter(new QCompleter),
	m_datasetCompleter(new QCompleter) {

	ui.setupUi(this);

	m_jsonDir = QStandardPaths::locate(QStandardPaths::AppDataLocation, QLatin1String("datasets"), QStandardPaths::LocateDirectory);
	loadCategories();

	ui.lwDatasets->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.twCategories->setSelectionMode(QAbstractItemView::SingleSelection);

	const int size = ui.leSearchCategories->height();
	ui.lSearchCategories->setPixmap( QIcon::fromTheme(QLatin1String("go-next")).pixmap(size, size) );
	ui.lSearchDatasets->setPixmap( QIcon::fromTheme(QLatin1String("go-next")).pixmap(size, size) );

	QString info = i18n("Enter the name of the category to navigate to it.");
	ui.lSearchCategories->setToolTip(info);
	ui.leSearchCategories->setToolTip(info);
	ui.leSearchCategories->setPlaceholderText(i18n("Category..."));

	info = i18n("Enter the name of the dataset to navigate to it.");
	ui.lSearchDatasets->setToolTip(info);
	ui.leSearchDatasets->setToolTip(info);
	ui.leSearchDatasets->setPlaceholderText(i18n("Dataset..."));

	connect(ui.cbCollections, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &ImportDatasetWidget::collectionChanged);
	connect(ui.twCategories, &QTreeWidget::itemDoubleClicked, this, &ImportDatasetWidget::listDatasetsForSubcategory);
	connect(ui.twCategories, &QTreeWidget::itemSelectionChanged, [this] {
		if(!m_loadingCategories)
			listDatasetsForSubcategory(ui.twCategories->selectedItems().first());
	});
	connect(ui.leSearchDatasets, &QLineEdit::textChanged, this, &ImportDatasetWidget::scrollToDatasetListItem);
// 	connect(ui.bClearCache, &QPushButton::clicked, this, &ImportDatasetWidget::clearCache);
	connect(ui.leSearchCategories, &QLineEdit::textChanged, this, &ImportDatasetWidget::scrollToCategoryTreeItem);
// 	connect(ui.bRefresh, &QPushButton::clicked, this, &ImportDatasetWidget::refreshCategories);
// 	connect(ui.bRestore, &QPushButton::clicked, this, &ImportDatasetWidget::restoreBackup);
	connect(ui.bNewDataset, &QPushButton::clicked, this, &ImportDatasetWidget::showDatasetMetadataManager);
	connect(ui.lwDatasets, &QListWidget::itemSelectionChanged, [this]() { datasetChanged(); });
	connect(ui.lwDatasets, &QListWidget::doubleClicked, [this]() {emit datasetDoubleClicked(); });

	ui.bRefresh->hide();
	ui.bClearCache->hide();
	ui.bRestore->hide();
	ui.bNewDataset->hide();
}

ImportDatasetWidget::~ImportDatasetWidget() {
	if(m_categoryCompleter != nullptr)
		delete m_categoryCompleter;
	if(m_datasetCompleter != nullptr)
		delete m_datasetCompleter;
}

/**
 * @brief Processes the json metadata file that contains the list of categories and subcategories and their datasets.
 */
void ImportDatasetWidget::loadCategories() {
	m_datasetsMap.clear();
	ui.cbCollections->clear();

	const QString collectionsFileName = m_jsonDir + QLatin1String("/DatasetCollections.json");
	QFile file(collectionsFileName);

	if (file.open(QIODevice::ReadOnly)) {
		QJsonDocument document = QJsonDocument::fromJson(file.readAll());
		file.close();
		if (!document.isArray()) {
			QDEBUG("Invalid definition of " + collectionsFileName);
			return;
		}

		m_collections = document.array();

		for (int col = 0; col < m_collections.size(); col++) {
			const QJsonObject& collection = m_collections[col].toObject();
			const QString& collectionName = collection["name"].toString();

			QString path = m_jsonDir + QLatin1Char('/') + collectionName + ".json";
			QFile collectionFile(path);
			if (collectionFile.open(QIODevice::ReadOnly)) {
				QJsonDocument collectionDocument = QJsonDocument::fromJson(collectionFile.readAll());
				if (!collectionDocument.isObject()) {
					QDEBUG("Invalid definition of " + path);
					continue;
				}

				QJsonObject collectionObject = collectionDocument.object();
				QJsonArray categoryArray = collectionObject.value("categories").toArray();

				//processing categories
				for(int i = 0 ; i < categoryArray.size(); ++i) {
					const QJsonObject& currentCategory = categoryArray[i].toObject();
					const QString& categoryName = currentCategory.value("category_name").toString();
					const QJsonArray& subcategories = currentCategory.value("subcategories").toArray();

					//processing subcategories
					for(int j = 0; j < subcategories.size(); ++j) {
						QJsonObject currentSubCategory = subcategories[j].toObject();
						QString subcategoryName = currentSubCategory.value("subcategory_name").toString();
						const QJsonArray& datasetArray = currentSubCategory.value("datasets").toArray();

						//processing the datasets of the actual subcategory
						for (const auto& dataset : datasetArray)
							m_datasetsMap[collectionName][categoryName][subcategoryName].push_back(dataset.toObject().value("filename").toString());
					}
				}
			}
		}

		if(m_datasetModel)
			delete m_datasetModel;
		m_datasetModel = new DatasetModel(m_datasetsMap);

		//Fill up collections combo box
		ui.cbCollections->addItem(i18n("All") + QString(" (" + QString::number(m_datasetModel->allDatasetsList().toStringList().size()) + ")"), QLatin1String("All"));
		for (QString collection : m_datasetModel->collections())
			ui.cbCollections->addItem(collection + " (" + QString::number(m_datasetModel->datasetCount(collection)) + ")", collection);

		collectionChanged(ui.cbCollections->currentIndex());
	} else
		QMessageBox::critical(this, i18n("File not found"),
							  i18n("Couldn't open the dataset collections file %1. Please check your installation.", collectionsFileName));
}

/**
 * Shows all categories and sub-categories for the currently selected collection
 */
void ImportDatasetWidget::collectionChanged(int index) {
	bool allCollections = (index == 0);
	QString collectionName = ui.cbCollections->itemData(index).toString();

	//update the info field
	QString info;
	if (!allCollections) {
		for (int i = 0; i < m_collections.size(); ++i) {
			const QJsonObject& collection = m_collections[i].toObject();
			if ( collectionName == collection["name"].toString() ) {
				info += collection["description"].toString();
				info += "<br><br></hline><br><br>";
				break;
			}
		}
	}
	ui.lInfo->setText(info);

	m_loadingCategories = true;
	ui.lwDatasets->clear();
	ui.twCategories->clear();
	QStringList categories = allCollections
							? m_datasetModel->allCategories().toStringList()
							: m_datasetModel->categories(collectionName);

	//Go through every category that was previously processed.
	for(auto category : categories) {
		QStringList categoryList(category);
		categoryList.append(QString::number(m_datasetModel->datasetCount(collectionName, category)));
		QTreeWidgetItem* const currentCategoryItem = new QTreeWidgetItem(categoryList);
		ui.twCategories->addTopLevelItem(currentCategoryItem);

		QStringList subcategories = allCollections
									? m_datasetModel->allSubcategories(category).toStringList()
									: m_datasetModel->subcategories(collectionName, category);

		//Go through every subcategory of the current category, that was previously processed.
		for(auto subcategory : subcategories) {
			QStringList subcategoryList(subcategory);
			subcategoryList.append(QString::number(m_datasetModel->datasetCount(collectionName, category, subcategory)));

			currentCategoryItem->addChild(new QTreeWidgetItem(QStringList(subcategoryList)));
		}
	}

	if(m_selectedCollection == collectionName) {
		restoreSelectedSubcategory(collectionName);
	} else {
		m_selectedCollection = collectionName;
		m_selectedCategory = "";
		m_selectedSubcategory = "";
	}

	m_loadingCategories = false;
	updateCategoryCompleter();
}

/**
 * @brief Restores the lastly selected collection, category and subcategory making it the selected QTreeWidgetItem and also lists the datasets belonigng to it
 */
void ImportDatasetWidget::restoreSelectedSubcategory(const QString& collectionName) {
	if(m_datasetModel->categories(collectionName).contains(m_selectedCategory)) {
		const QTreeWidgetItem* const categoryItem = ui.twCategories->findItems(m_selectedCategory, Qt::MatchExactly).first();

		if(m_datasetModel->subcategories(collectionName, m_selectedCategory).contains(m_selectedSubcategory)) {
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
			for(QString dataset : m_datasetModel->datasets(m_selectedCollection, categoryName, m_selectedSubcategory))
				ui.lwDatasets->addItem(new QListWidgetItem(dataset));

			updateDatasetCompleter();
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
			QString text = ui.twCategories->topLevelItem(i)->text(0) + QLatin1Char(':')
						+ ui.twCategories->topLevelItem(i)->child(j)->text(0);
			categoryList.append(text);
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
		if (!rootName.contains(QLatin1Char(':'))) {
			ui.twCategories->scrollToItem(ui.twCategories->topLevelItem(topItemIdx),
										  QAbstractItemView::ScrollHint::PositionAtTop);
		} else {
			int childIdx = -1;
			for (int j = 0; j < ui.twCategories->topLevelItem(topItemIdx)->childCount(); ++j) {
				if(rootName.endsWith(ui.twCategories->topLevelItem(topItemIdx)->child(j)->text(0))) {
					childIdx = j;
					break;
				}
			}

			if (childIdx >= 0)
				ui.twCategories->scrollToItem(ui.twCategories->topLevelItem(topItemIdx)->child(childIdx),
											  QAbstractItemView::ScrollHint::PositionAtTop);
			else
				ui.twCategories->scrollToItem(ui.twCategories->topLevelItem(topItemIdx),
											  QAbstractItemView::ScrollHint::PositionAtTop);
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
	if (ui.lwDatasets->selectedItems().count() > 0)
		return ui.lwDatasets->selectedItems().at(0)->text();
	else
		return QString();
}

/**
 * @brief Initiates the processing of the dataset's metadata file and of the dataset itself.
 * @param datasetHandler the DatasetHanlder that downloads processes the dataset
 */
void ImportDatasetWidget::loadDatasetToProcess(DatasetHandler* datasetHandler) {
	datasetHandler->processMetadata(m_datasetObject);
}

/**
 * @brief Returns the QJsonObject associated with the currently selected dataset.
 */
QJsonObject ImportDatasetWidget::loadDatasetObject() {
	bool allCollections = (ui.cbCollections->currentIndex() == 0);

	for (int i = 0; i < m_collections.size(); ++i) {
		const QJsonObject& collection = m_collections[i].toObject();
		const QString& collectionName = collection["name"].toString();

		//we have to find the selected collection in the metadata file.
		if(allCollections || collectionName == m_selectedCollection) {
			QFile file(m_jsonDir + QLatin1Char('/') + collectionName + ".json");

			//open the metadata file of the current collection
			if (file.open(QIODevice::ReadOnly)) {
				QJsonDocument collectionDocument = QJsonDocument::fromJson(file.readAll());
				file.close();
				if(!collectionDocument.isObject()) {
					qDebug()<< "The " +  collectionName + ".json file is invalid";
					return QJsonObject();
				}

				QJsonObject collectionObject = collectionDocument.object();
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

			if (!allCollections)
				break;
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

		//update the metadata document
		dlg->updateDocument(m_jsonDir);

		//Not working due to problems with KNS3 library
		/*uploadCategoryFile();
		uploadDatasetFile(dlg->getMetadataFilePath());*/

		//process the changes made in the metadata files
		loadCategories();
	}
	delete dlg;
}

/**
 * @brief Places the metadata file containing the list of collections into a specific directory.
 */
/*
void ImportDatasetWidget::downloadCollectionsFile() {
	const QString fileNameOld = QStandardPaths::locate(QStandardPaths::AppDataLocation, "datasets/DatasetCollections.json");
	const QString fileNameNew =m_jsonDir + QLatin1String("DatasetCollections.json");
	const QString parentDir  = m_jsonDir.left(m_jsonDir.left(m_jsonDir.length() - 1).lastIndexOf(QDir::separator()));

	if(!QDir(m_jsonDir).exists()) {
		qDebug() << parentDir;
		QDir(parentDir).mkdir(QLatin1String("labplot_data"));
	}

	QFile::copy(fileNameOld, fileNameNew);
}
*/

/**
 * @brief Places the metadata file of the given dataset into a specific directory.
 * @param datasetName the name of the dataset
 */
/*
void ImportDatasetWidget::downloadCollectionFile(const QString& collectionName) {
	const QString fileNameOld = QStandardPaths::locate(QStandardPaths::AppDataLocation, QLatin1String("datasets") + QDir::separator() + collectionName);
	const QString fileNameNew =m_jsonDir + collectionName;

	QFile::copy(fileNameOld, fileNameNew);
}
*/

/**
 * @brief Refreshes the categories, subcategories and datasets.
 */
/*
void ImportDatasetWidget::refreshCategories() {
	QMessageBox::StandardButton reply;
	reply = QMessageBox::question(this, "Refresh metadata files",
								  "Are you sure to refresh all of the metadata files? (every change will be removed, but a backup will be created)", QMessageBox::Yes|QMessageBox::No);

	if(reply == QMessageBox::Yes) {
		QString fileNameNew = m_jsonDir + QLatin1String("DatasetCollections.json");

		QFile existingCategoriesFile(fileNameNew);
		if(existingCategoriesFile.exists()) {

			//Delete old backup
			QFile oldBackup(m_jsonDir + QLatin1String("DatasetCollections_backup.json"));
			if(oldBackup.exists()) {
				oldBackup.remove();
			}
			oldBackup.close();

			//Create new backup
			if(!existingCategoriesFile.rename(m_jsonDir + QLatin1String("DatasetCollections_backup.json")))
				qDebug() << " Couldn't create backup because " << existingCategoriesFile.errorString();
		}

		//Obtain the new file
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

			//Go trough every collection's metadata file
			for (int collectionIndex = 0; collectionIndex < collections.size(); collectionIndex++) {
				const QString currentCollection = collections[collectionIndex].toString();

				QFile existingCollectionFile(m_jsonDir + currentCollection + ".json");
				//we copy the file to the data location if it doesn't exist
				if(!existingCollectionFile.exists()) {
					downloadCollectionFile(currentCollection + ".json");
				}
				//otherwise we have to create a backup first
				else {
					QFile oldBackupCollection(m_jsonDir + currentCollection + "_backup.json");
					if(oldBackupCollection.exists()) {
						oldBackupCollection.remove();
					}
					oldBackupCollection.close();

					if(!existingCollectionFile.rename(m_jsonDir + currentCollection + "_backup.json"))
						qDebug() << " Couldn't create backup because " << existingCollectionFile.errorString();
				}
			}
		}

		//process the "refreshed" files and update the widget accordingly
		loadCategories();
	}
}
*/

/**
 * @brief Restores the saved metadata files.  Revokes the effect of refreshCategories().
 */
/*
void ImportDatasetWidget::restoreBackup() {
	QMessageBox::StandardButton reply;
	reply = QMessageBox::question(this, "Restore backup", "Are you sure to restore the backup metadata files?", QMessageBox::Yes|QMessageBox::No);

	if(reply == QMessageBox::Yes) {
		//Restore the collection list first
		QFile backup(m_jsonDir + QLatin1String("DatasetCollections_backup.json"));
		if(backup.exists()) {
			QFile deleteFile(m_jsonDir + QLatin1String("DatasetCollections.json"));
			deleteFile.remove();

			if(!backup.rename(m_jsonDir + QLatin1String("DatasetCollections.json"))) {
				qDebug() << " Couldn't create backup because " << backup.errorString();
				downloadCollectionsFile();
			}
		}

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

			//Restore every collection's metadata file
			for (int collectionIndex = 0; collectionIndex < collections.size(); collectionIndex++) {
				const QString currentCollection = collections[collectionIndex].toString();

				QFile backupCollection(m_jsonDir + currentCollection + "_backup.json");
				if(backupCollection.exists()) {
					QFile collectionFile(m_jsonDir + currentCollection + ".json");
					collectionFile.remove();

					if(!backupCollection.rename(m_jsonDir + currentCollection + ".json")) {
						qDebug() << " Couldn't create backup because " << backupCollection.errorString();
						downloadCollectionFile(currentCollection + ".json");
					}
				}
			}
		}

		//process the restored files and update the widget accordingly
		loadCategories();
	}
}
*/

/**
 * @brief Clears the content of the directory in which the download of metadata files was done.
 */
/*
void ImportDatasetWidget::clearCache() {
	QMessageBox::StandardButton reply = QMessageBox::question(this, "Clear cache",
															  "Are you sure to remove every downloaded dataset?",
															QMessageBox::Yes|QMessageBox::No);

	if(reply == QMessageBox::Yes) {
		QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1String("/datasets_local/"));

		if(dir.exists()) {
			for(const auto& entry : dir.entryList()) {
				//delete every file that isn't potentially a metadata file
				if(!(entry.endsWith(QLatin1String(".json")) || entry.startsWith(QLatin1Char('.')))) {
					QFile deleteFile (dir.path() + QLatin1Char('/') + entry);
					if(deleteFile.exists()) {
						deleteFile.remove();
					}
				}
			}
		} else {
			qDebug("Couldn't clear cache, containing folder doesn't exist!");
		}
	}
}
*/

/**
 * @brief TODO: uploads the metadata file that contains the categories to store.kde.org -- Library doesn't work for indefinite time.
 */
/*
void ImportDatasetWidget::uploadCategoryFile() {
	KNS3::UploadDialog dialog("labplot2_datasets.knsrc", this);

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

	dialog.exec();
}
*/

/**
 * @brief TODO: uploads the metadata file of a dataset to store.kde.org -- Library doesn't work for indefinite time.
 */
/*
void ImportDatasetWidget::uploadDatasetFile(const QString& filePath) {
	KNS3::UploadDialog dialog("labplot2_datasets.knsrc", this);

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

	dialog.exec();
}
*/

/**
 * @brief Returns the structure containing the categories, subcategories and datasets.
 * @return the structure containing the categories, subcategories and datasets
 */
const DatasetsMap& ImportDatasetWidget::getDatasetsMap() {
	return m_datasetsMap;
}

/**
 * @brief Sets the currently selected collection
 * @param category the name of the collection
 */
void ImportDatasetWidget::setCollection(const QString& collection) {
	ui.cbCollections->setCurrentText(collection + " (" + QString(m_datasetModel->datasetCount(collection)) + ")");
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

/**
 * @brief Updates the details of the currently selected dataset
 */
void ImportDatasetWidget::datasetChanged() {
	QString info;
	if (ui.cbCollections->currentIndex() != 0) {
		const QString& collectionName = ui.cbCollections->itemData(ui.cbCollections->currentIndex()).toString();
		for (int i = 0; i < m_collections.size(); ++i) {
			const QJsonObject& collection = m_collections[i].toObject();
			if ( collectionName.startsWith(collection["name"].toString()) ) {
				info += collection["description"].toString();
				info += "<br><br>";
				break;
			}
		}
	}

	if(!getSelectedDataset().isEmpty()) {
		m_datasetObject = loadDatasetObject();

		info += "<b>" + i18n("Dataset") + ":</b><br>";
		info += m_datasetObject["name"].toString();
		info += "<br><br>";
		info += "<b>" + i18n("Description") + ":</b><br>";
		info += m_datasetObject["description"].toString();
	} else
		m_datasetObject = QJsonObject();

	ui.lInfo->setText(info);
	emit datasetSelected();
}
