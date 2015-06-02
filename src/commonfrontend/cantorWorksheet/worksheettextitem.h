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


#ifndef WORKSHEET_TEXT_ITEM_H
#define WORKSHEET_TEXT_ITEM_H

#include <QGraphicsTextItem>
#include <QTextDocument>
#include <QTextCursor>

#include <QMenu>
#include <KStandardAction>

class Worksheet;
class WorksheetView;
class WorksheetCursor;

namespace Cantor {
    class Session;
}

class QTextCharFormat;

class WorksheetTextItem : public QGraphicsTextItem
{
  Q_OBJECT
  public:
    explicit WorksheetTextItem(QGraphicsObject* parent,
                      Qt::TextInteractionFlags ti = Qt::NoTextInteraction);
    ~WorksheetTextItem();

    void setCursorPosition(const QPointF& pos);
    QPointF cursorPosition() const;
    QTextCursor cursorForPosition(const QPointF& pos) const;
    QRectF sceneCursorRect(QTextCursor cursor = QTextCursor()) const;
    QRectF cursorRect(QTextCursor cursor = QTextCursor()) const;

    enum {TopLeft, BottomRight, TopCoord, BottomCoord};
    enum {Type = UserType + 100};

    int type() const;

    void setFocusAt(int pos = TopLeft, qreal xCoord = 0);

    void enableCompletion(bool b);
    void activateCompletion(bool b);
    void setItemDragable(bool b);
    void enableRichText(bool b);

    virtual void populateMenu(QMenu *menu, const QPointF& pos);
    QString resolveImages(const QTextCursor& cursor);

    bool isEditable();
    bool richTextEnabled();
    double width() const;
    double height() const;
    virtual qreal setGeometry(qreal x, qreal y, qreal w, bool centered=false);

    Worksheet* worksheet();
    WorksheetView* worksheetView();

    void clearSelection();

    bool isUndoAvailable();
    bool isRedoAvailable();
    bool isCutAvailable();
    bool isCopyAvailable();
    bool isPasteAvailable();

    // richtext stuff
    void setTextForegroundColor();
    void setTextBackgroundColor();
    void setTextBold(bool b);
    void setTextItalic(bool b);
    void setTextUnderline(bool b);
    void setTextStrikeOut(bool b);
    void setAlignment(Qt::Alignment a);
    void setFontFamily(const QString& font);
    void setFontSize(int size);

    QTextCursor search(QString pattern,
                       QTextDocument::FindFlags qt_flags,
                       const WorksheetCursor& pos);

  Q_SIGNALS:
    void moveToPrevious(int pos, qreal xCoord);
    void moveToNext(int pos, qreal xCoord);
    void cursorPositionChanged(QTextCursor);
    void receivedFocus(WorksheetTextItem*);
    void tabPressed();
    void backtabPressed();
    void applyCompletion();
    void doubleClick();
    void execute();
    void deleteEntry();
    void sizeChanged();
    void menuCreated(QMenu*, const QPointF&);
    void drag(const QPointF&, const QPointF&);
    void undoAvailable(bool);
    void redoAvailable(bool);
    void cutAvailable(bool);
    void copyAvailable(bool);
    void pasteAvailable(bool);

  public Q_SLOTS:
    void insertTab();
    void cut();
    void copy();
    void paste();
    void undo();
    void redo();
    void clipboardChanged();
    void selectionChanged();

  protected:
    void keyPressEvent(QKeyEvent *event);
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void dragEnterEvent(QGraphicsSceneDragDropEvent* event);
    //void dragLeaveEvent(QGraphicsSceneDragDropEvent* event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent* event);
    void dropEvent(QGraphicsSceneDragDropEvent* event);
    bool sceneEvent(QEvent *event);

  private Q_SLOTS:
    //void setHeight();
    void testSize();
    void updateRichTextActions(QTextCursor cursor);

  private:
    void setLocalCursorPosition(const QPointF& pos);
    QPointF localCursorPosition() const;

    QKeyEvent* eventForStandardAction(KStandardAction::StandardAction actionID);
    Cantor::Session* session();

    // richtext
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);

  private:
    QSizeF m_size;
    qreal m_maxWidth;
    bool m_completionEnabled;
    bool m_completionActive;
    bool m_itemDragable;
    bool m_richTextEnabled;
};

#endif // WORKSHEET_TEXT_ITEM_H
