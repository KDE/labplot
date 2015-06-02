/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2012 Martin Kuettler <martin.kuettler@gmail.com>
 */

#include "resultitem.h"
#include "textresultitem.h"
#include "imageresultitem.h"
#include "animationresultitem.h"
#include "worksheetentry.h"

#include "cantor/result.h"
#include "cantor/textresult.h"
#include "cantor/latexresult.h"
#include "cantor/imageresult.h"
#include "cantor/epsresult.h"
#include "cantor/animationresult.h"

#include <QObject>

#include <QIcon>
#include <KLocale>
#include <QDebug>

ResultItem::ResultItem()
{
}

ResultItem::~ResultItem()
{
}

ResultItem* ResultItem::create(WorksheetEntry* parent, Cantor::Result* result)
{
    switch(result->type()) {
    case Cantor::TextResult::Type:
    case Cantor::LatexResult::Type:
        {
            TextResultItem* item = new TextResultItem(parent);
            item->updateFromResult(result);
            return item;
        }
    case Cantor::ImageResult::Type:
    case Cantor::EpsResult::Type:
        {
            ImageResultItem* item = new ImageResultItem(parent);
            item->updateFromResult(result);
            return item;
        }
    case Cantor::AnimationResult::Type:
        {
            AnimationResultItem* item = new AnimationResultItem(parent);
            item->updateFromResult(result);
            return item;
        }
    default:
        return 0;
    }
}

void ResultItem::addCommonActions(QObject* self, QMenu* menu)
{
    menu->addAction(i18n("Save result"), self, SLOT(saveResult()));
    menu->addAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Remove result"),
                    self, SIGNAL(removeResult()));
}

QGraphicsObject* ResultItem::graphicsObject()
{
    return dynamic_cast<QGraphicsObject*>(this);
}
