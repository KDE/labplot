/***************************************************************************
    File                 : CustomItemWidget.h
    Project              : LabPlot
    Description          : widget for Custom-Item properties
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)

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

#ifndef CUSTOMITEMWIDGET_H
#define CUSTOMITEMWIDGET_H

#include "ui_customitemwidget.h"
#include "backend/datapicker/CustomItem.h"

class CustomItemWidget: public QWidget{
	Q_OBJECT

public:
    explicit CustomItemWidget(QWidget *);

    void setCustomItems(QList<CustomItem*>);
	void load();

    void hidePositionWidgets();

private:
	Ui::CustomItemWidget ui;
	CustomItem *m_item;
    QList<CustomItem*> m_itemList;
	bool m_initializing;

    void init();
	void initConnections();

signals:
	void dataChanged(bool);

private slots:
	void positionXChanged(int);
	void positionYChanged(int);
	void customPositionXChanged(double);
	void customPositionYChanged(double);
    void styleChanged(int);
    void sizeChanged(double);
    void rotationChanged(int);
    void opacityChanged(int);
    void fillingStyleChanged(int);
    void fillingColorChanged(const QColor&);
    void borderStyleChanged(int);
    void borderColorChanged(const QColor&);
    void borderWidthChanged(double);
    void visibilityChanged(bool);
    void errorBarFillingStyleChanged(int);
    void errorBarFillingColorChanged(const QColor&);
    void errorBarSizeChanged(double);

    void customItemPositionChanged(const CustomItem::PositionWrapper&);
    void customItemStyleChanged(CustomItem::ItemsStyle);
    void customItemSizeChanged(qreal);
    void customItemRotationAngleChanged(qreal);
    void customItemOpacityChanged(qreal);
    void customItemBrushChanged(QBrush);
    void customItemPenChanged(const QPen&);
    void customItemVisibleChanged(bool);
    void customItemErrorBarSizeChanged(qreal);
    void customItemErrorBarBrushChanged(QBrush);

    void loadConfigFromTemplate(KConfig&);
    void loadConfig(KConfig&);
    void saveConfig(KConfig&);
};

#endif
