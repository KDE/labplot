/***************************************************************************
File                 : DatasetMetadataManagerWidget.cpp
Project              : LabPlot
Description          : widget for managing a metadata file of a dataset
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

#include "DatasetMetadataManagerWidget.h"
#include "QMap"
#include "QStringList"
#include "QMapIterator"
#include "backend/datasources/filters/AsciiFilter.h"
#include "QRegExpValidator"
#include "QRegExp"
#include "QUrl"
#include "QTcpSocket"
#include "QJsonDocument"
#include "QJsonArray"
#include "QJsonObject"
#include "QJsonValue"
#include "QFile"
#include "QDir"

DatasetMetadataManagerWidget::DatasetMetadataManagerWidget(QWidget* parent, const QMap<QString, QMap<QString, QVector<QString>>>& datasetMap) : QWidget(parent) {

	ui.setupUi(this);
	initCategories(datasetMap);
	initSubcategories(datasetMap);
	initDatasets(datasetMap);

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

	connect(ui.cbCategory, &QComboBox::currentTextChanged, this, &DatasetMetadataManagerWidget::updateSubcategories);
	connect(ui.bNewColumn, &QPushButton::clicked, this, &DatasetMetadataManagerWidget::addColumnDescription);
}

DatasetMetadataManagerWidget::~DatasetMetadataManagerWidget() {

}

void DatasetMetadataManagerWidget::initCategories(const QMap<QString, QMap<QString, QVector<QString>>>& datasetMap) {
	m_categoryList =  datasetMap.keys();
	ui.cbCategory->addItems(m_categoryList);
}

void DatasetMetadataManagerWidget::initSubcategories(const QMap<QString, QMap<QString, QVector<QString>>>& datasetMap) {
	for(auto i = datasetMap.begin(); i != datasetMap.end(); ++i) {
		m_subcategoryMap[i.key()] = i.value().keys();
	}

	QString selectedCategory = ui.cbCategory->currentText();
	ui.cbSubcategory->addItems(m_subcategoryMap[selectedCategory]);
}

void DatasetMetadataManagerWidget::initDatasets(const QMap<QString, QMap<QString, QVector<QString>>>& datasetMap) {
	for(auto i = datasetMap.begin(); i != datasetMap.end(); ++i) {
		for(auto j = i.value().begin(); j != i.value().end(); ++j) {
			m_datasetMap[j.key()] = j.value().toList();
		}
	}
}

bool DatasetMetadataManagerWidget::checkFileName() {
	QString fileName = ui.leFileName->text();
	QRegularExpression re("[\\w\\d-]+");
	QRegularExpressionMatch match = re.match(fileName);
	bool hasMatch = match.hasMatch();

	if(!hasMatch) {
		qDebug("File name invalid");
		QPalette palette;
		palette.setColor(QPalette::Base,Qt::red);
		palette.setColor(QPalette::Text,Qt::black);
		ui.leFileName->setPalette(palette);
		ui.leFileName->setToolTip("Invalid name for a file (it can contain:digits, letters, -, _)");
	} else {
		qDebug("File name valid");
		QPalette palette;
		palette.setColor(QPalette::Base,Qt::white);
		palette.setColor(QPalette::Text,Qt::black);
		ui.leFileName->setPalette(palette);
		ui.leFileName->setToolTip("");
	}

	bool found = false;

	if(m_categoryList.contains(ui.cbCategory->currentText())) {
		qDebug("Category found");
		if(m_datasetMap.keys().contains(ui.cbSubcategory->currentText())) {
			qDebug("Subcategory found");
			if(m_datasetMap[ui.cbSubcategory->currentText()].contains(fileName)) {
				qDebug("Dataset found");
				QPalette palette;
				palette.setColor(QPalette::Base,Qt::red);
				palette.setColor(QPalette::Text,Qt::black);
				ui.leFileName->setPalette(palette);
				ui.leFileName->setToolTip("There already is a dataset with this name!");
				found = true;
			} else {
				qDebug("Dataset not found");
				QPalette palette;
				palette.setColor(QPalette::Base,Qt::white);
				palette.setColor(QPalette::Text,Qt::black);
				ui.leFileName->setPalette(palette);
				ui.leFileName->setToolTip("");
			}
		}
	}

	return hasMatch && !found;
}

bool DatasetMetadataManagerWidget::urlExists() {
	if(QUrl(ui.leDownloadURL->text()).isValid()) {
		QPalette palette;
		palette.setColor(QPalette::Base,Qt::white);
		palette.setColor(QPalette::Text,Qt::black);
		ui.leDownloadURL->setPalette(palette);
		ui.leDownloadURL->setToolTip("");
		return true;
	} else {
		QPalette palette;
		palette.setColor(QPalette::Base,Qt::red);
		palette.setColor(QPalette::Text,Qt::black);
		ui.leDownloadURL->setPalette(palette);
		ui.leDownloadURL->setToolTip("The URL is invalid!");
	}
	return false;
}

bool DatasetMetadataManagerWidget::checkDataValidity() {
	bool fileNameOK = checkFileName();
	bool urlOk = urlExists();
	bool longNameOk = !ui.leDatasetName->text().isEmpty();
	bool descriptionOk = !ui.teDescription->toPlainText().isEmpty();
	bool categoryOk = !ui.cbCategory->currentText().isEmpty();
	bool subcategoryOk = !ui.cbSubcategory->currentText().isEmpty();

	return fileNameOK && urlOk && longNameOk && descriptionOk && subcategoryOk && categoryOk;
}

void DatasetMetadataManagerWidget::updateSubcategories(const QString& category) {
	ui.cbSubcategory->clear();
	if(m_categoryList.contains(category)) {
		ui.cbSubcategory->addItems(m_subcategoryMap[category]);
	}
}

void DatasetMetadataManagerWidget::updateDocument(const QString& fileName) {
	qDebug() << "updating: " << fileName;
	QFile file(fileName);
	if (file.open(QIODevice::ReadWrite)) {
		QJsonDocument document = QJsonDocument::fromJson(file.readAll());
		//qDebug() <<document.toJson();
		QJsonObject RootObject = document.object();
		QJsonValueRef categoryArrayRef = RootObject.find("categories").value();
		QJsonArray categoryArray = categoryArrayRef.toArray();

		bool foundCategory = false;
		for(int i = 0 ; i < categoryArray.size(); ++i) {
			QJsonValueRef categoryRef = categoryArray[i];
			QJsonObject currentCategory = categoryRef.toObject();
			QString categoryName = currentCategory.value("category_name").toString();
			qDebug() << "Category name: " << categoryName;

			if(categoryName.compare(ui.cbCategory->currentText()) == 0) {
				foundCategory = true;
				QJsonValueRef subcategoryArrayRef = currentCategory.find("subcategories").value();
				QJsonArray subcategoryArray = subcategoryArrayRef.toArray();
				qDebug() << "subcategoryArray: " << subcategoryArray.toVariantList();

				bool subcategoryFound = false;

				for(int j = 0; j < subcategoryArray.size(); ++j) {
					QJsonValueRef subcategoryRef = subcategoryArray[j];
					QJsonObject currentSubcategory = subcategoryRef.toObject();
					QString subcategoryName = currentSubcategory.value("subcategory_name").toString();
					qDebug() << "Subcat name: " << subcategoryName;

					if(subcategoryName.compare(ui.cbSubcategory->currentText()) == 0) {
						subcategoryFound = true;
						QJsonValueRef datasetsRef = currentSubcategory.find("datasets").value();
						QJsonArray datasets = datasetsRef.toArray();
						qDebug() <<"Datasets content: " << datasets.toVariantList();

						datasets.append(QJsonValue(ui.leFileName->text()));
						datasetsRef = datasets;

						subcategoryRef = currentSubcategory;
						subcategoryArrayRef = subcategoryArray;
						categoryRef = currentCategory;
						categoryArrayRef = categoryArray;
						document.setObject(RootObject);
						break;
					}
				}

				if(!subcategoryFound) {
					qDebug() << "Subcat not found";
					QJsonObject newSubcategory;
					newSubcategory.insert("subcategory_name", ui.cbSubcategory->currentText());

					QJsonArray datasets;
					datasets.append(QJsonValue(ui.leFileName->text()));

					newSubcategory.insert("datasets", datasets);
					subcategoryArray.append(newSubcategory);

					subcategoryArrayRef = subcategoryArray;
					categoryRef = currentCategory;
					categoryArrayRef = categoryArray;
					document.setObject(RootObject);
				}
				break;
			}
		}

		if(!foundCategory) {
			qDebug() << "Cat not found";
			QJsonObject newCategory;
			newCategory.insert("category_name", ui.cbCategory->currentText());

			QJsonArray subcategoryArray;

			QJsonObject newSubcategory;
			newSubcategory.insert("subcategory_name", ui.cbSubcategory->currentText());

			QJsonArray datasets;
			datasets.append(QJsonValue(ui.leFileName->text()));
			newSubcategory.insert("datasets", datasets);

			subcategoryArray.append(newSubcategory);
			newCategory.insert("subcategories", subcategoryArray);

			categoryArray.append(newCategory);
			categoryArrayRef = categoryArray;
			document.setObject(RootObject);
		}
		qDebug() <<document.toJson();
		file.close();
		file.open(QIODevice::ReadWrite | QIODevice::Truncate);
		file.write(document.toJson());
		file.close();
	} else {
		qDebug() << "Couldn't open dataset category file, because " << file.errorString();
	}
}

void DatasetMetadataManagerWidget::createNewMetadata(const QString& dirPath) {
	QString fileName = dirPath + QDir::separator() + ui.leFileName->text() + ".json";
	qDebug() << "Creating " << fileName;

	QFile file(fileName);
	if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		QJsonObject rootObject;

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

		for(int i = 0; i < m_columnDescriptions.size(); i++) {
			rootObject.insert(i18n("column_description_%1", i), m_columnDescriptions[i]);
		}

		QJsonDocument document;
		document.setObject(rootObject);
		qDebug() <<document.toJson();
		file.write(document.toJson());
		file.close();
	} else {
		qDebug() <<"Couldn't create new metadata file" << file.errorString();;
	}
}

void DatasetMetadataManagerWidget::addColumnDescription() {
	QLabel* label = new QLabel();
	label->setText(i18n("Description for column %1", m_columnDescriptions.size() + 1));
	QLineEdit* lineEdit = new QLineEdit;

	int layoutIndex = m_columnDescriptions.size() + 1;
	ui.columnLayout->addWidget(label, layoutIndex, 0);
	ui.columnLayout->addWidget(lineEdit, layoutIndex, 1);

	connect(lineEdit, &QLineEdit::textChanged, [this, layoutIndex] (const QString& text) {
		m_columnDescriptions[layoutIndex - 1] = text;
		qDebug() << m_columnDescriptions;
	});

	m_columnDescriptions.append("");
}
