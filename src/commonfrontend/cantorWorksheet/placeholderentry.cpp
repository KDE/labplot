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

#include "placeholderentry.h"

#include <QPropertyAnimation>

PlaceHolderEntry::PlaceHolderEntry(Worksheet* worksheet, QSizeF s)
    : WorksheetEntry(worksheet)
{
    setSize(s);
}

PlaceHolderEntry::~PlaceHolderEntry()
{
}

int PlaceHolderEntry::type() const
{
    return Type;
}

bool PlaceHolderEntry::isEmpty()
{
    /*
    // This is counter-intuitive. isEmpty() is used to find out whether a new
    // CommandEntry needs to be appended, and a PlaceHolderEntry should never
    // prevent that.
    return false;
    */
    return true;
}

bool PlaceHolderEntry::acceptRichText()
{
    return false;
}

void PlaceHolderEntry::setContent(const QString&)
{
}

void PlaceHolderEntry::setContent(const QDomElement&, const KZip&)
{
}

QDomElement PlaceHolderEntry::toXml(QDomDocument&, KZip*)
{
    return QDomElement();
}

QString PlaceHolderEntry::toPlain(const QString&, const QString&, const QString&){
    return QString();
}

void PlaceHolderEntry::interruptEvaluation()
{
    return;
}

void PlaceHolderEntry::layOutForWidth(qreal w, bool force)
{
    Q_UNUSED(w);
    Q_UNUSED(force);
}

bool PlaceHolderEntry::evaluate(EvaluationOption evalOp)
{
    evaluateNext(evalOp);
    return true;
}

void PlaceHolderEntry::updateEntry()
{
}

bool PlaceHolderEntry::wantToEvaluate()
{
    return false;
}

void PlaceHolderEntry::changeSize(QSizeF s)
{
    if (!worksheet()->animationsEnabled()) {
        setSize(s);
        worksheet()->updateEntrySize(this);
        return;
    }
    if (aboutToBeRemoved())
        return;

    if (animationActive())
        endAnimation();

    QPropertyAnimation* sizeAn = sizeChangeAnimation(s);

    sizeAn->setEasingCurve(QEasingCurve::InOutQuad);
    sizeAn->start(QAbstractAnimation::DeleteWhenStopped);
}

