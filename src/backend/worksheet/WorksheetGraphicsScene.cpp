/***************************************************************************
    File                 : WorksheetGraphicsScene.cpp
    Project              : LabPlot/SciDAVis
    Description          : A QGraphicsScene with project specific extensions.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
                           (replace * with @ in the email addresses) 
                           
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

#include "worksheet/WorksheetGraphicsScene.h"
#include "worksheet/WorksheetElementContainer.h"
#include <QGraphicsItem>
#include <QPainter>

/**
 * \class WorksheetGraphicsScene
 * \brief A QGraphicsScene with project specific extensions.
 *
 * 
 */

WorksheetGraphicsScene::WorksheetGraphicsScene()
	: QGraphicsScene(NULL) {
}

WorksheetGraphicsScene::~WorksheetGraphicsScene() {
}

void WorksheetGraphicsScene::drawItems(QPainter *painter, int numItems, QGraphicsItem *items[], 
		const QStyleOptionGraphicsItem options[], QWidget *widget) {
	
	painter->setClipRect(sceneRect());
	QGraphicsScene::drawItems(painter, numItems, items, options, widget);
}


void WorksheetGraphicsScene::addItem(QGraphicsItem* item){
//   WorksheetElementContainerPrivate* containerItem = qobject_cast<WorksheetElementContainerPrivate*>(item);
//   if (containerItem)
// 	connect(item, SIGNAL(selectedChange(QGraphicsItem*)),
//                     this, SIGNAL(itemSelected(QGraphicsItem*)));

QGraphicsScene::addItem(item);
}

/*
 void DiagramScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
 {
     if (mouseEvent->button() != Qt::LeftButton)
         return;

     DiagramItem *item;
     switch (myMode) {
         case InsertItem:
             item = new DiagramItem(myItemType, myItemMenu);
             item->setBrush(myItemColor);
             addItem(item);
             item->setPos(mouseEvent->scenePos());
             emit itemInserted(item);
             break;
         case InsertLine:
             line = new QGraphicsLineItem(QLineF(mouseEvent->scenePos(),
                                         mouseEvent->scenePos()));
             line->setPen(QPen(myLineColor, 2));
             addItem(line);
             break;
         case InsertText:
             textItem = new DiagramTextItem();
             textItem->setFont(myFont);
             textItem->setTextInteractionFlags(Qt::TextEditorInteraction);
             textItem->setZValue(1000.0);
             connect(textItem, SIGNAL(lostFocus(DiagramTextItem*)),
                     this, SLOT(editorLostFocus(DiagramTextItem*)));
             connect(textItem, SIGNAL(selectedChange(QGraphicsItem*)),
                     this, SIGNAL(itemSelected(QGraphicsItem*)));
             addItem(textItem);
             textItem->setDefaultTextColor(myTextColor);
             textItem->setPos(mouseEvent->scenePos());
             emit textInserted(textItem);
     default:
         ;
     }
     QGraphicsScene::mousePressEvent(mouseEvent);
 }

 void DiagramScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
 {
     if (myMode == InsertLine && line != 0) {
         QLineF newLine(line->line().p1(), mouseEvent->scenePos());
         line->setLine(newLine);
     } else if (myMode == MoveItem) {
         QGraphicsScene::mouseMoveEvent(mouseEvent);
     }
 }

 void DiagramScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
 {
     if (line != 0 && myMode == InsertLine) {
         QList<QGraphicsItem *> startItems = items(line->line().p1());
         if (startItems.count() && startItems.first() == line)
             startItems.removeFirst();
         QList<QGraphicsItem *> endItems = items(line->line().p2());
         if (endItems.count() && endItems.first() == line)
             endItems.removeFirst();

         removeItem(line);
         delete line;

         if (startItems.count() > 0 && endItems.count() > 0 &&
             startItems.first()->type() == DiagramItem::Type &&
             endItems.first()->type() == DiagramItem::Type &&
             startItems.first() != endItems.first()) {
             DiagramItem *startItem =
                 qgraphicsitem_cast<DiagramItem *>(startItems.first());
             DiagramItem *endItem =
                 qgraphicsitem_cast<DiagramItem *>(endItems.first());
             Arrow *arrow = new Arrow(startItem, endItem);
             arrow->setColor(myLineColor);
             startItem->addArrow(arrow);
             endItem->addArrow(arrow);
             arrow->setZValue(-1000.0);
             addItem(arrow);
             arrow->updatePosition();
         }
     }
     line = 0;
     QGraphicsScene::mouseReleaseEvent(mouseEvent);
 }

 bool DiagramScene::isItemChange(int type)
 {
     foreach (QGraphicsItem *item, selectedItems()) {
         if (item->type() == type)
             return true;
     }
     return false;
 }
 */