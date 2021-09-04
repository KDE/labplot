/*
    File                 : DatapickerPoint.h
    Project              : LabPlot
    Description          : Graphic Item for coordinate points of Datapicker
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Ankit Wagadre <wagadre.ankit@gmail.com>
    SPDX-FileCopyrightText: 2015-2019 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DATAPICKERPOINT_H
#define DATAPICKERPOINT_H

#include "backend/datapicker/DatapickerCurve.h"
#include "backend/lib/macros.h"

#include <QGraphicsItem>

//TODO: own file
class ErrorBarItem : public QObject, public QGraphicsRectItem {
	Q_OBJECT

public:
	enum class ErrorBarType {PlusDeltaX, MinusDeltaX, PlusDeltaY, MinusDeltaY};

	explicit ErrorBarItem(DatapickerPoint* parent = nullptr, ErrorBarType type = ErrorBarType::PlusDeltaX);
	void setRectSize(const qreal);

public slots:
	void setPosition(QPointF);

private:
	void initRect();
	void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
	void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
	QVariant itemChange(GraphicsItemChange, const QVariant &value) override;

	QGraphicsLineItem* barLineItem;
	QRectF m_rect;
	ErrorBarType m_type;
	DatapickerPoint* m_parentItem;
};

class DatapickerPointPrivate;
class DatapickerPoint : public AbstractAspect {
	Q_OBJECT

public:
	explicit DatapickerPoint(const QString& name);
	~DatapickerPoint() override;

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QGraphicsItem* graphicsItem() const;
	void setParentGraphicsItem(QGraphicsItem*);
	void setPrinting(bool);
	void initErrorBar(DatapickerCurve::Errors);

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	BASIC_D_ACCESSOR_DECL(QPointF, position, Position)
	BASIC_D_ACCESSOR_DECL(QPointF, plusDeltaXPos, PlusDeltaXPos)
	BASIC_D_ACCESSOR_DECL(QPointF, minusDeltaXPos, MinusDeltaXPos)
	BASIC_D_ACCESSOR_DECL(QPointF, plusDeltaYPos, PlusDeltaYPos)
	BASIC_D_ACCESSOR_DECL(QPointF, minusDeltaYPos, MinusDeltaYPos)

	typedef DatapickerPointPrivate Private;

public slots:
	void retransform();

protected:
	DatapickerPointPrivate* const d_ptr;
	DatapickerPoint(const QString &name, DatapickerPointPrivate *dd);
	static QPen selectedPen;
	static float selectedOpacity;

private:
	Q_DECLARE_PRIVATE(DatapickerPoint)
	void init();

	QList<ErrorBarItem*> m_errorBarItemList;

signals:
	void positionChanged(QPointF);
	void plusDeltaXPosChanged(QPointF);
	void minusDeltaXPosChanged(QPointF);
	void plusDeltaYPosChanged(QPointF);
	void minusDeltaYPosChanged(QPointF);
};

#endif
