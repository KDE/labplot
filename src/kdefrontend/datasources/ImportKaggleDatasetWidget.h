/*
	File                 : ImportKaggleDatasetWidget.h
	Project              : LabPlot
	Description          : import kaggle dataset widget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Israel Galadima <izzygaladima@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMPORTKAGGLEDATASETWIDGET_H
#define IMPORTKAGGLEDATASETWIDGET_H

#include "kdefrontend/datasources/ImportFileWidget.h"
#include "ui_importkaggledatasetwidget.h"

#include <QWidget>

class QProcess;

class ImportKaggleDatasetWidget : public QWidget {
	Q_OBJECT

public:
	explicit ImportKaggleDatasetWidget(QWidget*);
	~ImportKaggleDatasetWidget() override;

	void importToSpreadsheet(Spreadsheet*) const;

private:
	Ui::ImportKaggleDatasetWidget ui;
	ImportFileWidget* m_importFileWidget{nullptr};
	QProcess* m_kaggleCli{nullptr};
	int m_resultPage{1};
	QComboBox* m_cbDatasetFiles{nullptr};
	const int RESULTS_PER_PAGE{20};
	QString m_currentDatasetDirPath;

	void prepareImportFileWidget();
	void prepareImportKaggleDatasetWidget();
	void listKaggleDatasets();
	void displayKaggleDatasetMetadata();
	void searchKaggleDatasets();
	void fetchKaggleDatasetMetadata(int index);
	void downloadKaggleDataset();
	void listDownloadedKaggleDatasetFiles(const QString&);

public Q_SLOTS:
	void toggleOptionsVisibility();

Q_SIGNALS:
	void toggleOkBtn(bool);
	void toggleOptionsBtn(bool);
	void showError(const QString&);
	void clearError();
};

#endif
