/*
    File                 : ExamplesWidget.h
    Project              : LabPlot
    Description          : widget showing the available example projects
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EXAMPLESWIDGET_H
#define EXAMPLESWIDGET_H

#include "ui_exampleswidget.h"

class QCompleter;
class QStandardItemModel;
class ExamplesManager;

class ExamplesWidget : public QWidget {
	Q_OBJECT

public:
	explicit ExamplesWidget(QWidget*);
	~ExamplesWidget() override;

	QString path() const;

private:
	Ui::ExamplesWidget ui;
	QCompleter* m_completer{nullptr};
	QStandardItemModel* m_model{nullptr};
	ExamplesManager* m_manager{nullptr};

	void loadCollections();
	void activateIconViewItem(const QString& name);
	void activateListViewItem(const QString& name);

private Q_SLOTS:
	void collectionChanged(int);
	void exampleChanged();
	void showInfo();
	void toggleIconView();
	void viewModeChanged(int);
	void activated(const QString&);

Q_SIGNALS:
	void doubleClicked();
};

#endif // EXAMPLESWIDGET_H
