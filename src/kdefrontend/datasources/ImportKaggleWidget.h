/*
	File                 : ImportKaggleWidget.h
	Project              : LabPlot
	Description          : import kaggle dataset widget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Israel Galadima <izzygaladima@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMPORTKAGGLEWIDGET_H
#define IMPORTKAGGLEWIDGET_H

#include "kdefrontend/datasources/ImportDatasetWidget.h"
#include "kdefrontend/datasources/ImportFileWidget.h"
#include <QProcess>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>
#include <qobjectdefs.h>

class ImportKaggleWidget : public QWidget {
	Q_OBJECT

public:
	explicit ImportKaggleWidget(QWidget*);
	~ImportKaggleWidget() override;

	void importToSpreadsheet(Spreadsheet*) const;

private:
	ImportDatasetWidget* m_importDatasetWidget{nullptr};
	ImportFileWidget* m_importFileWidget{nullptr};
	QProcess* m_kaggleCli{nullptr};
	int m_resultPage{1};
	QVBoxLayout* m_vLayout{nullptr};
	QToolButton* m_bPrevDatasets{nullptr};
	QToolButton* m_bNextDatasets{nullptr};
	QToolButton* m_bDownloadDataset{nullptr};
	QComboBox* m_cbDatasetFiles{nullptr};

	void prepareImportDatasetWidget();
	void prepareImportFileWidget();
	void addWidgetsToUi();
	void listKaggleDatasets();
	void displayKaggleDatasetMetadata();
	void searchKaggleDatasets();
	void fetchKaggleDatasetMetadata(int index);
	void downloadKaggleDataset();
	void listDownloadedKaggleDatasetFiles(const QString&);

Q_SIGNALS:
	void toggleOkBtn(bool);
	void showError(const QString&);
	void clearError();
};

#endif
