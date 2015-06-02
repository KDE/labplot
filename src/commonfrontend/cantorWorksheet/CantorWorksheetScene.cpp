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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
    Copyright (C) 2012 Martin Kuettler <martin.kuettler@gmail.com>
 */

#include <QGraphicsWidget>
#include <QTextLayout>
#include <QTextDocument>
#include <QTimer>
#include <QDrag>
#include <QPrinter>
#include <QtXml>
#include <QtXmlPatterns/QXmlQuery>
#include <QtXmlPatterns/QXmlNamePool>

#include <KLocale>
#include <QIcon>

#include <KMessageBox>
#include <KActionCollection>
#include <KShortcut>
#include <QAction>
#include <KFontAction>
#include <KFontSizeAction>
#include <KSelectAction>
#include <KToggleAction>
#include <KColorDialog>
#include <KColorScheme>

// #include "config-cantor.h"
#include "CantorWorksheetScene.h"
// #include "settings.h"
#include "commandentry.h"
#include "textentry.h"
#include "latexentry.h"
#include "imageentry.h"
#include "pagebreakentry.h"
#include "placeholderentry.h"
#include "cantor/backend.h"
#include "cantor/extension.h"
#include "cantor/result.h"
#include "cantor/helpresult.h"
#include "cantor/session.h"
#include "cantor/defaulthighlighter.h"

const double CantorWorksheetScene::LeftMargin = 4;
const double CantorWorksheetScene::RightMargin = 4;
const double CantorWorksheetScene::TopMargin = 4;

CantorWorksheetScene::CantorWorksheetScene(Cantor::Backend* backend, QWidget* parent)
    : QGraphicsScene(parent)
{
    m_session = backend->createSession();
    m_highlighter = 0;

    m_firstEntry = 0;
    m_lastEntry = 0;
    m_lastFocusedTextItem = 0;
    m_dragEntry = 0;
    m_placeholderEntry = 0;
    m_viewWidth = 0;
    m_protrusion = 0;
    m_dragScrollTimer = 0;

    m_isPrinting = false;
    m_loginFlag = true;
    QTimer::singleShot(0, this, SLOT(loginToSession()));
}

CantorWorksheetScene::~CantorWorksheetScene()
{
    // This is necessary, because a SeachBar might access firstEntry()
    // while the scene is deleted. Maybe there is a better solution to
    // this problem, but I can't seem to find it.
    m_firstEntry = 0;
    m_session->logout();
}

void CantorWorksheetScene::loginToSession()
{
    if(m_loginFlag==true)
    {
        m_session->login();

        enableHighlighting(true);
        enableCompletion(true);
        enableExpressionNumbering(true);
        enableAnimations(true);
#ifdef WITH_EPS
        session()->setTypesettingEnabled(Settings::self()->typesetDefault());
#else
        session()->setTypesettingEnabled(false);
#endif

        m_loginFlag=false;
    }
}

void CantorWorksheetScene::print(QPrinter* printer)
{
    m_epsRenderer.useHighResolution(true);
    m_isPrinting = true;
    QRect pageRect = printer->pageRect();
    qreal scale = 1; // todo: find good scale for page size
    // todo: use epsRenderer()->scale() for printing ?
    const qreal width = pageRect.width()/scale;
    const qreal height = pageRect.height()/scale;
    setViewSize(width, height, scale, true);

    QPainter painter(printer);
    painter.scale(scale, scale);
    painter.setRenderHint(QPainter::Antialiasing);
    WorksheetEntry* entry = firstEntry();
    qreal y = 0;

    while (entry) {
        qreal h = 0;
        do {
            if (entry->type() == PageBreakEntry::Type) {
                entry = entry->next();
                break;
            }
            h += entry->size().height();
            entry = entry->next();
        } while (entry && h + entry->size().height() <= height);

        render(&painter, QRectF(0, 0, width, height),
               QRectF(0, y, width, h));
        y += h;
        if (entry)
            printer->newPage();
    }

    //render(&painter);

    painter.end();
    m_isPrinting = false;
    m_epsRenderer.useHighResolution(false);
    m_epsRenderer.setScale(-1);  // force update in next call to setViewSize,
    worksheetView()->updateSceneSize(); // ... which happens in here
}

bool CantorWorksheetScene::isPrinting()
{
    return m_isPrinting;
}

void CantorWorksheetScene::setViewSize(qreal w, qreal h, qreal s, bool forceUpdate)
{
    Q_UNUSED(h);

    m_viewWidth = w;
    if (s != m_epsRenderer.scale() || forceUpdate) {
        m_epsRenderer.setScale(s);
        for (WorksheetEntry *entry = firstEntry(); entry; entry = entry->next())
            entry->updateEntry();
    }
    updateLayout();
}

void CantorWorksheetScene::updateLayout()
{
    bool cursorRectVisible = false;
    bool atEnd = worksheetView()->isAtEnd();
    if (currentTextItem()) {
        QRectF cursorRect = currentTextItem()->sceneCursorRect();
        cursorRectVisible = worksheetView()->isVisible(cursorRect);
    }

    const qreal w = m_viewWidth - LeftMargin - RightMargin;
    qreal y = TopMargin;
    const qreal x = LeftMargin;
    for (WorksheetEntry *entry = firstEntry(); entry; entry = entry->next())
        y += entry->setGeometry(x, y, w);
    setSceneRect(QRectF(0, 0, m_viewWidth + m_protrusion, y));
    if (cursorRectVisible)
        makeVisible(worksheetCursor());
    else if (atEnd)
        worksheetView()->scrollToEnd();
}

void CantorWorksheetScene::updateEntrySize(WorksheetEntry* entry)
{
    bool cursorRectVisible = false;
    bool atEnd = worksheetView()->isAtEnd();
    if (currentTextItem()) {
        QRectF cursorRect = currentTextItem()->sceneCursorRect();
        cursorRectVisible = worksheetView()->isVisible(cursorRect);
    }

    qreal y = entry->y() + entry->size().height();
    for (entry = entry->next(); entry; entry = entry->next()) {
        entry->setY(y);
        y += entry->size().height();
    }
    setSceneRect(QRectF(0, 0, m_viewWidth + m_protrusion, y));
    if (cursorRectVisible)
        makeVisible(worksheetCursor());
    else if (atEnd)
        worksheetView()->scrollToEnd();
}

void CantorWorksheetScene::addProtrusion(qreal width)
{
    if (m_itemProtrusions.contains(width))
        ++m_itemProtrusions[width];
    else
        m_itemProtrusions.insert(width, 1);
    if (width > m_protrusion) {
        m_protrusion = width;
        qreal y = lastEntry()->size().height() + lastEntry()->y();
        setSceneRect(QRectF(0, 0, m_viewWidth + m_protrusion, y));
    }
}

void CantorWorksheetScene::updateProtrusion(qreal oldWidth, qreal newWidth)
{
    removeProtrusion(oldWidth);
    addProtrusion(newWidth);
}

void CantorWorksheetScene::removeProtrusion(qreal width)
{
    if (--m_itemProtrusions[width] == 0) {
        m_itemProtrusions.remove(width);
        if (width == m_protrusion) {
            qreal max = -1;
            foreach (qreal p, m_itemProtrusions.keys()) {
                if (p > max)
                    max = p;
            }
            m_protrusion = max;
            qreal y = lastEntry()->size().height() + lastEntry()->y();
            setSceneRect(QRectF(0, 0, m_viewWidth + m_protrusion, y));
        }
    }
}

bool CantorWorksheetScene::isEmpty()
{
    return !m_firstEntry;
}

void CantorWorksheetScene::makeVisible(WorksheetEntry* entry)
{
    QRectF r = entry->boundingRect();
    r = entry->mapRectToScene(r);
    r.adjust(0, -10, 0, 10);
    worksheetView()->makeVisible(r);
}

void CantorWorksheetScene::makeVisible(const WorksheetCursor& cursor)
{
    if (cursor.textCursor().isNull()) {
        if (cursor.entry())
            makeVisible(cursor.entry());
        return;
    }
    QRectF r = cursor.textItem()->sceneCursorRect(cursor.textCursor());
    QRectF er = cursor.entry()->boundingRect();
    er = cursor.entry()->mapRectToScene(er);
    er.adjust(0, -10, 0, 10);
    r.adjust(0, qMax(qreal(-100.0), er.top() - r.top()),
             0, qMin(qreal(100.0), er.bottom() - r.bottom()));
    worksheetView()->makeVisible(r);
}

CantorWorksheetViewHolder* CantorWorksheetScene::worksheetView()
{
    return qobject_cast<CantorWorksheetViewHolder*>(views()[0]);
}

void CantorWorksheetScene::setModified()
{
    emit modified();
}

WorksheetCursor CantorWorksheetScene::worksheetCursor()
{
    WorksheetEntry* entry = currentEntry();
    WorksheetTextItem* item = currentTextItem();

    if (!entry || !item)
        return WorksheetCursor();
    return WorksheetCursor(entry, item, item->textCursor());
}

void CantorWorksheetScene::setWorksheetCursor(const WorksheetCursor& cursor)
{
    if (!cursor.isValid())
        return;

    if (m_lastFocusedTextItem)
        m_lastFocusedTextItem->clearSelection();

    m_lastFocusedTextItem = cursor.textItem();

    cursor.textItem()->setTextCursor(cursor.textCursor());
}

WorksheetEntry* CantorWorksheetScene::currentEntry()
{
    QGraphicsItem* item = focusItem();
    if (!item /*&& !hasFocus()*/)
        item = m_lastFocusedTextItem;
    /*else
      m_focusItem = item;*/
    while (item && (item->type() < QGraphicsItem::UserType ||
                    item->type() >= QGraphicsItem::UserType + 100))
        item = item->parentItem();
    if (item) {
        WorksheetEntry* entry = qobject_cast<WorksheetEntry*>(item->toGraphicsObject());
        if (entry && entry->aboutToBeRemoved()) {
            if (entry->isAncestorOf(m_lastFocusedTextItem))
                m_lastFocusedTextItem = 0;
            return 0;
        }
        return entry;
    }
    return 0;
}

WorksheetEntry* CantorWorksheetScene::firstEntry()
{
    return m_firstEntry;
}

WorksheetEntry* CantorWorksheetScene::lastEntry()
{
    return m_lastEntry;
}

void CantorWorksheetScene::setFirstEntry(WorksheetEntry* entry)
{
    if (m_firstEntry)
        disconnect(m_firstEntry, SIGNAL(aboutToBeDeleted()),
                   this, SLOT(invalidateFirstEntry()));
    m_firstEntry = entry;
    if (m_firstEntry)
        connect(m_firstEntry, SIGNAL(aboutToBeDeleted()),
                this, SLOT(invalidateFirstEntry()), Qt::DirectConnection);
}

void CantorWorksheetScene::setLastEntry(WorksheetEntry* entry)
{
    if (m_lastEntry)
        disconnect(m_lastEntry, SIGNAL(aboutToBeDeleted()),
                   this, SLOT(invalidateLastEntry()));
    m_lastEntry = entry;
    if (m_lastEntry)
        connect(m_lastEntry, SIGNAL(aboutToBeDeleted()),
                this, SLOT(invalidateLastEntry()), Qt::DirectConnection);
}

void CantorWorksheetScene::invalidateFirstEntry()
{
    if (m_firstEntry)
        setFirstEntry(m_firstEntry->next());
}

void CantorWorksheetScene::invalidateLastEntry()
{
    if (m_lastEntry)
        setLastEntry(m_lastEntry->previous());
}

WorksheetEntry* CantorWorksheetScene::entryAt(qreal x, qreal y)
{
    QGraphicsItem* item = itemAt(x, y);
    while (item && (item->type() <= QGraphicsItem::UserType ||
                    item->type() >= QGraphicsItem::UserType + 100))
        item = item->parentItem();
    if (item)
        return qobject_cast<WorksheetEntry*>(item->toGraphicsObject());
    return 0;
}

WorksheetEntry* CantorWorksheetScene::entryAt(QPointF p)
{
    return entryAt(p.x(), p.y());
}

void CantorWorksheetScene::focusEntry(WorksheetEntry *entry)
{
    if (!entry)
        return;
    entry->focusEntry();
    //bool rt = entry->acceptRichText();
    //setActionsEnabled(rt);
    //setAcceptRichText(rt);
    //ensureCursorVisible();
}

void CantorWorksheetScene::startDrag(WorksheetEntry* entry, QDrag* drag)
{
    m_dragEntry = entry;
    WorksheetEntry* prev = entry->previous();
    WorksheetEntry* next = entry->next();
    m_placeholderEntry = new PlaceHolderEntry(this, entry->size());
    m_placeholderEntry->setPrevious(prev);
    m_placeholderEntry->setNext(next);
    if (prev)
        prev->setNext(m_placeholderEntry);
    else
        setFirstEntry(m_placeholderEntry);
    if (next)
        next->setPrevious(m_placeholderEntry);
    else
        setLastEntry(m_placeholderEntry);
    m_dragEntry->hide();
    Qt::DropAction action = drag->exec();

    qDebug() << action;
    if (action == Qt::MoveAction && m_placeholderEntry) {
        qDebug() << "insert in new position";
        prev = m_placeholderEntry->previous();
        next = m_placeholderEntry->next();
    }
    m_dragEntry->setPrevious(prev);
    m_dragEntry->setNext(next);
    if (prev)
        prev->setNext(m_dragEntry);
    else
        setFirstEntry(m_dragEntry);
    if (next)
        next->setPrevious(m_dragEntry);
    else
        setLastEntry(m_dragEntry);
    m_dragEntry->show();
    m_dragEntry->focusEntry();
    const QPointF scenePos = worksheetView()->sceneCursorPos();
    if (entryAt(scenePos) != m_dragEntry)
        m_dragEntry->hideActionBar();
    updateLayout();
    if (m_placeholderEntry) {
        m_placeholderEntry->setPrevious(0);
        m_placeholderEntry->setNext(0);
        m_placeholderEntry->hide();
        m_placeholderEntry->deleteLater();
        m_placeholderEntry = 0;
    }
    m_dragEntry = 0;
}

void CantorWorksheetScene::evaluate()
{
    qDebug()<<"evaluate worksheet";
    firstEntry()->evaluate(WorksheetEntry::EvaluateNext);

    emit modified();
}

void CantorWorksheetScene::evaluateCurrentEntry()
{
    qDebug() << "evaluation requested...";
    WorksheetEntry* entry = currentEntry();
    if(!entry)
        return;
    entry->evaluateCurrentItem();
}

bool CantorWorksheetScene::completionEnabled()
{
    return m_completionEnabled;
}

void CantorWorksheetScene::showCompletion()
{
    WorksheetEntry* current = currentEntry();
    current->showCompletion();
}

WorksheetEntry* CantorWorksheetScene::appendEntry(const int type)
{
    WorksheetEntry* entry = WorksheetEntry::create(type, this);

    if (entry)
    {
        qDebug() << "Entry Appended";
        entry->setPrevious(lastEntry());
        if (lastEntry())
            lastEntry()->setNext(entry);
        if (!firstEntry())
            setFirstEntry(entry);
        setLastEntry(entry);
        updateLayout();
        makeVisible(entry);
        focusEntry(entry);
    }
    return entry;
}

WorksheetEntry* CantorWorksheetScene::appendCommandEntry()
{
   return appendEntry(CommandEntry::Type);
}

WorksheetEntry* CantorWorksheetScene::appendTextEntry()
{
   return appendEntry(TextEntry::Type);
}


WorksheetEntry* CantorWorksheetScene::appendPageBreakEntry()
{
    return appendEntry(PageBreakEntry::Type);
}

WorksheetEntry* CantorWorksheetScene::appendImageEntry()
{
   return appendEntry(ImageEntry::Type);
}

WorksheetEntry* CantorWorksheetScene::appendLatexEntry()
{
    return appendEntry(LatexEntry::Type);
}

void CantorWorksheetScene::appendCommandEntry(const QString& text)
{
    WorksheetEntry* entry = lastEntry();
    if(!entry->isEmpty())
    {
        entry = appendCommandEntry();
    }

    if (entry)
    {
        focusEntry(entry);
        entry->setContent(text);
        evaluateCurrentEntry();
    }
}

WorksheetEntry* CantorWorksheetScene::insertEntry(const int type, WorksheetEntry* current)
{
    if (!current)
        current = currentEntry();

    if (!current)
        return appendEntry(type);

    WorksheetEntry *next = current->next();
    WorksheetEntry *entry = 0;

    if (!next || next->type() != type || !next->isEmpty())
    {
        entry = WorksheetEntry::create(type, this);
        entry->setPrevious(current);
        entry->setNext(next);
        current->setNext(entry);
        if (next)
            next->setPrevious(entry);
        else
            setLastEntry(entry);
        updateLayout();
    } else {
        entry = next;
    }

    focusEntry(entry);
    makeVisible(entry);
    return entry;
}

WorksheetEntry* CantorWorksheetScene::insertTextEntry(WorksheetEntry* current)
{
    return insertEntry(TextEntry::Type, current);
}

WorksheetEntry* CantorWorksheetScene::insertCommandEntry(WorksheetEntry* current)
{
    return insertEntry(CommandEntry::Type, current);
}

WorksheetEntry* CantorWorksheetScene::insertImageEntry(WorksheetEntry* current)
{
    return insertEntry(ImageEntry::Type, current);
}

WorksheetEntry* CantorWorksheetScene::insertPageBreakEntry(WorksheetEntry* current)
{
    return insertEntry(PageBreakEntry::Type, current);
}

WorksheetEntry* CantorWorksheetScene::insertLatexEntry(WorksheetEntry* current)
{
    return insertEntry(LatexEntry::Type, current);
}

void CantorWorksheetScene::insertCommandEntry(const QString& text)
{
    WorksheetEntry* entry = insertCommandEntry();
    if(entry&&!text.isNull())
    {
        entry->setContent(text);
        evaluateCurrentEntry();
    }
}


WorksheetEntry* CantorWorksheetScene::insertEntryBefore(int type, WorksheetEntry* current)
{
    if (!current)
        current = currentEntry();

    if (!current)
        return 0;

    WorksheetEntry *prev = current->previous();
    WorksheetEntry *entry = 0;

    if(!prev || prev->type() != type || !prev->isEmpty())
    {
        entry = WorksheetEntry::create(type, this);
        entry->setNext(current);
        entry->setPrevious(prev);
        current->setPrevious(entry);
        if (prev)
            prev->setNext(entry);
        else
            setFirstEntry(entry);
        updateLayout();
    }

    focusEntry(entry);
    return entry;
}

WorksheetEntry* CantorWorksheetScene::insertTextEntryBefore(WorksheetEntry* current)
{
    return insertEntryBefore(TextEntry::Type, current);
}

WorksheetEntry* CantorWorksheetScene::insertCommandEntryBefore(WorksheetEntry* current)
{
    return insertEntryBefore(CommandEntry::Type, current);
}

WorksheetEntry* CantorWorksheetScene::insertPageBreakEntryBefore(WorksheetEntry* current)
{
    return insertEntryBefore(PageBreakEntry::Type, current);
}

WorksheetEntry* CantorWorksheetScene::insertImageEntryBefore(WorksheetEntry* current)
{
    return insertEntryBefore(ImageEntry::Type, current);
}

WorksheetEntry* CantorWorksheetScene::insertLatexEntryBefore(WorksheetEntry* current)
{
    return insertEntryBefore(LatexEntry::Type, current);
}

void CantorWorksheetScene::interrupt()
{
    m_session->interrupt();
    emit updatePrompt();
}

void CantorWorksheetScene::interruptCurrentEntryEvaluation()
{
    currentEntry()->interruptEvaluation();
}

void CantorWorksheetScene::highlightItem(WorksheetTextItem* item)
{
    if (!m_highlighter)
        return;

    QTextDocument *oldDocument = m_highlighter->document();
    QList<QList<QTextLayout::FormatRange> > formats;

    if (oldDocument)
    {
        for (QTextBlock b = oldDocument->firstBlock();
             b.isValid(); b = b.next())
        {
            formats.append(b.layout()->additionalFormats());
        }
    }

    // Not every highlighter is a Cantor::DefaultHighligther (e.g. the
    // highlighter for KAlgebra)
    Cantor::DefaultHighlighter* hl = qobject_cast<Cantor::DefaultHighlighter*>(m_highlighter);
    if (hl) {
        hl->setTextItem(item);
    } else {
        m_highlighter->setDocument(item->document());
    }

    if (oldDocument)
    {
        QTextCursor cursor(oldDocument);
        cursor.beginEditBlock();
        for (QTextBlock b = oldDocument->firstBlock();
             b.isValid(); b = b.next())
        {
            b.layout()->setAdditionalFormats(formats.first());
            formats.pop_front();
        }
        cursor.endEditBlock();
    }

}

void CantorWorksheetScene::rehighlight()
{
    if(m_highlighter)
    {
        // highlight every entry
        WorksheetEntry* entry;
        for (entry = firstEntry(); entry; entry = entry->next()) {
            WorksheetTextItem* item = entry->highlightItem();
            if (!item)
                continue;
            highlightItem(item);
            m_highlighter->rehighlight();
        }
        entry = currentEntry();
        WorksheetTextItem* textitem = entry ? entry->highlightItem() : 0;
        if (textitem && textitem->hasFocus())
            highlightItem(textitem);
    } else
    {
        // remove highlighting from entries
        WorksheetEntry* entry;
        for (entry = firstEntry(); entry; entry = entry->next()) {
            WorksheetTextItem* item = entry->highlightItem();
            if (!item)
                continue;
            for (QTextBlock b = item->document()->firstBlock();
                 b.isValid(); b = b.next())
            {
                b.layout()->clearAdditionalFormats();
            }
        }
        update();
    }
}

void CantorWorksheetScene::enableHighlighting(bool highlight)
{
    if(highlight)
    {
        if(m_highlighter)
            m_highlighter->deleteLater();
        m_highlighter=session()->syntaxHighlighter(this);
        if(!m_highlighter)
            m_highlighter=new Cantor::DefaultHighlighter(this);

        connect(m_highlighter, SIGNAL(rulesChanged()), this, SLOT(rehighlight()));

    }else
    {
        if(m_highlighter)
            m_highlighter->deleteLater();
        m_highlighter=0;
    }

    rehighlight();
}

void CantorWorksheetScene::enableCompletion(bool enable)
{
    m_completionEnabled=enable;
}

Cantor::Session* CantorWorksheetScene::session()
{
    return m_session;
}

bool CantorWorksheetScene::isRunning()
{
    return m_session->status()==Cantor::Session::Running;
}

bool CantorWorksheetScene::showExpressionIds()
{
    return m_showExpressionIds;
}

bool CantorWorksheetScene::animationsEnabled()
{
    return m_animationsEnabled;
}

void CantorWorksheetScene::enableAnimations(bool enable)
{
    m_animationsEnabled = enable;
}

void CantorWorksheetScene::enableExpressionNumbering(bool enable)
{
    m_showExpressionIds=enable;
    emit updatePrompt();
}

QDomDocument CantorWorksheetScene::toXML(KZip* archive)
{
    QDomDocument doc( QLatin1String("CantorWorksheet") );
    QDomElement root=doc.createElement( QLatin1String("Worksheet") );
    root.setAttribute(QLatin1String("backend"), m_session->backend()->name());
    doc.appendChild(root);

    for( WorksheetEntry* entry = firstEntry(); entry; entry = entry->next())
    {
        QDomElement el = entry->toXml(doc, archive);
        root.appendChild( el );
    }
    return doc;
}

void CantorWorksheetScene::save( const QString& filename )
{
    qDebug()<<"saving to filename";
    KZip zipFile( filename );


    if ( !zipFile.open(QIODevice::WriteOnly) )
    {
        KMessageBox::error( worksheetView(),
                            i18n( "Cannot write file %1." , filename ),
                            i18n( "Error - Cantor" ));
        return;
    }

    QByteArray content = toXML(&zipFile).toByteArray();
    qDebug()<<"content: "<<content;
    zipFile.writeFile( QLatin1String("content.xml"), QString(), QString(), content.data(), content.size() );

    /*zipFile.close();*/
}


void CantorWorksheetScene::savePlain(const QString& filename)
{
    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly))
    {
        KMessageBox::error(worksheetView(), i18n("Error saving file %1", filename), i18n("Error - Cantor"));
        return;
    }

    QString cmdSep=QLatin1String(";\n");
    QString commentStartingSeq = QLatin1String("");
    QString commentEndingSeq = QLatin1String("");

    Cantor::Backend * const backend=session()->backend();
    if (backend->extensions().contains(QLatin1String("ScriptExtension")))
    {
        Cantor::ScriptExtension* e=dynamic_cast<Cantor::ScriptExtension*>(backend->extension(QLatin1String(("ScriptExtension"))));
        cmdSep=e->commandSeparator();
        commentStartingSeq = e->commentStartingSequence();
        commentEndingSeq = e->commentEndingSequence();
    }

    QTextStream stream(&file);

    for(WorksheetEntry * entry = firstEntry(); entry; entry = entry->next())
    {
        const QString& str=entry->toPlain(cmdSep, commentStartingSeq, commentEndingSeq);
        if(!str.isEmpty())
            stream << str + QLatin1Char('\n');
    }

    file.close();
}

void CantorWorksheetScene::saveLatex(const QString& filename)
{
    qDebug()<<"exporting to Latex: " <<filename;

    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly))
    {
        KMessageBox::error(worksheetView(), i18n("Error saving file %1", filename), i18n("Error - Cantor"));
        return;
    }

    QString xml = toXML().toString();
    QTextStream stream(&file);
    QXmlQuery query(QXmlQuery::XSLT20);
    query.setFocus(xml);

    QString stylesheet = QStandardPaths::locate(QStandardPaths::DataLocation, QLatin1String("xslt/latex.xsl"));
    if (stylesheet.isEmpty())
    {
        KMessageBox::error(worksheetView(), i18n("Error loading latex.xsl stylesheet"), i18n("Error - Cantor"));
        return;
    }

    query.setQuery(QUrl(stylesheet));
    QString out;
    if (query.evaluateTo(&out))
        stream << out;
    file.close();
}

void CantorWorksheetScene::load(const QString& filename )
{
    // m_file is always local so we can use QFile on it
    KZip file(filename);
    if (!file.open(QIODevice::ReadOnly))
        return ;

    const KArchiveEntry* contentEntry=file.directory()->entry(QLatin1String("content.xml"));
    if (!contentEntry->isFile())
    {
        qDebug()<<"error";
    }
    const KArchiveFile* content=static_cast<const KArchiveFile*>(contentEntry);
    QByteArray data=content->data();

    qDebug()<<"read: "<<data;

    QDomDocument doc;
    doc.setContent(data);
    QDomElement root=doc.documentElement();
    qDebug()<<root.tagName();

    const QString backendName=root.attribute(QLatin1String("backend"));
    Cantor::Backend* b=Cantor::Backend::createBackend(backendName);
    if (!b)
    {
        KMessageBox::error(worksheetView(), i18n("The backend with which this file was generated is not installed. It needs %1", backendName), i18n("Cantor"));
        return;
    }

    if(!b->isEnabled())
    {
        KMessageBox::information(worksheetView(), i18n("There are some problems with the %1 backend,\n"\
                                            "please check your configuration or install the needed packages.\n"
                                            "You will only be able to view this worksheet.", backendName), i18n("Cantor"));

    }


    //cleanup the worksheet and all it contains
    delete m_session;
    m_session=0;
    for(WorksheetEntry* entry = firstEntry(); entry; entry = entry->next())
        delete entry;
    clear();
    setFirstEntry(0);
    setLastEntry(0);

    m_session=b->createSession();
    m_loginFlag=true;

    qDebug()<<"loading entries";
    QDomElement expressionChild = root.firstChildElement();
    WorksheetEntry* entry;
    while (!expressionChild.isNull()) {
        QString tag = expressionChild.tagName();
        if (tag == QLatin1String("Expression"))
        {
            entry = appendCommandEntry();
            entry->setContent(expressionChild, file);
        } else if (tag == QLatin1String("Text"))
        {
            entry = appendTextEntry();
            entry->setContent(expressionChild, file);
        } else if (tag == QLatin1String("Latex"))
        {
            entry = appendLatexEntry();
            entry->setContent(expressionChild, file);
        } else if (tag == QLatin1String("PageBreak"))
        {
            entry = appendPageBreakEntry();
            entry->setContent(expressionChild, file);
        }
        else if (tag == QLatin1String("Image"))
        {
          entry = appendImageEntry();
          entry->setContent(expressionChild, file);
        }

        expressionChild = expressionChild.nextSiblingElement();
    }

    //login to the session, but let Qt process all the events in its pipeline
    //first.
    QTimer::singleShot(0, this, SLOT(loginToSession()));

    //Set the Highlighting, depending on the current state
    //If the session isn't logged in, use the default
    enableHighlighting( m_highlighter!=0 || (m_loginFlag) );



    emit sessionChanged();
}

void CantorWorksheetScene::gotResult(Cantor::Expression* expr)
{
    if(expr==0)
        expr=qobject_cast<Cantor::Expression*>(sender());

    if(expr==0)
        return;
    //We're only interested in help results, others are handled by the WorksheetEntry
    if(expr->result()&&expr->result()->type()==Cantor::HelpResult::Type)
    {
        QString help=expr->result()->toHtml();
        //Do some basic LaTeX replacing
        help.replace(QRegExp(QLatin1String("\\\\code\\{([^\\}]*)\\}")), QLatin1String("<b>\\1</b>"));
        help.replace(QRegExp(QLatin1String("\\$([^\\$])\\$")), QLatin1String("<i>\\1</i>"));

        emit showHelp(help);
    }
}

void CantorWorksheetScene::removeCurrentEntry()
{
    qDebug()<<"removing current entry";
    WorksheetEntry* entry=currentEntry();
    if(!entry)
        return;

    // In case we just removed this
    if (entry->isAncestorOf(m_lastFocusedTextItem))
        m_lastFocusedTextItem = 0;
    entry->startRemoving();
}

EpsRenderer* CantorWorksheetScene::epsRenderer()
{
    return &m_epsRenderer;
}

QMenu* CantorWorksheetScene::createContextMenu()
{
    QMenu *menu = new QMenu(worksheetView());
    connect(menu, SIGNAL(aboutToHide()), menu, SLOT(deleteLater()));

    return menu;
}

void CantorWorksheetScene::populateMenu(QMenu *menu, const QPointF& pos)
{
    WorksheetEntry* entry = entryAt(pos);
    if (entry && !entry->isAncestorOf(m_lastFocusedTextItem)) {
        WorksheetTextItem* item =
            qgraphicsitem_cast<WorksheetTextItem*>(itemAt(pos));
        if (item && item->isEditable())
            m_lastFocusedTextItem = item;
    }

    if (!isRunning())
        menu->addAction(QIcon::fromTheme(QLatin1String("system-run")), i18n("Evaluate Worksheet"),
                        this, SLOT(evaluate()), 0);
    else
        menu->addAction(QIcon::fromTheme(QLatin1String("process-stop")), i18n("Interrupt"), this,
                        SLOT(interrupt()), 0);
    menu->addSeparator();

    if (entry) {
        QMenu* insert = new QMenu(menu);
        QMenu* insertBefore = new QMenu(menu);

        insert->addAction(i18n("Command Entry"), entry, SLOT(insertCommandEntry()));
        insert->addAction(i18n("Text Entry"), entry, SLOT(insertTextEntry()));
        insert->addAction(i18n("LaTeX Entry"), entry, SLOT(insertLatexEntry()));
        insert->addAction(i18n("Image"), entry, SLOT(insertImageEntry()));
        insert->addAction(i18n("Page Break"), entry, SLOT(insertPageBreakEntry()));

        insertBefore->addAction(i18n("Command Entry"), entry, SLOT(insertCommandEntryBefore()));
        insertBefore->addAction(i18n("Text Entry"), entry, SLOT(insertTextEntryBefore()));
        insertBefore->addAction(i18n("LaTeX Entry"), entry, SLOT(insertLatexEntryBefore()));
        insertBefore->addAction(i18n("Image"), entry, SLOT(insertImageEntryBefore()));
        insertBefore->addAction(i18n("Page Break"), entry, SLOT(insertPageBreakEntryBefore()));

        insert->setTitle(i18n("Insert"));
        insertBefore->setTitle(i18n("Insert Before"));
        menu->addMenu(insert);
        menu->addMenu(insertBefore);
    } else {
        menu->addAction(i18n("Insert Command Entry"), this, SLOT(appendCommandEntry()));
        menu->addAction(i18n("Insert Text Entry"), this, SLOT(appendTextEntry()));
        menu->addAction(i18n("Insert LaTeX Entry"), this, SLOT(appendLatexEntry()));
        menu->addAction(i18n("Insert Image"), this, SLOT(appendImageEntry()));
        menu->addAction(i18n("Insert Page Break"), this, SLOT(appendPageBreakEntry()));
    }
}

void CantorWorksheetScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    // forward the event to the items
    QGraphicsScene::contextMenuEvent(event);

    if (!event->isAccepted()) {
        event->accept();
        QMenu *menu = createContextMenu();
        populateMenu(menu, event->scenePos());

        menu->popup(event->screenPos());
    }
}

void CantorWorksheetScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mousePressEvent(event);
    if (event->button() == Qt::LeftButton && !focusItem() && lastEntry() &&
        event->scenePos().y() > lastEntry()->y() + lastEntry()->size().height())
        lastEntry()->focusEntry(WorksheetTextItem::BottomRight);
}

void CantorWorksheetScene::createActions(KActionCollection* collection)
{
    // Mostly copied from KRichTextWidget::createActions(KActionCollection*)
    // It would be great if this wasn't necessary.

    // Text color
    QAction * action;
    /* This is "format-stroke-color" in KRichTextWidget */
    action = new QAction(QIcon::fromTheme(QLatin1String("format-text-color")),
                         i18nc("@action", "Text &Color..."), collection);
    action->setIconText(i18nc("@label text color", "Color"));
    action->setPriority(QAction::LowPriority);
    m_richTextActionList.append(action);
    collection->addAction(QLatin1String("format_text_foreground_color"), action);
    connect(action, SIGNAL(triggered()), this, SLOT(setTextForegroundColor()));

    // Text color
    action = new QAction(QIcon::fromTheme(QLatin1String("format-fill-color")),
                         i18nc("@action", "Text &Highlight..."), collection);
    action->setPriority(QAction::LowPriority);
    m_richTextActionList.append(action);
    collection->addAction(QLatin1String("format_text_background_color"), action);
    connect(action, SIGNAL(triggered()), this, SLOT(setTextBackgroundColor()));

    // Font Family
    m_fontAction = new KFontAction(i18nc("@action", "&Font"), collection);
    m_richTextActionList.append(m_fontAction);
    collection->addAction(QLatin1String("format_font_family"), m_fontAction);
    connect(m_fontAction, SIGNAL(triggered(QString)), this,
            SLOT(setFontFamily(QString)));

    // Font Size
    m_fontSizeAction = new KFontSizeAction(i18nc("@action", "Font &Size"),
                                           collection);
    m_richTextActionList.append(m_fontSizeAction);
    collection->addAction(QLatin1String("format_font_size"), m_fontSizeAction);
    connect(m_fontSizeAction, SIGNAL(fontSizeChanged(int)), this,
            SLOT(setFontSize(int)));

    // Bold
    m_boldAction = new KToggleAction(QIcon::fromTheme(QLatin1String("format-text-bold")),
                                     i18nc("@action boldify selected text", "&Bold"),
                                     collection);
    m_boldAction->setPriority(QAction::LowPriority);
    QFont bold;
    bold.setBold(true);
    m_boldAction->setFont(bold);
    m_richTextActionList.append(m_boldAction);
    collection->addAction(QLatin1String("format_text_bold"), m_boldAction);
    m_boldAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));
    connect(m_boldAction, SIGNAL(triggered(bool)), this,
            SLOT(setTextBold(bool)));

    // Italic
    m_italicAction = new KToggleAction(QIcon::fromTheme(QLatin1String("format-text-italic")),
                                       i18nc("@action italicize selected text",
                                             "&Italic"),
                                       collection);
    m_italicAction->setPriority(QAction::LowPriority);
    QFont italic;
    italic.setItalic(true);
    m_italicAction->setFont(italic);
    m_richTextActionList.append(m_italicAction);
    collection->addAction(QLatin1String("format_text_italic"), m_italicAction);
    m_italicAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));
    connect(m_italicAction, SIGNAL(triggered(bool)), this,
            SLOT(setTextItalic(bool)));

    // Underline
    m_underlineAction = new KToggleAction(QIcon::fromTheme(QLatin1String("format-text-underline")),
                                          i18nc("@action underline selected text",
                                                "&Underline"),
                                          collection);
    m_underlineAction->setPriority(QAction::LowPriority);
    QFont underline;
    underline.setUnderline(true);
    m_underlineAction->setFont(underline);
    m_richTextActionList.append(m_underlineAction);
    collection->addAction(QLatin1String("format_text_underline"), m_underlineAction);
    m_underlineAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_U));
    connect(m_underlineAction, SIGNAL(triggered(bool)), this,
            SLOT(setTextUnderline(bool)));

    // Strike
    m_strikeOutAction = new KToggleAction(QIcon::fromTheme(QLatin1String("format-text-strikethrough")),
                                          i18nc("@action", "&Strike Out"),
                                          collection);
    m_strikeOutAction->setPriority(QAction::LowPriority);
    m_richTextActionList.append(m_strikeOutAction);
    collection->addAction(QLatin1String("format_text_strikeout"), m_strikeOutAction);
    m_strikeOutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_L));
    connect(m_strikeOutAction, SIGNAL(triggered(bool)), this,
            SLOT(setTextStrikeOut(bool)));

    // Alignment
    QActionGroup *alignmentGroup = new QActionGroup(this);

    //   Align left
    m_alignLeftAction = new KToggleAction(QIcon::fromTheme(QLatin1String("format-justify-left")),
                                          i18nc("@action", "Align &Left"),
                                          collection);
    m_alignLeftAction->setPriority(QAction::LowPriority);
    m_alignLeftAction->setIconText(i18nc("@label left justify", "Left"));
    m_richTextActionList.append(m_alignLeftAction);
    collection->addAction(QLatin1String("format_align_left"), m_alignLeftAction);
    connect(m_alignLeftAction, SIGNAL(triggered()), this,
            SLOT(setAlignLeft()));
    alignmentGroup->addAction(m_alignLeftAction);

     //   Align center
    m_alignCenterAction = new KToggleAction(QIcon::fromTheme(QLatin1String("format-justify-center")),
                                            i18nc("@action", "Align &Center"),
                                            collection);
    m_alignCenterAction->setPriority(QAction::LowPriority);
    m_alignCenterAction->setIconText(i18nc("@label center justify", "Center"));
    m_richTextActionList.append(m_alignCenterAction);
    collection->addAction(QLatin1String("format_align_center"), m_alignCenterAction);
    connect(m_alignCenterAction, SIGNAL(triggered()), this,
            SLOT(setAlignCenter()));
    alignmentGroup->addAction(m_alignCenterAction);

    //   Align right
    m_alignRightAction = new KToggleAction(QIcon::fromTheme(QLatin1String("format-justify-right")),
                                           i18nc("@action", "Align &Right"),
                                           collection);
    m_alignRightAction->setPriority(QAction::LowPriority);
    m_alignRightAction->setIconText(i18nc("@label right justify", "Right"));
    m_richTextActionList.append(m_alignRightAction);
    collection->addAction(QLatin1String("format_align_right"), m_alignRightAction);
    connect(m_alignRightAction, SIGNAL(triggered()), this,
            SLOT(setAlignRight()));
    alignmentGroup->addAction(m_alignRightAction);

    //   Align justify
    m_alignJustifyAction = new KToggleAction(QIcon::fromTheme(QLatin1String("format-justify-fill")),
                                             i18nc("@action", "&Justify"),
                                             collection);
    m_alignJustifyAction->setPriority(QAction::LowPriority);
    m_alignJustifyAction->setIconText(i18nc("@label justify fill", "Justify"));
    m_richTextActionList.append(m_alignJustifyAction);
    collection->addAction(QLatin1String("format_align_justify"), m_alignJustifyAction);
    connect(m_alignJustifyAction, SIGNAL(triggered()), this,
            SLOT(setAlignJustify()));
    alignmentGroup->addAction(m_alignJustifyAction);

     /*
     // List style
     KSelectAction* selAction;
     selAction = new KSelectAction(QIcon::fromTheme("format-list-unordered"),
                                   i18nc("@title:menu", "List Style"),
                                   collection);
     QStringList listStyles;
     listStyles      << i18nc("@item:inmenu no list style", "None")
                     << i18nc("@item:inmenu disc list style", "Disc")
                     << i18nc("@item:inmenu circle list style", "Circle")
                     << i18nc("@item:inmenu square list style", "Square")
                     << i18nc("@item:inmenu numbered lists", "123")
                     << i18nc("@item:inmenu lowercase abc lists", "abc")
                     << i18nc("@item:inmenu uppercase abc lists", "ABC");
     selAction->setItems(listStyles);
     selAction->setCurrentItem(0);
     action = selAction;
     m_richTextActionList.append(action);
     collection->addAction("format_list_style", action);
     connect(action, SIGNAL(triggered(int)),
             this, SLOT(_k_setListStyle(int)));
     connect(action, SIGNAL(triggered()),
             this, SLOT(_k_updateMiscActions()));

     // Indent
     action = new QAction(QIcon::fromTheme("format-indent-more"),
                          i18nc("@action", "Increase Indent"), collection);
     action->setPriority(QAction::LowPriority);
     m_richTextActionList.append(action);
     collection->addAction("format_list_indent_more", action);
     connect(action, SIGNAL(triggered()),
             this, SLOT(indentListMore()));
     connect(action, SIGNAL(triggered()),
             this, SLOT(_k_updateMiscActions()));

     // Dedent
     action = new QAction(QIcon::fromTheme("format-indent-less"),
                          i18nc("@action", "Decrease Indent"), collection);
     action->setPriority(QAction::LowPriority);
     m_richTextActionList.append(action);
     collection->addAction("format_list_indent_less", action);
     connect(action, SIGNAL(triggered()), this, SLOT(indentListLess()));
     connect(action, SIGNAL(triggered()), this, SLOT(_k_updateMiscActions()));
     */
}

WorksheetTextItem* CantorWorksheetScene::lastFocusedTextItem()
{
    return m_lastFocusedTextItem;
}

void CantorWorksheetScene::updateFocusedTextItem(WorksheetTextItem* newItem)
{
    if (m_lastFocusedTextItem && m_lastFocusedTextItem != newItem) {
        disconnect(m_lastFocusedTextItem, SIGNAL(undoAvailable(bool)),
                   this, SIGNAL(undoAvailable(bool)));
        disconnect(m_lastFocusedTextItem, SIGNAL(redoAvailable(bool)),
                   this, SIGNAL(redoAvailable(bool)));
        disconnect(this, SIGNAL(undo()), m_lastFocusedTextItem, SLOT(undo()));
        disconnect(this, SIGNAL(redo()), m_lastFocusedTextItem, SLOT(redo()));
        disconnect(m_lastFocusedTextItem, SIGNAL(cutAvailable(bool)),
                   this, SIGNAL(cutAvailable(bool)));
        disconnect(m_lastFocusedTextItem, SIGNAL(copyAvailable(bool)),
                   this, SIGNAL(copyAvailable(bool)));
        disconnect(m_lastFocusedTextItem, SIGNAL(pasteAvailable(bool)),
                   this, SIGNAL(pasteAvailable(bool)));
        disconnect(this, SIGNAL(cut()), m_lastFocusedTextItem, SLOT(cut()));
        disconnect(this, SIGNAL(copy()), m_lastFocusedTextItem, SLOT(copy()));
        disconnect(this, SIGNAL(paste()), m_lastFocusedTextItem, SLOT(paste()));

        m_lastFocusedTextItem->clearSelection();
    }

    if (newItem && m_lastFocusedTextItem != newItem) {
        setAcceptRichText(newItem->richTextEnabled());
        emit undoAvailable(newItem->isUndoAvailable());
        emit redoAvailable(newItem->isRedoAvailable());
        connect(newItem, SIGNAL(undoAvailable(bool)),
                this, SIGNAL(undoAvailable(bool)));
        connect(newItem, SIGNAL(redoAvailable(bool)),
                this, SIGNAL(redoAvailable(bool)));
        connect(this, SIGNAL(undo()), newItem, SLOT(undo()));
        connect(this, SIGNAL(redo()), newItem, SLOT(redo()));
        emit cutAvailable(newItem->isCutAvailable());
        emit copyAvailable(newItem->isCopyAvailable());
        emit pasteAvailable(newItem->isPasteAvailable());
        connect(newItem, SIGNAL(cutAvailable(bool)),
                this, SIGNAL(cutAvailable(bool)));
        connect(newItem, SIGNAL(copyAvailable(bool)),
                this, SIGNAL(copyAvailable(bool)));
        connect(newItem, SIGNAL(pasteAvailable(bool)),
                this, SIGNAL(pasteAvailable(bool)));
        connect(this, SIGNAL(cut()), newItem, SLOT(cut()));
        connect(this, SIGNAL(copy()), newItem, SLOT(copy()));
        connect(this, SIGNAL(paste()), newItem, SLOT(paste()));
    } else if (!newItem) {
        emit undoAvailable(false);
        emit redoAvailable(false);
        emit cutAvailable(false);
        emit copyAvailable(false);
        emit pasteAvailable(false);
    }
    m_lastFocusedTextItem = newItem;
}

void CantorWorksheetScene::setRichTextInformation(const RichTextInfo& info)
{
    m_boldAction->setChecked(info.bold);
    m_italicAction->setChecked(info.italic);
    m_underlineAction->setChecked(info.underline);
    m_strikeOutAction->setChecked(info.strikeOut);
    m_fontAction->setFont(info.font);
    if (info.fontSize > 0)
        m_fontSizeAction->setFontSize(info.fontSize);

    if (info.align & Qt::AlignLeft)
        m_alignLeftAction->setChecked(true);
    else if (info.align & Qt::AlignCenter)
        m_alignCenterAction->setChecked(true);
    else if (info.align & Qt::AlignRight)
        m_alignRightAction->setChecked(true);
    else if (info.align & Qt::AlignJustify)
        m_alignJustifyAction->setChecked(true);
}

void CantorWorksheetScene::setAcceptRichText(bool b)
{
    foreach(QAction * action, m_richTextActionList) {
        action->setEnabled(b);
    }

    /*
    foreach(QWidget* widget, m_fontAction->createdWidgets()) {
        widget->setEnabled(b);
    }

    foreach(QWidget* widget, m_fontSizeAction->createdWidgets()) {
        widget->setEnabled(b);
    }
    */
}

WorksheetTextItem* CantorWorksheetScene::currentTextItem()
{
    QGraphicsItem* item = focusItem();
    if (!item)
        item = m_lastFocusedTextItem;
    while (item && item->type() != WorksheetTextItem::Type)
        item = item->parentItem();

    return qgraphicsitem_cast<WorksheetTextItem*>(item);
}

void CantorWorksheetScene::setTextForegroundColor()
{
    WorksheetTextItem* item = currentTextItem();
    if (item)
        item->setTextForegroundColor();
}

void CantorWorksheetScene::setTextBackgroundColor()
{
    WorksheetTextItem* item = currentTextItem();
    if (item)
        item->setTextBackgroundColor();
}

void CantorWorksheetScene::setTextBold(bool b)
{
    WorksheetTextItem* item = currentTextItem();
    if (item)
        item->setTextBold(b);
}

void CantorWorksheetScene::setTextItalic(bool b)
{
    WorksheetTextItem* item = currentTextItem();
    if (item)
        item->setTextItalic(b);
}

void CantorWorksheetScene::setTextUnderline(bool b)
{
    WorksheetTextItem* item = currentTextItem();
    if (item)
        item->setTextUnderline(b);
}

void CantorWorksheetScene::setTextStrikeOut(bool b)
{
    WorksheetTextItem* item = currentTextItem();
    if (item)
        item->setTextStrikeOut(b);
}

void CantorWorksheetScene::setAlignLeft()
{
    WorksheetTextItem* item = currentTextItem();
    if (item)
        item->setAlignment(Qt::AlignLeft);
}

void CantorWorksheetScene::setAlignRight()
{
    WorksheetTextItem* item = currentTextItem();
    if (item)
        item->setAlignment(Qt::AlignRight);
}

void CantorWorksheetScene::setAlignCenter()
{
    WorksheetTextItem* item = currentTextItem();
    if (item)
        item->setAlignment(Qt::AlignCenter);
}

void CantorWorksheetScene::setAlignJustify()
{
    WorksheetTextItem* item = currentTextItem();
    if (item)
        item->setAlignment(Qt::AlignJustify);
}

void CantorWorksheetScene::setFontFamily(QString font)
{
    WorksheetTextItem* item = currentTextItem();
    if (item)
        item->setFontFamily(font);
}

void CantorWorksheetScene::setFontSize(int size)
{
    WorksheetTextItem* item = currentTextItem();
    if (item)
        item->setFontSize(size);
}

bool CantorWorksheetScene::isShortcut(QKeySequence sequence)
{
    return m_shortcuts.contains(sequence);
}

void CantorWorksheetScene::registerShortcut(QAction* action)
{
    qDebug() << action->shortcuts();
    foreach(QKeySequence shortcut, action->shortcuts()) {
        m_shortcuts.insert(shortcut, action);
    }
    connect(action, SIGNAL(changed()), this, SLOT(updateShortcut()));
}

void CantorWorksheetScene::updateShortcut()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action)
        return;

    // delete the old shortcuts of this action
    QList<QKeySequence> shortcuts = m_shortcuts.keys(action);
    foreach(QKeySequence shortcut, shortcuts) {
        m_shortcuts.remove(shortcut);
    }
    // add the new shortcuts
    foreach(QKeySequence shortcut, action->shortcuts()) {
        m_shortcuts.insert(shortcut, action);
    }
}

void CantorWorksheetScene::dragEnterEvent(QGraphicsSceneDragDropEvent* event)
{
    qDebug() << "enter";
    if (m_dragEntry)
        event->accept();
    else
        QGraphicsScene::dragEnterEvent(event);
}

void CantorWorksheetScene::dragLeaveEvent(QGraphicsSceneDragDropEvent* event)
{
    if (!m_dragEntry) {
        QGraphicsScene::dragLeaveEvent(event);
        return;
    }

    qDebug() << "leave";
    event->accept();
    if (m_placeholderEntry) {
        m_placeholderEntry->startRemoving();
        m_placeholderEntry = 0;
    }
}

void CantorWorksheetScene::dragMoveEvent(QGraphicsSceneDragDropEvent* event)
{
    if (!m_dragEntry) {
        QGraphicsScene::dragMoveEvent(event);
        return;
    }

    QPointF pos = event->scenePos();
    WorksheetEntry* entry = entryAt(pos);
    WorksheetEntry* prev = 0;
    WorksheetEntry* next = 0;
    if (entry) {
        if (pos.y() < entry->y() + entry->size().height()/2) {
            prev = entry->previous();
            next = entry;
        } else if (pos.y() >= entry->y() + entry->size().height()/2) {
            prev = entry;
            next = entry->next();
        }
    } else {
        WorksheetEntry* last = lastEntry();
        if (last && pos.y() > last->y() + last->size().height()) {
            prev = last;
            next = 0;
        }
    }

    if (prev || next) {
        PlaceHolderEntry* oldPlaceHolder = m_placeholderEntry;
        if (prev && prev->type() == PlaceHolderEntry::Type &&
            (!prev->aboutToBeRemoved() || prev->stopRemoving())) {
            m_placeholderEntry = qgraphicsitem_cast<PlaceHolderEntry*>(prev);
            m_placeholderEntry->changeSize(m_dragEntry->size());
        } else if (next && next->type() == PlaceHolderEntry::Type &&
                   (!next->aboutToBeRemoved() || next->stopRemoving())) {
            m_placeholderEntry = qgraphicsitem_cast<PlaceHolderEntry*>(next);
            m_placeholderEntry->changeSize(m_dragEntry->size());
        } else {
            m_placeholderEntry = new PlaceHolderEntry(this, QSizeF(0,0));
            m_placeholderEntry->setPrevious(prev);
            m_placeholderEntry->setNext(next);
            if (prev)
                prev->setNext(m_placeholderEntry);
            else
                setFirstEntry(m_placeholderEntry);
            if (next)
                next->setPrevious(m_placeholderEntry);
            else
                setLastEntry(m_placeholderEntry);
            m_placeholderEntry->changeSize(m_dragEntry->size());
        }
        if (oldPlaceHolder && oldPlaceHolder != m_placeholderEntry)
            oldPlaceHolder->startRemoving();
        updateLayout();
    }

    const QPoint viewPos = worksheetView()->mapFromScene(pos);
    const int viewHeight = worksheetView()->viewport()->height();
    if ((viewPos.y() < 10 || viewPos.y() > viewHeight - 10) &&
        !m_dragScrollTimer) {
        m_dragScrollTimer = new QTimer(this);
        m_dragScrollTimer->setSingleShot(true);
        m_dragScrollTimer->setInterval(100);
        connect(m_dragScrollTimer, SIGNAL(timeout()), this,
                SLOT(updateDragScrollTimer()));
        m_dragScrollTimer->start();
    }

    event->accept();
}

void CantorWorksheetScene::dropEvent(QGraphicsSceneDragDropEvent* event)
{
    if (!m_dragEntry)
        QGraphicsScene::dropEvent(event);
    event->accept();
}

void CantorWorksheetScene::updateDragScrollTimer()
{
    if (!m_dragScrollTimer)
        return;

    const QPoint viewPos = worksheetView()->viewCursorPos();
    const QWidget* viewport = worksheetView()->viewport();
    const int viewHeight = viewport->height();
    if (!m_dragEntry || !(viewport->rect().contains(viewPos)) ||
        (viewPos.y() >= 10 && viewPos.y() <= viewHeight - 10)) {
        delete m_dragScrollTimer;
        m_dragScrollTimer = 0;
        return;
    }

    if (viewPos.y() < 10)
        worksheetView()->scrollBy(-10*(10 - viewPos.y()));
    else
        worksheetView()->scrollBy(10*(viewHeight - viewPos.y()));

    m_dragScrollTimer->start();
}


