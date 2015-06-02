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

#include "pagebreakentry.h"

#include <QTextCursor>
#include <QTextCharFormat>
#include <QPalette>
#include <KColorScheme>
#include <KLocale>

PageBreakEntry::PageBreakEntry(Worksheet* worksheet)
  : WorksheetEntry(worksheet)
{
    m_msgItem = new WorksheetTextItem(this);

    QTextCursor cursor = m_msgItem->textCursor();
    KColorScheme color = KColorScheme(QPalette::Normal, KColorScheme::View);
    QTextCharFormat cformat(cursor.charFormat());
    cformat.setForeground(color.foreground(KColorScheme::InactiveText));

    cursor.insertText(i18n("--- Page Break ---"), cformat);

    setFlag(QGraphicsItem::ItemIsFocusable);
}

PageBreakEntry::~PageBreakEntry()
{
}

bool PageBreakEntry::isEmpty()
{
    return false;
}

int PageBreakEntry::type() const
{
    return Type;
}

void PageBreakEntry::populateMenu(QMenu *menu, const QPointF& pos)
{
    WorksheetEntry::populateMenu(menu, pos);
}

bool PageBreakEntry::acceptRichText()
{
    return false;
}

void PageBreakEntry::setContent(const QString& content)
{
    Q_UNUSED(content);
    return;
}

void PageBreakEntry::setContent(const QDomElement& content, const KZip& file)
{
    Q_UNUSED(content);
    Q_UNUSED(file);
    return;
}

QDomElement PageBreakEntry::toXml(QDomDocument& doc, KZip* archive)
{
    Q_UNUSED(archive);

    QDomElement pgbrk = doc.createElement(QLatin1String("PageBreak"));
    return pgbrk;
}

QString PageBreakEntry::toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq)
{
    Q_UNUSED(commandSep);

    return commentStartingSeq + QLatin1String("page break") + commentEndingSeq;
}

void PageBreakEntry::interruptEvaluation()
{
    return;
}

void PageBreakEntry::layOutForWidth(qreal w, bool force)
{
    if (size().width() == w && !force)
        return;

    if (m_msgItem->isVisible()) {
        m_msgItem->setGeometry(0, 0, w, true);

        setSize(QSizeF(m_msgItem->width(), m_msgItem->height() + VerticalMargin));
    } else {
        setSize(QSizeF(w, 0));
    }
}

bool PageBreakEntry::evaluate(EvaluationOption evalOp)
{
    evaluateNext(evalOp);
    return true;
}

void PageBreakEntry::updateEntry()
{
    if (worksheet()->isPrinting()) {
        m_msgItem->setVisible(false);
        recalculateSize();
    } else if (!m_msgItem->isVisible()) {
        m_msgItem->setVisible(true);
        recalculateSize();
    }
    return;
}

/*
void PageBreakEntry::paint(QPainter* painter, const QStyleOptionGraphicsItem*,
                           QWidget*)
{
    if (worksheet()->isPrinting()) {
        QPaintDevice* device = painter->paintEngine()->paintDevice();
        QPrinter* printer = qobject_cast<QPrinter*>(device);
        if (printer)
            printer->newPage();
    }
}
*/

bool PageBreakEntry::wantToEvaluate()
{
    return false;
}

bool PageBreakEntry::wantFocus()
{
    return false;
}


