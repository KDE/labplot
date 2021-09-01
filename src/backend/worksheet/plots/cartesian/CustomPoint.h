/*
    File                 : CustomPoint.h
    Project              : LabPlot
    Description          : Custom user-defined point on the plot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Ankit Wagadre (wagadre.ankit@gmail.com)
    SPDX-FileCopyrightText: 2015 Alexander Semke (alexander.semke@web.de)
*/
/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef CUSTOMPOINT_H
#define CUSTOMPOINT_H

#include <QPen>

#include "backend/lib/macros.h"
#include "backend/worksheet/WorksheetElement.h"

class CartesianPlot;
class CustomPointPrivate;
class Symbol;
class QBrush;

class CustomPoint : public WorksheetElement {
	Q_OBJECT

public:
	explicit CustomPoint(CartesianPlot*, const QString&);
	~CustomPoint() override;

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QGraphicsItem* graphicsItem() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	BASIC_D_ACCESSOR_DECL(QPointF, position, Position)
	Symbol* symbol() const;

	void setVisible(bool on) override;
	bool isVisible() const override;
	void setParentGraphicsItem(QGraphicsItem* item);

	void retransform() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;

	typedef CustomPointPrivate Private;

private slots:
	void visibilityChanged();

protected:
	CustomPointPrivate* const d_ptr;
	CustomPoint(const QString& name, CustomPointPrivate* dd);

private:
	Q_DECLARE_PRIVATE(CustomPoint)
	void init();
	void initActions();

	QAction* visibilityAction;

signals:
	friend class CustomPointSetPositionCmd;
	void positionChanged(const QPointF&);
	void visibleChanged(bool);
	void changed();
};

#endif
