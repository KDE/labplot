/*
	File                 : ColorMapsWidget.h
	Project              : LabPlot
	Description          : widget showing the available color maps
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Alexander Semke (alexander.semke@web.de)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/


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

	void loadCollections();
	void activateIconViewItem(const QString& name);
	void activateListViewItem(const QString& name);

private slots:
	void collectionChanged(int);
	void colorMapChanged();
	void showInfo();
	void toggleIconView();
	void viewModeChanged(int);
	void activated(const QString&);

signals:
	void doubleClicked();
};

#endif // COLORMAPSWIDGET_H
