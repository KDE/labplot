#ifndef CUSTOMITEMWIDGET_H
#define CUSTOMITEMWIDGET_H

#include "ui_customitemwidget.h"
#include "backend/worksheet/CustomItem.h"

class CustomItemWidget: public QWidget{
	Q_OBJECT

public:
    explicit CustomItemWidget(QWidget *);

    void setCustomItems(QList<CustomItem*>);
	void load();

    void hidePositionWidgets();
    void updateItemList(QList<CustomItem*>);

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

    void customItemPositionChanged(const CustomItem::PositionWrapper&);
    void customItemStyleChanged(CustomItem::ItemsStyle);
    void customItemSizeChanged(qreal);
    void customItemRotationAngleChanged(qreal);
    void customItemOpacityChanged(qreal);
    void customItemBrushChanged(QBrush);
    void customItemPenChanged(const QPen&);
    void customItemVisibleChanged(bool);

    void loadConfigFromTemplate(KConfig&);
    void loadConfig(KConfig&);
    void saveConfig(KConfig&);
};

#endif
