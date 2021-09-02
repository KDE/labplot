/*
    File                 : FITSHeaderEditWidget.h
    Project              : LabPlot
    Description          : Widget for listing/editing FITS header keywords
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Fabian Kristof <fkristofszabolcs@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef FITSHEADEREDITWIDGET_H
#define FITSHEADEREDITWIDGET_H

#include <QWidget>
#include "backend/datasources/filters/FITSFilter.h"

namespace Ui {
	class FITSHeaderEditWidget;
}

class FITSHeaderEditWidget : public QWidget {
	Q_OBJECT

public:
	explicit FITSHeaderEditWidget(QWidget *parent = nullptr);
	~FITSHeaderEditWidget() override;

private:
	Ui::FITSHeaderEditWidget* ui;
	QAction* m_actionRemoveKeyword{nullptr};
	QAction* m_actionAddKeyword{nullptr};
	QAction* m_actionAddmodifyUnit{nullptr};
	QAction* m_actionRemoveExtension{nullptr};

	QMenu* m_keywordActionsMenu{nullptr};
	QMenu* m_extensionActionsMenu{nullptr};

	struct HeaderUpdate {
		QList<FITSFilter::Keyword> newKeywords;
		QVector<FITSFilter::Keyword> updatedKeywords;
		QList<FITSFilter::Keyword> removedKeywords;
	};

	struct ExtensionData {
		HeaderUpdate updates;
		QList<FITSFilter::Keyword> keywords;
	};

	QMap<QString, ExtensionData> m_extensionData;
	QStringList m_removedExtensions;
	QString m_seletedExtension;

	FITSFilter* m_fitsFilter;

	bool m_initializingTable{false};

	void initActions();
	void initContextMenus();
	void connectActions();
	void fillTable();
	QList<QString> mandatoryKeywords() const;
	bool eventFilter(QObject*, QEvent*) override;

public slots:
	bool save();

private slots:
	void openFile();

	void fillTableSlot(QTreeWidgetItem* item, int col);
	void updateKeyword(QTableWidgetItem*);

	void removeKeyword();
	void removeExtension();
	void addKeyword();
	void addModifyKeywordUnit();
	void closeFile();
	void enableButtonCloseFile(QTreeWidgetItem *, int);
	void enableButtonAddUnit();
signals:
	void changed(bool);
};

#endif // FITSHEADEREDITWIDGET_H
