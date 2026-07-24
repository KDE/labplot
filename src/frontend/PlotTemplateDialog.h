/*
	File                 : TemplateChooserDialog.cpp
	Project              : LabPlot
	Description          : dialog to load user-defined plot definitions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PLOTTEMPLATEDIALOG_H
#define PLOTTEMPLATEDIALOG_H

#include <QAbstractListModel>
#include <QDialog>

class Project;
class Worksheet;
class TemplateListModel;
class CartesianPlot;

namespace Ui {
class PlotTemplateDialog;
}

class PlotTemplateDialog : public QDialog {
	Q_OBJECT

public:
	explicit PlotTemplateDialog(QWidget* parent = nullptr);
	void updateErrorMessage(const QString&);
	~PlotTemplateDialog();

	QString templatePath() const;
	void showPreview();
	static const QString format;
	static QString defaultTemplateInstallPath();
	CartesianPlot* generatePlot();

private:
	void chooseTemplateSearchPath();
	void listViewTemplateChanged(const QModelIndex& current, const QModelIndex& previous);
	void changePreviewSource(int row);
	void customTemplatePathChanged(const QString&);

private:
	Ui::PlotTemplateDialog* ui;
	Project* m_project; // TODO: use smart pointer!
	Worksheet* m_worksheet;
	QWidget* m_worksheetView;
	TemplateListModel* mTemplateListModelDefault;
	TemplateListModel* mTemplateListModelCustom;
	bool mLoading{false};
};

class TemplateListModel : public QAbstractListModel {
public:
	explicit TemplateListModel(const QString& searchPath, QObject* parent = nullptr);
	void setSearchPath(const QString& searchPath);
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	const QString& searchPath() {
		return mSearchPath;
	}

	enum Roles {
		FilenameRole = Qt::ItemDataRole::UserRole + 1,
		FilePathRole,
	};

	struct File {
		QString path; // path with file name and extension. Used to create plot
		QString fileName; // file name only without extension. Visible part in the view
	};

private:
	QVector<File> mFiles;
	QString mSearchPath;
};

#endif // PLOTTEMPLATEDIALOG_H
