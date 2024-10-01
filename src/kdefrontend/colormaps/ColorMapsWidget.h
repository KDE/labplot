/*
	File                 : ColorMapsWidget.h
	Project              : LabPlot
	Description          : widget showing the available color maps
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COLORMAPSWIDGET_H
#define COLORMAPSWIDGET_H

#include "ui_colormapswidget.h"

class QCompleter;
class QStandardItemModel;
class ColorMapsManager;

class ColorMapsWidget : public QWidget {
	Q_OBJECT

public:
	explicit ColorMapsWidget(QWidget*);
	~ColorMapsWidget() override;

	enum class ViewMode { IconView, ListView, ListDetailsView };

	QPixmap previewPixmap();
	QString name() const;
	QVector<QColor> colors() const;

private:
	Ui::ColorMapsWidget ui;
	QCompleter* m_completer{nullptr};
	QPixmap m_pixmap;
	QVector<QColor> m_colormap;
	QStandardItemModel* m_model{nullptr};
	ColorMapsManager* m_manager{nullptr};
	ViewMode m_viewMode{ViewMode::IconView};

	bool eventFilter(QObject*, QEvent*) override;
	void loadCollections();
	void activateIconViewItem(const QString& name);
	void activateListViewItem(const QString& name);
	void activateListDetailsViewItem(const QString& name);
	void switchViewMode();
	QString colorMapName() const;

private Q_SLOTS:
	void collectionChanged(int);
	void colorMapChanged();
	void colorMapDetailsChanged();
	void showInfo();
	void showViewModeMenu();
	void viewModeChanged();
	void activated(const QString&);

	void detailsContextMenuRequest(QPoint);
	void detailsCopy(bool copyAll = false);
	void detailsCopyAll();

Q_SIGNALS:
	void doubleClicked();
};

#endif // COLORMAPSWIDGET_H
