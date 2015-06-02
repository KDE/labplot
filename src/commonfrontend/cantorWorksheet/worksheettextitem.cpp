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

#include "worksheettextitem.h"
#include "worksheet.h"
#include "worksheetentry.h"
#include "epsrenderer.h"
#include "worksheetcursor.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextLine>
#include <QGraphicsSceneResizeEvent>
#include <QtGlobal>

#include <QDebug>
#include <KGlobalSettings>
#include <KStandardAction>
#include <QAction>
#include <KColorDialog>
#include <KColorScheme>
#include <QFontDatabase>

WorksheetTextItem::WorksheetTextItem(QGraphicsObject* parent, Qt::TextInteractionFlags ti)
    : QGraphicsTextItem(parent)
{
    setTextInteractionFlags(ti);
    if (ti & Qt::TextEditable) {
        setCursor(Qt::IBeamCursor);
        connect(this, SIGNAL(sizeChanged()), parent,
                SLOT(recalculateSize()));
    }
    m_completionEnabled = false;
    m_completionActive = false;
    m_itemDragable = false;
    m_richTextEnabled = false;
    m_size = document()->size();;
    m_maxWidth = -1;
    setAcceptDrops(true);
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    connect(document(), SIGNAL(contentsChanged()), this, SLOT(testSize()));
    connect(this, SIGNAL(menuCreated(QMenu*, const QPointF&)), parent,
            SLOT(populateMenu(QMenu*, const QPointF&)), Qt::DirectConnection);
    connect(this, SIGNAL(deleteEntry()), parent, SLOT(startRemoving()));
    connect(this, &WorksheetTextItem::cursorPositionChanged, this, &WorksheetTextItem::updateRichTextActions);
    connect(document(), SIGNAL(undoAvailable(bool)),
            this, SIGNAL(undoAvailable(bool)));
    connect(document(), SIGNAL(redoAvailable(bool)),
            this, SIGNAL(redoAvailable(bool)));
}

WorksheetTextItem::~WorksheetTextItem()
{
    if (worksheet() && this == worksheet()->lastFocusedTextItem())
        worksheet()->updateFocusedTextItem(0);
    if (worksheet() && m_maxWidth > 0 && width() > m_maxWidth)
        worksheet()->removeProtrusion(width() - m_maxWidth);
}

int WorksheetTextItem::type() const
{
    return Type;
}

/*
void WorksheetTextItem::setHeight()
{
    m_height = height();
}
*/

void WorksheetTextItem::testSize()
{
    qreal h = document()->size().height();
    if (h != m_size.height()) {
        emit sizeChanged();
        m_size.setHeight(h);
    }

    qreal w = document()->size().width();
    if (w != m_size.width()) {
        if (m_maxWidth > 0) {
            qreal oldDiff = m_size.width() - m_maxWidth;
            qreal newDiff = w - m_maxWidth;
            if (w > m_maxWidth) {
                if (m_size.width() > m_maxWidth)
                    worksheet()->updateProtrusion(oldDiff, newDiff);
                else
                    worksheet()->addProtrusion(newDiff);
            } else if (m_size.width() > m_maxWidth) {
                worksheet()->removeProtrusion(oldDiff);
            }
        }
        m_size.setWidth(w);
    }
}

qreal WorksheetTextItem::setGeometry(qreal x, qreal y, qreal w, bool centered)
{
    if (m_size.width() < w && centered)
        setPos(x + w/2 - m_size.width()/2, y);
    else
        setPos(x,y);

    qreal oldDiff = 0;
    if (m_maxWidth > 0 && width() > m_maxWidth)
        oldDiff = width() - m_maxWidth;
    m_maxWidth = w;
    setTextWidth(w);
    m_size = document()->size();

    if (oldDiff) {
        if (m_size.width() > m_maxWidth) {
            qreal newDiff = m_size.width() - m_maxWidth;
            worksheet()->updateProtrusion(oldDiff, newDiff);
        } else {
            worksheet()->removeProtrusion(oldDiff);
        }
    } else if (m_size.width() > m_maxWidth) {
        qreal newDiff = m_size.width() - m_maxWidth;
        worksheet()->addProtrusion(newDiff);
    }

    return m_size.height();
}

void WorksheetTextItem::populateMenu(QMenu *menu, const QPointF& pos)
{
    qDebug() << "populate Menu";
    QAction * cut = KStandardAction::cut(this, SLOT(cut()), menu);
    QAction * copy = KStandardAction::copy(this, SLOT(copy()), menu);
    QAction * paste = KStandardAction::paste(this, SLOT(paste()), menu);
    if (!textCursor().hasSelection()) {
        cut->setEnabled(false);
        copy->setEnabled(false);
    }
    if (QApplication::clipboard()->text().isEmpty()) {
        paste->setEnabled(false);
    }
    bool actionAdded = false;
    if (isEditable()) {
        menu->addAction(cut);
        actionAdded = true;
    }
    if (!m_itemDragable && (flags() & Qt::TextSelectableByMouse)) {
        menu->addAction(copy);
        actionAdded = true;
    }
    if (isEditable()) {
        menu->addAction(paste);
        actionAdded = true;
    }
    if (actionAdded)
        menu->addSeparator();

    emit menuCreated(menu, mapToParent(pos));
}

QKeyEvent* WorksheetTextItem::eventForStandardAction(KStandardAction::StandardAction actionID)
{
    // there must be a better way to get the shortcut...
    QAction * action = KStandardAction::create(actionID, this, 0, this);
    QKeySequence keySeq = action->shortcut();
    // we do not support key sequences with multiple keys here
    int code = keySeq[0];
    const int ModMask = Qt::ShiftModifier | Qt::ControlModifier |
        Qt::AltModifier | Qt::MetaModifier;
    const int KeyMask = ~ModMask;
    QKeyEvent* event = new QKeyEvent(QEvent::KeyPress, code & KeyMask,
                                     QFlags<Qt::KeyboardModifier>(code & ModMask));
    delete action;
    return event;
}

void WorksheetTextItem::cut()
{
    if (richTextEnabled()) {
        QKeyEvent* event = eventForStandardAction(KStandardAction::Cut);
        QApplication::sendEvent(worksheet(), event);
        delete event;
    } else {
        copy();
        textCursor().removeSelectedText();
    }
}

void WorksheetTextItem::paste()
{
    if (richTextEnabled()) {
        QKeyEvent* event = eventForStandardAction(KStandardAction::Paste);
        QApplication::sendEvent(worksheet(), event);
        delete event;
    } else {
        textCursor().insertText(QApplication::clipboard()->text());
    }
}

void WorksheetTextItem::copy()
{
    if (richTextEnabled()) {
        QKeyEvent* event = eventForStandardAction(KStandardAction::Copy);
        QApplication::sendEvent(worksheet(), event);
        delete event;
    } else {
        if (!textCursor().hasSelection())
            return;
        QApplication::clipboard()->setText(resolveImages(textCursor()));
    }
}

void WorksheetTextItem::undo()
{
    document()->undo();
}

void WorksheetTextItem::redo()
{
    document()->redo();
}

void WorksheetTextItem::clipboardChanged()
{
    if (isEditable())
        emit pasteAvailable(!QApplication::clipboard()->text().isEmpty());
}

void WorksheetTextItem::selectionChanged()
{
    emit copyAvailable(textCursor().hasSelection());
    if (isEditable())
        emit cutAvailable(textCursor().hasSelection());
}

QString WorksheetTextItem::resolveImages(const QTextCursor& cursor)
{
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();

    const QString repl = QString(QChar::ObjectReplacementCharacter);
    QString result;
    QTextCursor cursor1 = textCursor();
    cursor1.setPosition(start);
    QTextCursor cursor2 = document()->find(repl, cursor1);

    for (; !cursor2.isNull() && cursor2.selectionEnd() <= end;
         cursor2 = document()->find(repl, cursor1)) {
        cursor1.setPosition(cursor2.selectionStart(), QTextCursor::KeepAnchor);
        result += cursor1.selectedText();
        QVariant var = cursor2.charFormat().property(EpsRenderer::Delimiter);
        QString delim;
        if (var.isValid())
            delim = qVariantValue<QString>(var);
        else
            delim = QLatin1String("");
        result += delim + qVariantValue<QString>(cursor2.charFormat().property(EpsRenderer::Code)) + delim;
        cursor1.setPosition(cursor2.selectionEnd());
    }

    cursor1.setPosition(end, QTextCursor::KeepAnchor);
    result += cursor1.selectedText();
    return result;
}

void WorksheetTextItem::setCursorPosition(const QPointF& pos)
{
    QTextCursor cursor = cursorForPosition(pos);
    setTextCursor(cursor);
    emit cursorPositionChanged(cursor);
    //setLocalCursorPosition(mapFromParent(pos));
}

QPointF WorksheetTextItem::cursorPosition() const
{
    return mapToParent(localCursorPosition());
}

void WorksheetTextItem::setLocalCursorPosition(const QPointF& pos)
{
    int p = document()->documentLayout()->hitTest(pos, Qt::FuzzyHit);
    QTextCursor cursor = textCursor();
    cursor.setPosition(p);
    setTextCursor(cursor);
    emit cursorPositionChanged(cursor);
}

QPointF WorksheetTextItem::localCursorPosition() const
{
    QTextCursor cursor = textCursor();
    QTextBlock block = cursor.block();
    int p = cursor.position() - block.position();
    QTextLine line = block.layout()->lineForTextPosition(p);
    if (!line.isValid()) // can this happen?
        return block.layout()->position();
    return QPointF(line.cursorToX(p), line.y() + line.height());
}

QRectF WorksheetTextItem::sceneCursorRect(QTextCursor cursor) const
{
    return mapRectToScene(cursorRect(cursor));
}

QRectF WorksheetTextItem::cursorRect(QTextCursor cursor) const
{
    if (cursor.isNull())
        cursor = textCursor();
    QTextCursor startCursor = cursor;
    startCursor.setPosition(cursor.selectionStart());
    QTextBlock block = startCursor.block();
    if (!block.layout())
        return mapRectToScene(boundingRect());
    int p = startCursor.position() - block.position();
    QTextLine line = block.layout()->lineForTextPosition(p);
    QRectF r1(line.cursorToX(p), line.y(), 1, line.height()+line.leading());

    if (!cursor.hasSelection())
        return r1;

    QTextCursor endCursor = cursor;
    endCursor.setPosition(cursor.selectionEnd());
    block = endCursor.block();
    p = endCursor.position() - block.position();
    line = block.layout()->lineForTextPosition(p);
    QRectF r2(line.cursorToX(p), line.y(), 1, line.height()+line.leading());

    if (r1.y() == r2.y())
        return r1.united(r2);
    else
        return QRectF(x(), qMin(r1.y(), r2.y()), boundingRect().width(),
                      qMax(r1.y() + r1.height(), r2.y() + r2.height()));
}

QTextCursor WorksheetTextItem::cursorForPosition(const QPointF& pos) const
{
    QPointF lpos = mapFromParent(pos);
    int p = document()->documentLayout()->hitTest(lpos, Qt::FuzzyHit);
    QTextCursor cursor = textCursor();
    cursor.setPosition(p);
    return cursor;
}

bool WorksheetTextItem::isEditable()
{
    return textInteractionFlags() & Qt::TextEditable;
}

bool WorksheetTextItem::richTextEnabled()
{
    return m_richTextEnabled;
}

void WorksheetTextItem::enableCompletion(bool b)
{
    m_completionEnabled = b;
}

void WorksheetTextItem::activateCompletion(bool b)
{
    m_completionActive = b;
}

void WorksheetTextItem::setItemDragable(bool b)
{
    m_itemDragable = b;
}

void WorksheetTextItem::enableRichText(bool b)
{
    m_richTextEnabled = b;
}

void WorksheetTextItem::setFocusAt(int pos, qreal xCoord)
{
    QTextCursor cursor = textCursor();
    if (pos == TopLeft) {
        cursor.movePosition(QTextCursor::Start);
    } else if (pos == BottomRight) {
        cursor.movePosition(QTextCursor::End);
    } else {
        QTextLine line;
        if (pos == TopCoord) {
            line = document()->firstBlock().layout()->lineAt(0);
        } else {
            QTextLayout* layout = document()->lastBlock().layout();
            qDebug() << document()->blockCount() << "blocks";
            qDebug() << document()->lastBlock().lineCount() << "lines in last block";
            line = layout->lineAt(document()->lastBlock().lineCount()-1);
        }
        qreal x = mapFromScene(xCoord, 0).x();
        int p = line.xToCursor(x);
        cursor.setPosition(p);
        // Hack: The code for selecting the last line above does not work.
        // This is a workaround
        if (pos == BottomCoord)
            while (cursor.movePosition(QTextCursor::Down))
                ;
    }
    setTextCursor(cursor);
    emit cursorPositionChanged(cursor);
    setFocus();
}

Cantor::Session* WorksheetTextItem::session()
{
    return worksheet()->session();
}

void WorksheetTextItem::keyPressEvent(QKeyEvent *event)
{
    if (!isEditable())
        return;

    switch (event->key()) {
    case Qt::Key_Left:
        if (event->modifiers() == Qt::NoModifier && textCursor().atStart()) {
            emit moveToPrevious(BottomRight, 0);
            qDebug()<<"Reached leftmost valid position";
            return;
        }
        break;
    case Qt::Key_Right:
        if (event->modifiers() == Qt::NoModifier && textCursor().atEnd()) {
            emit moveToNext(TopLeft, 0);
            qDebug()<<"Reached rightmost valid position";
            return;
        }
        break;
    case Qt::Key_Up:
        if (event->modifiers() == Qt::NoModifier && !textCursor().movePosition(QTextCursor::Up)) {
            qreal x = mapToScene(localCursorPosition()).x();
            emit moveToPrevious(BottomCoord, x);
            qDebug()<<"Reached topmost valid position" << localCursorPosition().x();
            return;
        }
        break;
    case Qt::Key_Down:
        if (event->modifiers() == Qt::NoModifier && !textCursor().movePosition(QTextCursor::Down)) {
            qreal x = mapToScene(localCursorPosition()).x();
            emit moveToNext(TopCoord, x);
            qDebug()<<"Reached bottommost valid position" << localCursorPosition().x();
            return;
        }
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (event->modifiers() == Qt::NoModifier && m_completionActive) {
            emit applyCompletion();
            return;
        }
        break;
    case Qt::Key_Tab:
        qDebug() << "Tab";
        break;
    default:
        break;
    }
    int p = textCursor().position();
    bool b = textCursor().hasSelection();
    QGraphicsTextItem::keyPressEvent(event);
    if (p != textCursor().position())
        emit cursorPositionChanged(textCursor());
    if (b != textCursor().hasSelection())
        selectionChanged();
}

bool WorksheetTextItem::sceneEvent(QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        // QGraphicsTextItem's TabChangesFocus feature prevents calls to
        // keyPressEvent for Tab, even when it's turned off. So we got to catch
        // that here.
        QKeyEvent* kev = dynamic_cast<QKeyEvent*>(event);
        if (kev->key() == Qt::Key_Tab && kev->modifiers() == Qt::NoModifier) {
            emit tabPressed();
            return true;
        } else if ((kev->key() == Qt::Key_Tab &&
                    kev->modifiers() == Qt::ShiftModifier) ||
                   kev->key() == Qt::Key_Backtab) {
            emit backtabPressed();
            return true;
        }
    } else if (event->type() == QEvent::ShortcutOverride) {
        QKeyEvent* kev = dynamic_cast<QKeyEvent*>(event);
        QKeySequence seq(kev->key() + kev->modifiers());
        if (worksheet()->isShortcut(seq)) {
            qDebug() << "ShortcutOverride" << kev->key() << kev->modifiers();
            kev->ignore();
            return false;
        }
    }
    return QGraphicsTextItem::sceneEvent(event);
}

void WorksheetTextItem::focusInEvent(QFocusEvent *event)
{
    QGraphicsTextItem::focusInEvent(event);
    //parentItem()->ensureVisible(QRectF(), 0, 0);
    WorksheetEntry* entry = qobject_cast<WorksheetEntry*>(parentObject());
    WorksheetCursor c(entry, this, textCursor());
    worksheet()->makeVisible(c);
    worksheet()->updateFocusedTextItem(this);
    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this,
            SLOT(clipboardChanged()));
    emit receivedFocus(this);
    emit cursorPositionChanged(textCursor());
}

void WorksheetTextItem::focusOutEvent(QFocusEvent *event)
{
    QGraphicsTextItem::focusOutEvent(event);
    emit cursorPositionChanged(QTextCursor());
}

void WorksheetTextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    int p = textCursor().position();
    bool b = textCursor().hasSelection();

    QGraphicsTextItem::mousePressEvent(event);

    if (isEditable() && event->button() == Qt::MiddleButton &&
        QApplication::clipboard()->supportsSelection() &&
        !event->isAccepted())
        event->accept();

    if (m_itemDragable && event->button() == Qt::LeftButton)
        event->accept();

    if (p != textCursor().position())
        emit cursorPositionChanged(textCursor());
    if (b != textCursor().hasSelection())
        selectionChanged();
}

void WorksheetTextItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    const QPointF buttonDownPos = event->buttonDownPos(Qt::LeftButton);
    if (m_itemDragable && event->buttons() == Qt::LeftButton &&
        contains(buttonDownPos) &&
        (event->pos() - buttonDownPos).manhattanLength() >= QApplication::startDragDistance()) {
        ungrabMouse();
        emit drag(mapToParent(buttonDownPos), mapToParent(event->pos()));
        event->accept();
    } else {
        bool b = textCursor().hasSelection();
        QGraphicsTextItem::mouseMoveEvent(event);
        if (b != textCursor().hasSelection())
            selectionChanged();
    }
}

void WorksheetTextItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    int p = textCursor().position();

    // custom middle-click paste that does not copy rich text
    if (isEditable() && event->button() == Qt::MiddleButton &&
        QApplication::clipboard()->supportsSelection() &&
        !richTextEnabled()) {
        setLocalCursorPosition(mapFromScene(event->scenePos()));
        const QString& text = QApplication::clipboard()->text(QClipboard::Selection);
        textCursor().insertText(text);
    } else {
        QGraphicsTextItem::mouseReleaseEvent(event);
    }

    if (p != textCursor().position())
        emit cursorPositionChanged(textCursor());
}

void WorksheetTextItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QTextCursor cursor = textCursor();
    const QChar repl = QChar::ObjectReplacementCharacter;

    if (!cursor.hasSelection()) {
        // We look at the current cursor and the next cursor for a
        // ObjectReplacementCharacter
        for (int i = 2; i; --i) {
            if (document()->characterAt(cursor.position()-1) == repl) {
                setTextCursor(cursor);
                emit doubleClick();
                return;
            }
            cursor.movePosition(QTextCursor::NextCharacter);
        }
    } else if (cursor.selectedText().contains(repl)) {
        emit doubleClick();
        return;
    }

    QGraphicsTextItem::mouseDoubleClickEvent(event);
}

void WorksheetTextItem::dragEnterEvent(QGraphicsSceneDragDropEvent* event)
{
    if (isEditable() && event->mimeData()->hasFormat(QLatin1String("text/plain"))) {
        if (event->proposedAction() & (Qt::CopyAction | Qt::MoveAction)) {
            event->acceptProposedAction();
        } else if (event->possibleActions() & Qt::CopyAction) {
            event->setDropAction(Qt::CopyAction);
            event->accept();
        } else if (event->possibleActions() & Qt::MoveAction) {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        event->ignore();
    }
}

void WorksheetTextItem::dragMoveEvent(QGraphicsSceneDragDropEvent* event)
{
    if (isEditable() && event->mimeData()->hasFormat(QLatin1String("text/plain")))
        setLocalCursorPosition(mapFromScene(event->scenePos()));
}

void WorksheetTextItem::dropEvent(QGraphicsSceneDragDropEvent* event)
{
    if (isEditable()) {
        if (richTextEnabled() && event->mimeData()->hasFormat(QLatin1String("text/html")))
            textCursor().insertHtml(event->mimeData()->html());
        else
            textCursor().insertText(event->mimeData()->text());
        event->accept();
    }
}

void WorksheetTextItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu *menu = worksheet()->createContextMenu();
    populateMenu(menu, event->pos());

    menu->popup(event->screenPos());
}

void WorksheetTextItem::insertTab()
{
    QTextCursor cursor = textCursor();
    cursor.clearSelection();
    cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
    QString sel = cursor.selectedText();
    bool spacesOnly = true;
    qDebug() << sel;
    for (QString::iterator it = sel.begin(); it != sel.end(); ++it) {
        if (! it->isSpace()) {
            spacesOnly = false;
            break;
        }
    }

    cursor.setPosition(cursor.selectionEnd());
    if (spacesOnly) {
        while (document()->characterAt(cursor.position()) == QLatin1Char(' '))
            cursor.movePosition(QTextCursor::NextCharacter);
    }

    QTextLayout *layout = textCursor().block().layout();
    if (!layout) {
        cursor.insertText(QLatin1String("    "));
    } else {
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        int i = cursor.selectionEnd() - cursor.selectionStart();
        i = ((i+4) & (~3)) - i;
        cursor.setPosition(cursor.selectionEnd());

        QString insertBlankSpace = QLatin1String(" ");
        cursor.insertText(insertBlankSpace.repeated(i));
    }
    setTextCursor(cursor);
    emit cursorPositionChanged(textCursor());
}

double WorksheetTextItem::width() const
{
    return m_size.width();
}

double WorksheetTextItem::height() const
{
    return m_size.height();
}

Worksheet* WorksheetTextItem::worksheet()
{
    return qobject_cast<Worksheet*>(scene());
}

WorksheetView* WorksheetTextItem::worksheetView()
{
    return worksheet()->worksheetView();
}

void WorksheetTextItem::clearSelection()
{
    QTextCursor cursor = textCursor();
    cursor.clearSelection();
    setTextCursor(cursor);
    selectionChanged();
}

bool WorksheetTextItem::isUndoAvailable()
{
    return document()->isUndoAvailable();
}

bool WorksheetTextItem::isRedoAvailable()
{
    return document()->isRedoAvailable();
}

bool WorksheetTextItem::isCutAvailable()
{
    return isEditable() && textCursor().hasSelection();
}

bool WorksheetTextItem::isCopyAvailable()
{
    return !m_itemDragable && textCursor().hasSelection();
}

bool WorksheetTextItem::isPasteAvailable()
{
    return isEditable() && !QApplication::clipboard()->text().isEmpty();
}

QTextCursor WorksheetTextItem::search(QString pattern,
                                      QTextDocument::FindFlags qt_flags,
                                      const WorksheetCursor& pos)
{
    if (pos.isValid() && pos.textItem() != this)
        return QTextCursor();

    QTextDocument* doc = document();
    QTextCursor cursor;
    if (pos.isValid()) {
        cursor = doc->find(pattern, pos.textCursor(), qt_flags);
    } else {
        cursor = textCursor();
        if (qt_flags & QTextDocument::FindBackward)
            cursor.movePosition(QTextCursor::End);
        else
            cursor.movePosition(QTextCursor::Start);
        cursor = doc->find(pattern, cursor, qt_flags);
    }

    return cursor;
}

// RichText

void WorksheetTextItem::updateRichTextActions(QTextCursor cursor)
{
    if (cursor.isNull())
        return;
    Worksheet::RichTextInfo info;
    QTextCharFormat fmt = cursor.charFormat();
    info.bold = (fmt.fontWeight() == QFont::Bold);
    info.italic = fmt.fontItalic();
    info.underline = fmt.fontUnderline();
    info.strikeOut = fmt.fontStrikeOut();
    info.font = fmt.fontFamily();
    info.fontSize = fmt.font().pointSize();

    QTextBlockFormat bfmt = cursor.blockFormat();
    info.align = bfmt.alignment();

    worksheet()->setRichTextInformation(info);
}

void WorksheetTextItem::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    qDebug() << format;
    QTextCursor cursor = textCursor();
    QTextCursor wordStart(cursor);
    QTextCursor wordEnd(cursor);

    wordStart.movePosition(QTextCursor::StartOfWord);
    wordEnd.movePosition(QTextCursor::EndOfWord);

    //cursor.beginEditBlock();
    if (!cursor.hasSelection() && cursor.position() != wordStart.position() && cursor.position() != wordEnd.position())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
    //q->mergeCurrentCharFormat(format);
    //cursor.endEditBlock();
    setTextCursor(cursor);
}

void WorksheetTextItem::setTextForegroundColor()
{
    QTextCharFormat fmt = textCursor().charFormat();
    QColor color = fmt.foreground().color();

    int result = KColorDialog::getColor(color, KColorScheme(QPalette::Active, KColorScheme::View).foreground().color(), worksheetView());
    if (!color.isValid())
        color = KColorScheme(QPalette::Active, KColorScheme::View).foreground().color();
    if (result != QDialog::Accepted)
        return;

    QTextCharFormat newFmt;
    newFmt.setForeground(color);
    mergeFormatOnWordOrSelection(newFmt);
}

void WorksheetTextItem::setTextBackgroundColor()
{
    QTextCharFormat fmt = textCursor().charFormat();
    QColor color = fmt.background().color();

    int result = KColorDialog::getColor(color, KColorScheme(QPalette::Active, KColorScheme::View).background().color(), worksheetView());
    if (!color.isValid())
        color = KColorScheme(QPalette::Active, KColorScheme::View).background().color() ;
    if (result != QDialog::Accepted)
        return;

    QTextCharFormat newFmt;
    newFmt.setBackground(color);
    mergeFormatOnWordOrSelection(newFmt);
}

void WorksheetTextItem::setTextBold(bool b)
{
    QTextCharFormat fmt;
    fmt.setFontWeight(b ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(fmt);
}

void WorksheetTextItem::setTextItalic(bool b)
{
    QTextCharFormat fmt;
    fmt.setFontItalic(b);
    mergeFormatOnWordOrSelection(fmt);
}

void WorksheetTextItem::setTextUnderline(bool b)
{
    QTextCharFormat fmt;
    fmt.setFontUnderline(b);
    mergeFormatOnWordOrSelection(fmt);
}

void WorksheetTextItem::setTextStrikeOut(bool b)
{
    QTextCharFormat fmt;
    fmt.setFontStrikeOut(b);
    mergeFormatOnWordOrSelection(fmt);
}

void WorksheetTextItem::setAlignment(Qt::Alignment a)
{
    QTextBlockFormat fmt;
    fmt.setAlignment(a);
    QTextCursor cursor = textCursor();
    cursor.mergeBlockFormat(fmt);
    setTextCursor(cursor);
}


void WorksheetTextItem::setFontFamily(const QString& font)
{
    if (!richTextEnabled())
        return;
    QTextCharFormat fmt;
    fmt.setFontFamily(font);
    mergeFormatOnWordOrSelection(fmt);
}

void WorksheetTextItem::setFontSize(int size)
{
    if (!richTextEnabled())
        return;
    QTextCharFormat fmt;
    fmt.setFontPointSize(size);
    mergeFormatOnWordOrSelection(fmt);
}


