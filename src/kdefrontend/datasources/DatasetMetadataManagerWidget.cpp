/*
File                 : DatasetMetadataManagerWidget.cpp
Project              : LabPlot
Description          : widget for managing a metadata file of a dataset
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2019 Ferencz Kovacs <kferike98@gmail.com>
SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "backend/datasources/filters/AsciiFilter.h"
#include "src/kdefrontend/DatasetModel.h"
#include "src/kdefrontend/datasources/DatasetMetadataManagerWidget.h"

#include <KConfigGroup>
#include <KSharedConfig>
#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QStringList>
#include <QMap>
#include <QMapIterator>
#include <QRegularExpression>
#include <QTcpSocket>
#include <QUrl>

/*!
	\class DatasetMetadataManagerWidget
	\brief Widget for adding a new dataset to LabPlot's current collection.

	\ingroup kdefrontend
 */
DatasetMetadataManagerWidget::DatasetMetadataManagerWidget(QWidget* parent, const QMap< QString, QMap<QString, QMap<QString, QVector<QString>>>>& datasetMap) : QWidget(parent) {
	ui.setupUi(this);
	m_datasetModel = new DatasetModel(datasetMap);

	m_baseColor = (palette().color(QPalette::Base).lightness() < 128) ? QLatin1String("#5f5f5f") : QLatin1String("#ffffff");
	m_textColor = (palette().color(QPalette::Base).lightness() < 128) ? QLatin1String("#ffffff") : QLatin1String("#000000");

	ui.cbCollection->addItems(m_datasetModel->collections());
	ui.cbCategory->addItems(m_datasetModel->categories(ui.cbCollection->currentText()));
	ui.cbSubcategory->addItems(m_datasetModel->subcategories(ui.cbCollection->currentText(), ui.cbCategory->currentText()));

	ui.cbSeparatingCharacter->addItems(AsciiFilter::separatorCharacters());
	ui.cbCommentCharacter->addItems(AsciiFilter::commentCharacters());
	ui.cbNumberFormat->addItems(AbstractFileFilter::numberFormats());
	ui.cbDateTimeFormat->addItems(AbstractColumn::dateTimeFormats());

	connect(ui.leDatasetName, &QLineEdit::textChanged, [this] {
		emit checkOk();
	});
	connect(ui.leDownloadURL, &QLineEdit::textChanged, [this] {
		emit checkOk();
	});
	connect(ui.teDescription, &QTextEdit::textChanged, [this] {
		emit checkOk();
	});
	connect(ui.leFileName, &QLineEdit::textChanged, [this] {
		emit checkOk();
	});

	connect(ui.cbSubcategory, &QComboBox::currentTextChanged, [this] {
		emit checkOk();
	});

	connect(ui.cbCollection, &QComboBox::currentTextChanged, this, &DatasetMetadataManagerWidget::updateCategories);
	connect(ui.cbCategory, &QComboBox::currentTextChanged, this, &DatasetMetadataManagerWidget::updateSubcategories);
	connect(ui.bNewColumn, &QPushButton::clicked, this, &DatasetMetadataManagerWidget::addColumnDescription);
	connect(ui.bDelete, &QPushButton::clicked, this, &DatasetMetadataManagerWidget::removeColumnDescription);

	loadSettings();
}

DatasetMetadataManagerWidget::~DatasetMetadataManagerWidget() {
	KConfigGroup conf(KSharedConfig::openConfig(), "DatasetMetadataManagerWidget");

	//filter settings
	conf.writeEntry("separator", ui.cbSeparatingCharacter->currentText());
	conf.writeEntry("commentChar", ui.cbCommentCharacter->currentText());
	conf.writeEntry("numberFormat", ui.cbNumberFormat->currentIndex());
	conf.writeEntry("dateTimeFormat", ui.cbDateTimeFormat->currentText());
	conf.writeEntry("createIndexColumn", ui.chbCreateIndex->isChecked());
	conf.writeEntry("skipEmptyParts", ui.chbSkipEmptyParts->isChecked());
	conf.writeEntry("simplifyWhitespaces", ui.chbSimplifyWhitespaces->isChecked());
	conf.writeEntry("removeQuotes", ui.chbRemoveQuotes->isChecked());
	conf.writeEntry("useFirstRowForVectorName", ui.chbHeader->isChecked());
	conf.writeEntry("collection", ui.cbCollection->currentText());
	conf.writeEntry("category", ui.cbCategory->currentText());
	conf.writeEntry("subcategory", ui.cbSubcategory->currentText());
}

/**
 * @brief Loads the settings of the widget.
 */
void DatasetMetadataManagerWidget::loadSettings() {
	KConfigGroup conf(KSharedConfig::openConfig(), "DatasetMetadataManagerWidget");
	ui.cbCommentCharacter->setCurrentItem(conf.readEntry("commentChar", "#"));
	ui.cbSeparatingCharacter->setCurrentItem(conf.readEntry("separator", "auto"));
	//TODO: use general setting for decimal separator?
	ui.cbNumberFormat->setCurrentIndex(conf.readEntry("numberFormat", static_cast<int>(QLocale::AnyLanguage)));
	ui.cbDateTimeFormat->setCurrentItem(conf.readEntry("dateTimeFormat", "yyyy-MM-dd hh:mm:ss.zzz"));
	ui.chbCreateIndex->setChecked(conf.readEntry("createIndexColumn", false));
	ui.chbSimplifyWhitespaces->setChecked(conf.readEntry("simplifyWhitespaces", true));
	ui.chbRemoveQuotes->setChecked(conf.readEntry("removeQuotes", false));
	ui.chbSkipEmptyParts->setChecked(conf.readEntry("skipEmptyParts", false));
	ui.chbHeader->setChecked(conf.readEntry("useFirstRowForVectorName", true));

	QString lastCollection = conf.readEntry("collection", "");
	if(m_datasetModel->collections().contains(lastCollection))
		ui.cbCollection->setCurrentText(lastCollection);

	QString lastCategory = conf.readEntry("category", "");
	if(m_datasetModel->categories(ui.cbCollection->currentText()).contains(lastCategory))
		ui.cbCategory->setCurrentText(lastCategory);

	QString lastSubcategory = conf.readEntry("subcategory", "");
	if(m_datasetModel->subcategories(ui.cbCollection->currentText(), ui.cbCategory->currentText()).contains(lastSubcategory))
		ui.cbSubcategory->setCurrentText(lastSubcategory);
}

/**
 * @brief Checks whether leFileName contains a valid file name.
 */
bool DatasetMetadataManagerWidget::checkFileName() {
	const QString fileName = ui.leFileName->text();

	//check whether it contains only digits, letters, -, _ or not
	const QRegularExpression re("^[\\w\\d-]+$");
	const QRegularExpressionMatch match = re.match(fileName);
	bool hasMatch = match.hasMatch();

	if(!hasMatch || fileName.isEmpty()) {
		qDebug("File name invalid");
		QPalette palette;
		palette.setColor(QPalette::Base, Qt::red);
		palette.setColor(QPalette::Text, Qt::black);
		ui.leFileName->setPalette(palette);
		ui.leFileName->setToolTip("Invalid name for a file (it can contain:digits, letters, -, _)");
	} else {
		QPalette palette;
		palette.setColor(QPalette::Base, m_baseColor);
		palette.setColor(QPalette::Text, m_textColor);
		ui.leFileName->setPalette(palette);
		ui.leFileName->setToolTip("");
	}

	//check whether there already is a file named like this or not.
	bool found = false;

	if(m_datasetModel->allDatasetsList().toStringList().contains(fileName)) {
		qDebug("There already is a metadata file with this name");
		QPalette palette;
		palette.setColor(QPalette::Base, Qt::red);
		palette.setColor(QPalette::Text, Qt::black);
		ui.leFileName->setPalette(palette);
		ui.leFileName->setToolTip("There already is a dataset metadata file with this name!");
		found = true;
	} else {
		if(hasMatch) {
			QPalette palette;
			palette.setColor(QPalette::Base, m_baseColor);
			palette.setColor(QPalette::Text, m_textColor);
			ui.leFileName->setPalette(palette);
			ui.leFileName->setToolTip("");
		}
	}

	return hasMatch && !found;
}

/**
 * @brief Checks whether leDownloadURL contains a valid URL.
 */
bool DatasetMetadataManagerWidget::urlExists() {

	//Check whether the given url is acceptable syntactically
	const QRegularExpression re(R"(^(?:http(s)?:\/\/)?[\w.-]+(?:\.[\w\.-]+)+[\w\-\._~:/?#[\]@!\$&'\(\)\*\+,;=.]+$)");
	const QRegularExpressionMatch match = re.match(ui.leDownloadURL->text());
	bool hasMatch = match.hasMatch();
	const bool urlExists = hasMatch && !ui.leDownloadURL->text().isEmpty();

	if(!urlExists){
		QPalette palette;
		palette.setColor(QPalette::Base, Qt::red);
		palette.setColor(QPalette::Text, Qt::black);
		ui.leDownloadURL->setPalette(palette);
		ui.leDownloadURL->setToolTip("The URL is invalid!");
	} else {
		QPalette palette;
		palette.setColor(QPalette::Base, m_baseColor);
		palette.setColor(QPalette::Text, m_textColor);
		ui.leDownloadURL->setPalette(palette);
		ui.leDownloadURL->setToolTip("");
	}
	return urlExists;
}

/**
 * @brief Checks whether leDatasetName is empty or not.
 */
bool DatasetMetadataManagerWidget::checkDatasetName() {
	const bool longNameOk = !ui.leDatasetName->text().isEmpty();
	if(!longNameOk)	{
		QPalette palette;
		palette.setColor(QPalette::Base, Qt::red);
		palette.setColor(QPalette::Text, Qt::black);
		ui.leDatasetName->setPalette(palette);
		ui.leDatasetName->setToolTip("Please fill this out!");
	} else {
		QPalette palette;
		palette.setColor(QPalette::Base, m_baseColor);
		palette.setColor(QPalette::Text, m_textColor);
		ui.leDatasetName->setPalette(palette);
		ui.leDatasetName->setToolTip("");
	}

	return longNameOk;
}

/**
 * @brief Checks whether teDescription is empty or not.
 */
bool DatasetMetadataManagerWidget::checkDescription() {
	const bool descriptionOk = !ui.teDescription->toPlainText().isEmpty();
	if(!descriptionOk) {
		QPalette palette;
		palette.setColor(QPalette::Base, Qt::red);
		palette.setColor(QPalette::Text, Qt::black);
		ui.teDescription->setPalette(palette);
		ui.teDescription->setToolTip("Please fill this out!");
	} else {
		QPalette palette;
		palette.setColor(QPalette::Base, m_baseColor);
		palette.setColor(QPalette::Text, m_textColor);
		ui.teDescription->setPalette(palette);
		ui.teDescription->setToolTip("");
	}

	return descriptionOk;
}

/**
 * @brief Checks whether the given QComboBox's current text is empty or not.
 */
bool DatasetMetadataManagerWidget::checkCategories(QComboBox* comboBox) {
	//Check whether it is a word or not (might contain digits)
	const QString fileName = comboBox->currentText();
	const QRegularExpression re("^[\\w\\d]+$");
	const QRegularExpressionMatch match = re.match(fileName);
	const bool hasMatch = match.hasMatch();

	if(!hasMatch || fileName.isEmpty()) {
		qDebug("category/subcategory name invalid");
		QPalette palette;
		palette.setColor(QPalette::Base,Qt::red);
		palette.setColor(QPalette::Text,Qt::black);
		comboBox->setPalette(palette);
		comboBox->setToolTip("Invalid or empty name for a category/subcategory (only digits and letters)");
	} else {
		QPalette palette;
		palette.setColor(QPalette::Base, m_baseColor);
		palette.setColor(QPalette::Text, m_textColor);
		comboBox->setPalette(palette);
		comboBox->setToolTip("");
	}

	return hasMatch;
}

/**
 * @brief Enables/disables the widget's components meant to configure the metadata file of the new dataset.
 */
void DatasetMetadataManagerWidget::enableDatasetSettings(bool enable) {
	ui.leFileName->setEnabled(enable);
	ui.leFileName->setReadOnly(!enable);
	ui.leDatasetName->setEnabled(enable);
	ui.leDatasetName->setReadOnly(!enable);
	ui.leDownloadURL->setEnabled(enable);
	ui.leDownloadURL->setReadOnly(!enable);
	ui.teDescription->setEnabled(enable);
	ui.teDescription->setReadOnly(!enable);
	ui.gbColumnDescriptions->setEnabled(enable);
	ui.gbFilter->setEnabled(enable);
}

/**
 * @brief Checks whether the introduced data is valid or not. Used by DatasetMetadataManagerDialog.
 */
bool DatasetMetadataManagerWidget::checkDataValidity() {
	const bool fileNameOK = checkFileName();
	const bool urlOk = urlExists();
	const bool longNameOk = checkDatasetName();
	const bool descriptionOk = checkDescription();
	const bool categoryOk = checkCategories(ui.cbCategory);
	const bool subcategoryOk = checkCategories(ui.cbSubcategory);
	const bool collectionOk = checkCategories(ui.cbCollection);

	enableDatasetSettings(categoryOk && subcategoryOk && collectionOk);

	return fileNameOK && urlOk && longNameOk && descriptionOk && subcategoryOk && categoryOk && collectionOk;
}

/**
 * @brief Updates content of cbCategory based on current collection.
 */
void DatasetMetadataManagerWidget::updateCategories(const QString& collection) {
	ui.cbCategory->clear();
	if( m_datasetModel->collections().contains(collection))
		ui.cbCategory->addItems(m_datasetModel->categories(collection));

	emit checkOk();
}

/**
 * @brief Updates content of cbSubcategory based on current category.
 */
void DatasetMetadataManagerWidget::updateSubcategories(const QString& category) {
	ui.cbSubcategory->clear();
	const QString collection = ui.cbCollection->currentText();
	if( m_datasetModel->categories(collection).contains(category))
		ui.cbSubcategory->addItems(m_datasetModel->subcategories(collection, category));

	emit checkOk();
}

/**
 * @brief Updates the metadata file containing the categories, subcategories and datasets.
 * @param fileName the name of the metadata file (path)
 */
void DatasetMetadataManagerWidget::updateDocument(const QString& dirPath) {
	if(checkDataValidity()) {
		//Check whether the current collection already exists, if yes update it
		if(m_datasetModel->collections().contains(ui.cbCollection->currentText())) {
			QString fileName = dirPath + ui.cbCollection->currentText() + ".json";
			qDebug() << "Updating: " << fileName;
			QFile file(fileName);
			if (file.open(QIODevice::ReadWrite)) {
				QJsonDocument document = QJsonDocument::fromJson(file.readAll());
				QJsonObject rootObject = document.object();
				QJsonValueRef categoryArrayRef = rootObject.find("categories").value();
				QJsonArray categoryArray = categoryArrayRef.toArray();

				//Check whether the current category already exists
				bool foundCategory = false;
				for(int i = 0 ; i < categoryArray.size(); ++i) {
					QJsonValueRef categoryRef = categoryArray[i];
					QJsonObject currentCategory = categoryRef.toObject();
					QString categoryName = currentCategory.value("category_name").toString();

					//If we find the category we have to update that QJsonObject
					if(categoryName.compare(ui.cbCategory->currentText()) == 0) {
						foundCategory = true;
						QJsonValueRef subcategoryArrayRef = currentCategory.find("subcategories").value();
						QJsonArray subcategoryArray = subcategoryArrayRef.toArray();

						//Check whether the current subcategory already exists
						bool subcategoryFound = false;
						for(int j = 0; j < subcategoryArray.size(); ++j) {
							QJsonValueRef subcategoryRef = subcategoryArray[j];
							QJsonObject currentSubcategory = subcategoryRef.toObject();
							QString subcategoryName = currentSubcategory.value("subcategory_name").toString();

							//If we find the subcategory we have to update that QJsonObject
							if(subcategoryName.compare(ui.cbSubcategory->currentText()) == 0) {
								subcategoryFound = true;
								QJsonValueRef datasetsRef = currentSubcategory.find("datasets").value();
								QJsonArray datasets = datasetsRef.toArray();

								datasets.append(createDatasetObject());
								datasetsRef = datasets;

								subcategoryRef = currentSubcategory;
								subcategoryArrayRef = subcategoryArray;
								categoryRef = currentCategory;
								categoryArrayRef = categoryArray;
								document.setObject(rootObject);
								break;
							}
						}

						//If we didn't find the subcategory, we have to create it
						if(!subcategoryFound) {
							qDebug() << "Subcategory not found";
							QJsonObject newSubcategory;
							newSubcategory.insert("subcategory_name", ui.cbSubcategory->currentText());

							QJsonArray datasets;
							datasets.append(createDatasetObject());

							newSubcategory.insert("datasets", datasets);
							subcategoryArray.append(newSubcategory);

							subcategoryArrayRef = subcategoryArray;
							categoryRef = currentCategory;
							categoryArrayRef = categoryArray;
							document.setObject(rootObject);
						}
						break;
					}
				}

				//If we didn't find the category, we have to create it
				if(!foundCategory) {
					qDebug() << "Category not found";
					QJsonObject newCategory;
					newCategory.insert("category_name", ui.cbCategory->currentText());

					QJsonArray subcategoryArray;

					QJsonObject newSubcategory;
					newSubcategory.insert("subcategory_name", ui.cbSubcategory->currentText());

					QJsonArray datasets;
					datasets.append(createDatasetObject());
					newSubcategory.insert("datasets", datasets);

					subcategoryArray.append(newSubcategory);
					newCategory.insert("subcategories", subcategoryArray);

					categoryArray.append(newCategory);
					categoryArrayRef = categoryArray;
					document.setObject(rootObject);
				}
				file.close();
				bool rc = file.open(QIODevice::ReadWrite | QIODevice::Truncate);
				if (rc) {
					file.write(document.toJson());
					file.close();
				} else
					qDebug() << "Couldn't write file, because " << file.errorString();
			} else
				qDebug() << "Couldn't open dataset category file, because " << file.errorString();
		}
		//If the collection doesn't exist we have to create a new json file for it.
		else {
			QString fileName = dirPath + "DatasetCollections.json";
			qDebug() << "creating: " << fileName;
			QFile file(fileName);
			if (file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
				QJsonArray collectionArray;

				for(QString collection : m_datasetModel->collections())
					collectionArray.append(collection);

				collectionArray.append(ui.cbCollection->currentText());

				QJsonDocument newDocument;
				newDocument.setArray(collectionArray);
				file.write(newDocument.toJson());
				file.close();
			}

			QJsonObject rootObject;

			rootObject.insert("collection_name", ui.cbCollection->currentText());

			QJsonArray categoryArray;
			QJsonObject newCategory;
			newCategory.insert("category_name", ui.cbCategory->currentText());

			QJsonArray subcategoryArray;

			QJsonObject newSubcategory;
			newSubcategory.insert("subcategory_name", ui.cbSubcategory->currentText());

			QJsonArray datasets;
			datasets.append(createDatasetObject());
			newSubcategory.insert("datasets", datasets);

			subcategoryArray.append(newSubcategory);
			newCategory.insert("subcategories", subcategoryArray);
			categoryArray.append(newCategory);
			rootObject.insert("categories", categoryArray);

			QJsonDocument document;
			document.setObject(rootObject);

			QFile collectionFile(dirPath + ui.cbCollection->currentText() + ".json");
			if (collectionFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
				collectionFile.write(document.toJson());
				collectionFile.close();
			}
		}
	}
}

/**
 * @brief Creates and returns a QJsonObject based on the given settings of the widget, this will be part of the collection's metadata file
 */
QJsonObject DatasetMetadataManagerWidget::createDatasetObject() {
	QJsonObject rootObject;
	rootObject.insert("filename", ui.leFileName->text());
	rootObject.insert("name", ui.leDatasetName->text());
	rootObject.insert("download", ui.leDownloadURL->text());
	rootObject.insert("description", ui.teDescription->toPlainText());
	rootObject.insert("separator", ui.cbSeparatingCharacter->currentText());
	rootObject.insert("comment_character", ui.cbCommentCharacter->currentText());
	rootObject.insert("DateTime_format", ui.cbDateTimeFormat->currentText());
	rootObject.insert("number_format", ui.cbNumberFormat->currentIndex());
	rootObject.insert("create_index_column", ui.chbCreateIndex->isChecked());
	rootObject.insert("skip_empty_parts", ui.chbSkipEmptyParts->isChecked());
	rootObject.insert("simplify_whitespaces", ui.chbSimplifyWhitespaces->isChecked());
	rootObject.insert("remove_quotes", ui.chbRemoveQuotes->isChecked());
	rootObject.insert("use_first_row_for_vectorname", ui.chbHeader->isChecked());

	for(int i = 0; i < m_columnDescriptions.size(); ++i)
		rootObject.insert(i18n("column_description_%1", i), m_columnDescriptions[i]);

	return rootObject;
}

/**
 * @brief Adds a new QLineEdit so the user can set a new column description.
 */
void DatasetMetadataManagerWidget::addColumnDescription() {
	QLabel* label = new QLabel();
	label->setText(i18n("Description for column %1", m_columnDescriptions.size() + 1));
	QLineEdit* lineEdit = new QLineEdit;

	int layoutIndex = m_columnDescriptions.size() + 1;
	ui.columnLayout->addWidget(label, layoutIndex, 0);
	ui.columnLayout->addWidget(lineEdit, layoutIndex, 1, 1, -1);

	connect(lineEdit, &QLineEdit::textChanged, [this, layoutIndex] (const QString& text) {
		m_columnDescriptions[layoutIndex - 1] = text;
	});

	m_columnDescriptions.append("");
}

/**
 * @brief Removes the lastly added QLineEdit (used to set a column description).
 */
void DatasetMetadataManagerWidget::removeColumnDescription() {
	const int index = ui.columnLayout->count() - 1;

	QLayoutItem *item;
	if ((item = ui.columnLayout->takeAt(index)) != nullptr) {
		delete item->widget();
		delete item;
	}

	if ((item = ui.columnLayout->takeAt(index - 1)) != nullptr){
		delete item->widget();
		delete item;
	}

	m_columnDescriptions.removeLast();
}

/**
 * @brief returns the path to the new metadata file of the new dataset.
 */
QString DatasetMetadataManagerWidget::getMetadataFilePath() const {
	return m_metadataFilePath;
}

/**
 * @brief Sets the collection name.
 */
void DatasetMetadataManagerWidget::setCollection(const QString& collection) {
	ui.cbCollection->setCurrentText(collection);
}

/**
 * @brief Sets the category name.
 */
void DatasetMetadataManagerWidget::setCategory(const QString& category) {
	ui.cbCategory->setCurrentText(category);
}

/**
 * @brief Sets the subcategory name.
 */
void DatasetMetadataManagerWidget::setSubcategory(const QString& subcategory) {
	ui.cbSubcategory->setCurrentText(subcategory);
}

/**
 * @brief Sets the short name of the dataset.
 */
void DatasetMetadataManagerWidget::setShortName(const QString& name) {
	ui.leFileName->setText(name);
}

/**
 * @brief Sets the full name of the dataset.
 */
void DatasetMetadataManagerWidget::setFullName(const QString& name) {
	ui.leDatasetName->setText(name);
}

/**
 * @brief Sets the text of the description.
 */
void DatasetMetadataManagerWidget::setDescription(const QString& description) {
	ui.teDescription->setText(description);
}

/**
 * @brief Sets the download url.
 */
void DatasetMetadataManagerWidget::setURL(const QString& url) {
	ui.leDownloadURL->setText(url);
}
