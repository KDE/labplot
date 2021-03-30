/***************************************************************************
	File                 : ColorMapsWidget.h
	Project              : LabPlot
	Description          : widget showing the available color maps
	--------------------------------------------------------------------
	Copyright            : (C) 2021 by Alexander Semke (alexander.semke@web.de)

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


#ifndef COLORMAPSWIDGET_H
#define COLORMAPSWIDGET_H

#include "ui_colormapswidget.h"

#include "QMap"

class QCompleter;
class QStandardItemModel;

class ColorMapsWidget : public QWidget {
	Q_OBJECT

public:
	explicit ColorMapsWidget(QWidget*);
	~ColorMapsWidget() override;

	QPixmap previewPixmap();
	QString name() const;
	QVector<QColor> colors() const;
	void render(QPixmap&, const QString& name);

private:
	Ui::ColorMapsWidget ui;
	QMap<QString, QString> m_collections; //collections (key = collection name, value = description)
	QMap<QString, QStringList> m_colorMaps; //color maps in a collection (key = collection name, value = list of color map names)
	QMap<QString, QStringList> m_colors; //colors (key = color map name, value = list of colors in the string representation)
	QCompleter* m_completer{nullptr};
	QString m_jsonDir;
	QPixmap m_pixmap;
	QVector<QColor> m_colormap;
	QStandardItemModel* m_model{nullptr};

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
};

#endif // COLORMAPSWIDGET_H
