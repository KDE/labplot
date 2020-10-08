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
#include "kdefrontend/DatasetModel.h"

#include <QCompleter>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonValue>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStandardPaths>

#include <KConfigGroup>
#include <KSharedConfig>
#include <KLocalizedString>

/*!
	\class ImportDatasetWidget
	\brief Widget for importing data from a dataset.

	\ingroup kdefrontend
 */
ImportDatasetWidget::ImportDatasetWidget(QWidget* parent) : QWidget(parent),
	m_networkManager(new QNetworkAccessManager(this)) {

	ui.setupUi(this);

	m_jsonDir = QStandardPaths::locate(QStandardPaths::AppDataLocation, QLatin1String("datasets"), QStandardPaths::LocateDirectory);
	loadCategories();

	ui.lwDatasets->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.twCategories->setSelectionMode(QAbstractItemView::SingleSelection);

	const int size = ui.leSearch->height();
	ui.lSearch->setPixmap( QIcon::fromTheme(QLatin1String("edit-find")).pixmap(size, size) );

	QString info = i18n("Enter the keyword you want to search for");
	ui.lSearch->setToolTip(info);
	ui.leSearch->setToolTip(info);
	ui.leSearch->setPlaceholderText(i18n("Search..."));
	ui.leSearch->setFocus();

	connect(ui.cbCollections, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &ImportDatasetWidget::collectionChanged);
	connect(ui.twCategories, &QTreeWidget::itemDoubleClicked, this, &ImportDatasetWidget::updateDatasets);
	connect(ui.twCategories, &QTreeWidget::itemSelectionChanged, [this] {
		if (!m_initializing)
			updateDatasets(ui.twCategories->selectedItems().first());
	});

	connect(ui.leSearch, &QLineEdit::textChanged, this, &ImportDatasetWidget::updateCategories);
	connect(ui.lwDatasets, &QListWidget::itemSelectionChanged, [this]() {
		if (!m_initializing)
			datasetChanged();
	});
	connect(ui.lwDatasets, &QListWidget::doubleClicked, [this]() {emit datasetDoubleClicked(); });
	connect(m_networkManager, &QNetworkAccessManager::finished, this, &ImportDatasetWidget::downloadFinished);

	//select the last used collection
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportDatasetWidget");
	const QString& collection = conf.readEntry("Collection", QString());
	if (collection.isEmpty())
		ui.cbCollections->setCurrentIndex(0);
	else {
		for (int i = 0; i < ui.cbCollections->count(); ++i) {
			if (ui.cbCollections->itemData(i).toString() == collection) {
				ui.cbCollections->setCurrentIndex(i);
				break;
			}
		}
	}
}

ImportDatasetWidget::~ImportDatasetWidget() {
	delete m_model;

	//save the selected collection
	if (ui.cbCollections->currentIndex() != -1) {
		KConfigGroup conf(KSharedConfig::openConfig(), "ImportDatasetWidget");
		conf.writeEntry("Collection", ui.cbCollections->itemData(ui.cbCollections->currentIndex()).toString());
	}
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

		for (const auto& col : m_collections) {
			const QJsonObject& collection = col.toObject();
			const QString& m_collection = collection[QLatin1String("name")].toString();

			QString path = m_jsonDir + QLatin1Char('/') + m_collection + ".json";
			QFile collectionFile(path);
			if (collectionFile.open(QIODevice::ReadOnly)) {
				QJsonDocument collectionDocument = QJsonDocument::fromJson(collectionFile.readAll());
				if (!collectionDocument.isObject()) {
					QDEBUG("Invalid definition of " + path);
					continue;
				}

				const QJsonObject& collectionObject = collectionDocument.object();
				const QJsonArray& categoryArray = collectionObject.value(QLatin1String("categories")).toArray();

				//processing categories
				for (const auto& cat : categoryArray) {
					const QJsonObject& currentCategory = cat.toObject();
					const QString& categoryName = currentCategory.value(QLatin1String("name")).toString();
					const QJsonArray& subcategories = currentCategory.value(QLatin1String("subcategories")).toArray();

					//processing subcategories
					for (const auto& sub : subcategories) {
						QJsonObject currentSubCategory = sub.toObject();
						QString subcategoryName = currentSubCategory.value(QLatin1String("name")).toString();
						const QJsonArray& datasetArray = currentSubCategory.value(QLatin1String("datasets")).toArray();

						//processing the datasets of the actual subcategory
						for (const auto& dataset : datasetArray)
							m_datasetsMap[m_collection][categoryName][subcategoryName].push_back(dataset.toObject().value(QLatin1String("filename")).toString());
					}
				}
			}
		}

		if (m_model)
			delete m_model;
		m_model = new DatasetModel(m_datasetsMap);

		//Fill up collections combo box
		ui.cbCollections->addItem(i18n("All") + QLatin1String(" (") + QString::number(m_model->allDatasetsList().toStringList().size()) + QLatin1Char(')'));
		for (const QString& collection : m_model->collections())
			ui.cbCollections->addItem(collection + QLatin1String(" (") + QString::number(m_model->datasetCount(collection)) + QLatin1Char(')'), collection);

		collectionChanged(ui.cbCollections->currentIndex());
	} else
		QMessageBox::critical(this, i18n("File not found"),
							  i18n("Couldn't open the dataset collections file %1. Please check your installation.", collectionsFileName));
}

/**
 * Shows all categories and sub-categories for the currently selected collection
 */
void ImportDatasetWidget::collectionChanged(int index) {
	m_allCollections = (index == 0);

	if (!m_allCollections)
		m_collection = ui.cbCollections->itemData(index).toString();
	else
		m_collection = "";

	//update the info field
	QString info;
	if (!m_allCollections) {
		for (const auto& col : m_collections) {
			const QJsonObject& collection = col.toObject();
			if ( m_collection == collection[QLatin1String("name")].toString() ) {
				info += collection[QLatin1String("description")].toString();
				info += QLatin1String("<br><br></hline><br><br>");
				break;
			}
		}
	} else {
		for (const auto& col : m_collections) {
			const QJsonObject& collection = col.toObject();
			info += collection[QLatin1String("description")].toString();
			info += QLatin1String("<br><br>");
		}
	}
	ui.lInfo->setText(info);
	updateCategories();

	//update the completer
	if (m_completer)
		delete m_completer;

	//add all categories, sub-categories and the dataset names for the current collection
	QStringList keywords;
	for (const auto& category : m_model->categories(m_collection)) {
		keywords << category;
		for (const auto& subcategory : m_model->subcategories(m_collection, category)) {
			keywords << subcategory;
			for (const QString& dataset : m_model->datasets(m_collection, category, subcategory))
				keywords << dataset;
		}
	}

	m_completer = new QCompleter(keywords, this);
	m_completer->setCompletionMode(QCompleter::PopupCompletion);
	m_completer->setCaseSensitivity(Qt::CaseSensitive);
	ui.leSearch->setCompleter(m_completer);
}

void ImportDatasetWidget::updateCategories() {
	m_initializing = true;
	ui.twCategories->clear();

	auto* rootItem = new QTreeWidgetItem(QStringList(i18n("All")));
	ui.twCategories->addTopLevelItem(rootItem);

	const QString& filter = ui.leSearch->text();

	//add categories
	for (const auto& category : m_model->categories(m_collection)) {
		const bool categoryMatch = (filter.isEmpty() || category.startsWith(filter, Qt::CaseInsensitive));

		if (categoryMatch) {
			auto* const item = new QTreeWidgetItem(QStringList(category));
			rootItem->addChild(item);

			//add all sub-categories
			for (const auto& subcategory : m_model->subcategories(m_collection, category))
				item->addChild(new QTreeWidgetItem(QStringList(subcategory)));
		} else {
			QTreeWidgetItem* item = nullptr;
			for (const auto& subcategory : m_model->subcategories(m_collection, category)) {
				bool subcategoryMatch = subcategory.startsWith(filter, Qt::CaseInsensitive);

				if (subcategoryMatch) {
					if (!item) {
						item = new QTreeWidgetItem(QStringList(category));
						rootItem->addChild(item);
						item->setExpanded(true);
					}
					item->addChild(new QTreeWidgetItem(QStringList(subcategory)));
				} else {
					for (const QString& dataset : m_model->datasets(m_collection, category, subcategory)) {
						bool datasetMatch = dataset.startsWith(filter, Qt::CaseInsensitive);
						if (datasetMatch) {
							if (!item) {
								item = new QTreeWidgetItem(QStringList(category));
								rootItem->addChild(item);
								item->setExpanded(true);
							}
							item->addChild(new QTreeWidgetItem(QStringList(subcategory)));
							break;
						}
					}
				}
			}
		}
	}

	//remote the root item "All" if nothing has matched to the filter string
	if (rootItem->childCount() == 0)
		ui.twCategories->clear();


	//expand the root item and select the first category item
	rootItem->setExpanded(true);
	if (filter.isEmpty()) {
		rootItem->setSelected(true);
		updateDatasets(rootItem);
	} else {
		if (rootItem->child(0) && rootItem->child(0)->child(0)) {
			rootItem->child(0)->child(0)->setSelected(true);
			updateDatasets(rootItem->child(0)->child(0));
		} else
			updateDatasets(nullptr);
	}

	m_initializing = false;
}

/**
 * @brief Populates lwDatasets with the datasets of the selected subcategory or its parent
 * @param item the selected subcategory
 */
void ImportDatasetWidget::updateDatasets(QTreeWidgetItem* item) {
	m_initializing = true;
	ui.lwDatasets->clear();

	if (!item) {
		//no category item is selected because nothing matches the search string
		m_initializing = false;
		datasetChanged();
		return;
	}

	const QString& filter = ui.leSearch->text();

	if (item->childCount() == 0) {
		//sub-category was selected -> show all its datasets
		m_category = item->parent()->text(0);
		m_subcategory = item->text(0);

		addDatasetItems(m_collection, m_category, m_subcategory, filter);
	} else {
		if (!item->parent()) {
			//top-level item "All" was selected -> show datasets for all categories and their sub-categories
			m_category = "";
			m_subcategory = "";

			for (const auto& category : m_model->categories(m_collection)) {
				for (const auto& subcategory : m_model->subcategories(m_collection, category))
					addDatasetItems(m_collection, category, subcategory, filter);
			}
		} else {
			//a category was selected -> show all its datasets
			m_category = item->text(0);
			m_subcategory = "";

			for (const auto& subcategory : m_model->subcategories(m_collection, m_category))
				addDatasetItems(m_collection, m_category, subcategory, filter);
		}
	}

	m_initializing = false;

	//select the first available dataset
	if (ui.lwDatasets->count())
		ui.lwDatasets->setCurrentRow(0);
}

void ImportDatasetWidget::addDatasetItems(const QString& collection, const QString& category, const QString& subcategory, const QString& filter) {
	if (!filter.isEmpty() &&
		(category.startsWith(filter, Qt::CaseInsensitive) || subcategory.startsWith(filter, Qt::CaseInsensitive))) {

		for (const QString& dataset : m_model->datasets(collection, category, subcategory))
			ui.lwDatasets->addItem(new QListWidgetItem(dataset));
	} else {
		for (const QString& dataset : m_model->datasets(collection, category, subcategory)) {
			if (filter.isEmpty() || dataset.startsWith(filter, Qt::CaseInsensitive))
				ui.lwDatasets->addItem(new QListWidgetItem(dataset));
		}
	}
}

/**
 * @brief Returns the name of the selected dataset
 */
QString ImportDatasetWidget::getSelectedDataset() const {
	if (!ui.lwDatasets->selectedItems().isEmpty())
		return ui.lwDatasets->selectedItems().at(0)->text();
	else
		return QString();
}

/**
 * @brief Initiates the processing of the dataset's metadata file and of the dataset itself.
 * @param datasetHandler the DatasetHanlder that downloads processes the dataset
 */
void ImportDatasetWidget::import(DatasetHandler* datasetHandler) {
	datasetHandler->processMetadata(m_datasetObject);
}

/**
 * @brief Returns the QJsonObject associated with the currently selected dataset.
 */
QJsonObject ImportDatasetWidget::loadDatasetObject() {
	for (const auto& col : m_collections) {
		const QJsonObject& collectionJson = col.toObject();
		const QString& collection = collectionJson[QLatin1String("name")].toString();

		//we have to find the selected collection in the metadata file.
		if (m_allCollections || m_collection == collection) {
			QFile file(m_jsonDir + QLatin1Char('/') + collection + QLatin1String(".json"));

			//open the metadata file of the current collection
			if (file.open(QIODevice::ReadOnly)) {
				QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
				file.close();
				if (!doc.isObject()) {
					DEBUG("The " <<  STDSTRING(collection) << ".json file is invalid");
					return QJsonObject();
				}

				QJsonArray categoryArray = doc.object().value(QLatin1String("categories")).toArray();

				//processing categories
				for (const auto& cat : categoryArray) {
					const QJsonObject currentCategory = cat.toObject();
					const QString categoryName = currentCategory.value(QLatin1String("name")).toString();
					if (m_category.isEmpty() || categoryName.compare(m_category) == 0) {
						const QJsonArray subcategories = currentCategory.value(QLatin1String("subcategories")).toArray();

						//processing subcategories
						for (const auto& sub : subcategories) {
							QJsonObject currentSubCategory = sub.toObject();
							QString subcategoryName = currentSubCategory.value(QLatin1String("name")).toString();

							if (m_subcategory.isEmpty() || subcategoryName.compare(m_subcategory) == 0) {
								const QJsonArray datasetArray = currentSubCategory.value(QLatin1String("datasets")).toArray();

								//processing the datasets of the actual subcategory
								for (const auto& dataset : datasetArray) {
									if (getSelectedDataset().compare(dataset.toObject().value(QLatin1String("filename")).toString()) == 0)
										return dataset.toObject();
								}

								if (!m_subcategory.isEmpty())
									break;
							}
						}

						if (!m_category.isEmpty())
							break;
					}
				}
			}

			if (!m_allCollections)
				break;
		}
	}

	return QJsonObject();
}

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
	ui.cbCollections->setCurrentText(collection + QLatin1String(" (")
				+ QString(m_model->datasetCount(collection)) + QLatin1Char(')'));
}

/**
 * @brief Sets the currently selected category
 * @param category the name of the category
 */
void ImportDatasetWidget::setCategory(const QString &category) {
	for (int i = 0; i < ui.twCategories->topLevelItemCount(); i++) {
		if (ui.twCategories->topLevelItem(i)->text(0).compare(category) == 0) {
			updateDatasets(ui.twCategories->topLevelItem(i));
			break;
		}
	}
}

/**
 * @brief Sets the currently selected subcategory
 * @param subcategory the name of the subcategory
 */
void ImportDatasetWidget::setSubcategory(const QString &subcategory) {
	for (int i = 0; i < ui.twCategories->topLevelItemCount(); i++) {
		if (ui.twCategories->topLevelItem(i)->text(0).compare(m_category) == 0) {
			QTreeWidgetItem* categoryItem = ui.twCategories->topLevelItem(i);
			for (int j = 0; j <categoryItem->childCount(); j++) {
				if (categoryItem->child(j)->text(0).compare(subcategory) == 0) {
					updateDatasets(categoryItem->child(j));
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
	for (int i = 0; i < ui.lwDatasets->count() ; i++) {
		if (ui.lwDatasets->item(i)->text().compare(datasetName) == 0) {
			ui.lwDatasets->item(i)->setSelected(true);
			break;
		}
	}
}

/**
 * @brief Updates the details of the currently selected dataset
 */
void ImportDatasetWidget::datasetChanged() {
	QString dataset = getSelectedDataset();

	//no need to fetch the same dataset description again if it's already shown
	if (m_collection == m_prevCollection && m_category == m_prevCategory
		&& m_subcategory == m_prevSubcategory && dataset == m_prevDataset)
		return;

	m_prevCollection = m_collection;
	m_prevCategory = m_category;
	m_prevSubcategory = m_subcategory;
	m_prevDataset = dataset;

	QString info;
	if (ui.cbCollections->currentIndex() != 0) {
		const QString& m_collection = ui.cbCollections->itemData(ui.cbCollections->currentIndex()).toString();
		for (const auto& col : m_collections) {
			const QJsonObject& collection = col.toObject();
			if ( m_collection.startsWith(collection[QLatin1String("name")].toString()) ) {
				info += collection[QLatin1String("description")].toString();
				info += QLatin1String("<br><br>");
				break;
			}
		}
	}

	if (!dataset.isEmpty()) {
		m_datasetObject = loadDatasetObject();

		info += QLatin1String("<b>") + i18n("Dataset") + QLatin1String(":</b><br>");
		info += m_datasetObject[QLatin1String("name")].toString();
		info += QLatin1String("<br><br>");

		if (m_datasetObject.contains(QLatin1String("description_url"))) {
			WAIT_CURSOR;
			if (m_networkManager->networkAccessible() == QNetworkAccessManager::Accessible)
				m_networkManager->get(QNetworkRequest(QUrl(m_datasetObject[QLatin1String("description_url")].toString())));
			else
				info += m_datasetObject[QLatin1String("description")].toString();
		} else {
			info += QLatin1String("<b>") + i18n("Description") + QLatin1String(":</b><br>");
			info += m_datasetObject[QLatin1String("description")].toString();
		}
	} else
		m_datasetObject = QJsonObject();

	ui.lInfo->setText(info);
	emit datasetSelected();
}

void ImportDatasetWidget::downloadFinished(QNetworkReply* reply) {
	if (reply->error() == QNetworkReply::NoError) {
		QByteArray ba = reply->readAll();
		QString info(ba);

		if (m_collection == QLatin1String("Rdatasets")) {
			//detailed descriptions for R is in html format,
			//remove the header from the html file since we construct our own header

			int headerStart = info.indexOf(QLatin1String("<head>"));
			int headerEnd = info.indexOf(QLatin1String("</head>"));
			info = info.left(headerStart) + info.right(info.length() - headerEnd - 7);

			headerStart = info.indexOf(QLatin1String("<table"));
			headerEnd = info.indexOf(QLatin1String("</table>"));
			info = info.left(headerStart) + info.right(info.length() - headerEnd - 8);

			headerStart = info.indexOf(QLatin1String("<h2>"));
			headerEnd = info.indexOf(QLatin1String("</h2>"));
			info = info.left(headerStart) + info.right(info.length() - headerEnd - 5);

			info = info.replace(QLatin1String("<body>\n\n\n\n\n\n"), QLatin1String("<body>"));
			info = info.remove(QLatin1String("\n\n\n"));
		} else
			info = info.replace(QLatin1Char('\n'), QLatin1String("<br>"));

		//do further collection specific replacements to get better formatting
		if (m_collection == QLatin1String("JSEDataArchive")) {
			info = info.replace(QLatin1String("NAME:"), QLatin1String("<b>NAME:</b>"), Qt::CaseInsensitive);
			info = info.replace(QLatin1String("TYPE:"), QLatin1String("<b>TYPE:</b>"), Qt::CaseSensitive);
			info = info.replace(QLatin1String("SIZE:"), QLatin1String("<b>SIZE:</b>"), Qt::CaseSensitive);
			info = info.replace(QLatin1String("DESCRIPTIVE ABSTRACT:"), QLatin1String("<b>DESCRIPTIVE ABSTRACT:</b>"), Qt::CaseInsensitive);
			info = info.replace(QLatin1String("NOTE:"), QLatin1String("<b>NOTE:</b>"), Qt::CaseSensitive);
			info = info.replace(QLatin1String("SPECIAL NOTES:"), QLatin1String("<b>SPECIAL NOTES:</b>"), Qt::CaseSensitive);
			info = info.replace(QLatin1String("SOURCE:"), QLatin1String("<b>SOURCE:</b>"), Qt::CaseSensitive);
			info = info.replace(QLatin1String("SOURCES:"), QLatin1String("<b>SOURCES:</b>"), Qt::CaseInsensitive);
			info = info.replace(QLatin1String("DATA <b>SOURCE:</b>"), QLatin1String("<b>DATA SOURCE:</b>"), Qt::CaseSensitive);
			info = info.replace(QLatin1String("DATASET LAYOUT:"), QLatin1String("<b>DATASET LAYOUT:</b>"), Qt::CaseSensitive);
			info = info.replace(QLatin1String("DATASETS LAYOUT:"), QLatin1String("<b>DATASETS LAYOUT:</b>"), Qt::CaseSensitive);
			info = info.replace(QLatin1String("VARIABLE DESCRIPTIONS:"), QLatin1String("<b>VARIABLE DESCRIPTIONS:</b>"), Qt::CaseSensitive);
			info = info.replace(QLatin1String("VARIABLES DESCRIPTIONS:"), QLatin1String("<b>VARIABLES DESCRIPTIONS:</b>"), Qt::CaseSensitive);
			info = info.replace(QLatin1String("RELATED DATASETS:"), QLatin1String("<b>RELATED DATASETS:</b>"), Qt::CaseSensitive);
			info = info.replace(QLatin1String("SPECIAL NOTES:"), QLatin1String("<b>SPECIAL NOTES:</b>"), Qt::CaseSensitive);
			info = info.replace(QLatin1String("STORY BEHIND THE DATA:"), QLatin1String("<b>STORY BEHIND THE DATA:</b>"), Qt::CaseSensitive);
			info = info.replace(QLatin1String("THE <b>STORY BEHIND THE DATA:</b>"), QLatin1String("<b>THE STORY BEHIND THE DATA:</b>"), Qt::CaseSensitive);
			info = info.replace(QLatin1String("PEDAGOGICAL NOTES:"), QLatin1String("<b>PEDAGOGICAL NOTES:</b>"), Qt::CaseSensitive);
			info = info.replace(QLatin1String("REFERENCE:"), QLatin1String("<b>REFERENCE:</b>"), Qt::CaseSensitive);
			info = info.replace(QLatin1String("REFERENCES:"), QLatin1String("<b>REFERENCES:</b>"), Qt::CaseSensitive);
			info = info.replace(QLatin1String("SUBMITTED BY:"), QLatin1String("<b>SUBMITTED BY:</b>"), Qt::CaseSensitive);
		}
		ui.lInfo->setText(ui.lInfo->text() + info);
	} else {
		DEBUG("Failed to fetch the description.");
		ui.lInfo->setText(ui.lInfo->text() + m_datasetObject[QLatin1String("description")].toString());
	}
	reply->deleteLater();
	RESET_CURSOR;
}
