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

#include "latexentry.h"

#include "worksheetentry.h"
#include "worksheet.h"
#include "epsrenderer.h"
#include "cantor/defaulthighlighter.h"
#include "latexrenderer.h"

#include <QTextCursor>

#include <QDebug>
#include <KGlobal>
#include <KLocale>

LatexEntry::LatexEntry(Worksheet* worksheet) : WorksheetEntry(worksheet), m_textItem(new WorksheetTextItem(this, Qt::TextEditorInteraction))
{
    connect(m_textItem, &WorksheetTextItem::moveToPrevious, this, &LatexEntry::moveToPreviousEntry);
    connect(m_textItem, &WorksheetTextItem::moveToNext, this, &LatexEntry::moveToNextEntry);
    connect(m_textItem, SIGNAL(execute()), this, SLOT(evaluate()));
    connect(m_textItem, &WorksheetTextItem::doubleClick, this, &LatexEntry::resolveImagesAtCursor);
}

LatexEntry::~LatexEntry()
{
}

void LatexEntry::populateMenu(QMenu *menu, const QPointF& pos)
{
    bool imageSelected = false;
    QTextCursor cursor = m_textItem->textCursor();
    const QChar repl = QChar::ObjectReplacementCharacter;
    if (cursor.hasSelection()) {
        QString selection = m_textItem->textCursor().selectedText();
        imageSelected = selection.contains(repl);
    } else {
        // we need to try both the current cursor and the one after the that
        cursor = m_textItem->cursorForPosition(pos);
        for (int i = 2; i; --i) {
            int p = cursor.position();
            if (m_textItem->document()->characterAt(p-1) == repl &&
                cursor.charFormat().hasProperty(EpsRenderer::CantorFormula)) {
                m_textItem->setTextCursor(cursor);
                imageSelected = true;
                break;
            }
            cursor.movePosition(QTextCursor::NextCharacter);
        }
    }
    if (imageSelected) {
        menu->addAction(i18n("Show LaTeX code"), this, SLOT(resolveImagesAtCursor()));
        menu->addSeparator();
    }
    WorksheetEntry::populateMenu(menu, pos);
}

int LatexEntry::type() const
{
    return Type;
}

bool LatexEntry::isEmpty()
{
    return m_textItem->document()->isEmpty();
}

bool LatexEntry::acceptRichText()
{
    return false;
}

bool LatexEntry::focusEntry(int pos, qreal xCoord)
{
    if (aboutToBeRemoved())
        return false;
    m_textItem->setFocusAt(pos, xCoord);
    return true;
}

void LatexEntry::setContent(const QString& content)
{
    m_textItem->setPlainText(content);
}

void LatexEntry::setContent(const QDomElement& content, const KZip& file)
{
    QString latexCode = content.text();
    qDebug() << latexCode;

    m_textItem->document()->clear();
    QTextCursor cursor = m_textItem->textCursor();
    cursor.movePosition(QTextCursor::Start);

    if(content.hasAttribute(QLatin1String("filename")))
    {
        const KArchiveEntry* imageEntry=file.directory()->entry(content.attribute(QLatin1String("filename")));
        if (imageEntry&&imageEntry->isFile())
        {
            const KArchiveFile* imageFile=static_cast<const KArchiveFile*>(imageEntry);
            QString dir=QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QLatin1String("/") + QLatin1String("cantor");
            imageFile->copyTo(dir);
            QString imagePath=QString(dir+QLatin1String("/")+imageFile->name());

            QUrl internal=QUrl::fromLocalFile(imagePath);
            internal.setScheme(QLatin1String("internal"));

            QTextImageFormat format = worksheet()->epsRenderer()->render(m_textItem->document(), QUrl::fromLocalFile(imagePath));
            qDebug()<<"rendering successful? " << !format.name().isEmpty();

            format.setProperty(EpsRenderer::CantorFormula,
                               EpsRenderer::LatexFormula);
            format.setProperty(EpsRenderer::ImagePath, imagePath);
            format.setProperty(EpsRenderer::Code, latexCode);
            cursor.insertText(QString(QChar::ObjectReplacementCharacter), format);
        } else
        {
            cursor.insertText(latexCode);
        }
    } else
    {
        cursor.insertText(latexCode);
    }
}

QDomElement LatexEntry::toXml(QDomDocument& doc, KZip* archive)
{
    Q_UNUSED(archive);

    QDomElement el = doc.createElement(QLatin1String("Latex"));
    el.appendChild( doc.createTextNode( latexCode() ));
    return el;
}

QString LatexEntry::toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq)
{
    Q_UNUSED(commandSep);

    if (commentStartingSeq.isEmpty())
        return QString();

    QString text = latexCode();
    if (!commentEndingSeq.isEmpty())
        return commentStartingSeq + text + commentEndingSeq + QLatin1String("\n");
    return commentStartingSeq + text.replace(QLatin1String("\n"), QLatin1String("\n") + commentStartingSeq) + QLatin1String("\n");
}

void LatexEntry::interruptEvaluation()
{

}

bool LatexEntry::evaluate(EvaluationOption evalOp)
{
    bool success = false;

    if (isOneImageOnly())
    {
        success = true;
    }
    else
    {
        QString latex = latexCode();
        Cantor::LatexRenderer* renderer = new Cantor::LatexRenderer(this);
        renderer->setLatexCode(latex);
        renderer->setEquationOnly(false);
        renderer->setMethod(Cantor::LatexRenderer::LatexMethod);
        renderer->renderBlocking();

        QTextImageFormat formulaFormat;
        if (renderer->renderingSuccessful())
        {
            EpsRenderer* epsRend = worksheet()->epsRenderer();
            formulaFormat = epsRend->render(m_textItem->document(), renderer);
            success = !formulaFormat.name().isEmpty();
        }

        if(success)
        {
            QTextCursor cursor = m_textItem->textCursor();
            cursor.movePosition(QTextCursor::Start);
            cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
            cursor.insertText(QString(QChar::ObjectReplacementCharacter), formulaFormat);
        }
        delete renderer;
    }

    qDebug()<<"rendering successful? "<<success;

    evaluateNext(evalOp);
    return success;
}

void LatexEntry::updateEntry()
{
    QTextCursor cursor = m_textItem->document()->find(QString(QChar::ObjectReplacementCharacter));
    while (!cursor.isNull())
    {
        qDebug()<<"found a formula... rendering the eps...";
        QTextCharFormat format=cursor.charFormat();
        QUrl url=qVariantValue<QUrl>(format.property(EpsRenderer::ImagePath));
        QSizeF s = worksheet()->epsRenderer()->renderToResource(m_textItem->document(), url);
        qDebug()<<"rendering successful? "<< !s.isValid();

        //HACK: reinsert this image, to make sure the layout is updated to the new size
        //cursor.removeSelectedText();
        //cursor.insertText(QString(QChar::ObjectReplacementCharacter), format);
        cursor.movePosition(QTextCursor::NextCharacter);

        cursor = m_textItem->document()->find(QString(QChar::ObjectReplacementCharacter), cursor);
    }
}

void LatexEntry::resolveImagesAtCursor()
{
    QTextCursor cursor = m_textItem->textCursor();
    if (!cursor.hasSelection())
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);

    cursor.insertText(m_textItem->resolveImages(cursor));
}

QString LatexEntry::latexCode()
{
    QTextCursor cursor = m_textItem->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

    QString code = m_textItem->resolveImages(cursor);
    code.replace(QChar::ParagraphSeparator, QLatin1Char('\n')); //Replace the U+2029 paragraph break by a Normal Newline
    code.replace(QChar::LineSeparator, QLatin1Char('\n')); //Replace the line break by a Normal Newline
    return code;
}

bool LatexEntry::isOneImageOnly()
{
    QTextCursor cursor = m_textItem->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

    return (cursor.selectionEnd() == 1 && cursor.selectedText() == QString(QChar::ObjectReplacementCharacter));
}

int LatexEntry::searchText(QString text, QString pattern,
                          QTextDocument::FindFlags qt_flags)
{
    Qt::CaseSensitivity caseSensitivity;
    if (qt_flags & QTextDocument::FindCaseSensitively)
        caseSensitivity = Qt::CaseSensitive;
    else
        caseSensitivity = Qt::CaseInsensitive;

    int position;
    if (qt_flags & QTextDocument::FindBackward)
        position = text.lastIndexOf(pattern, -1, caseSensitivity);
    else
        position = text.indexOf(pattern, 0, caseSensitivity);

    return position;
}

WorksheetCursor LatexEntry::search(QString pattern, unsigned flags,
                                   QTextDocument::FindFlags qt_flags,
                                   const WorksheetCursor& pos)
{
    if (!(flags & WorksheetEntry::SearchLaTeX))
        return WorksheetCursor();
    if (pos.isValid() && (pos.entry() != this || pos.textItem() != m_textItem))
        return WorksheetCursor();

    QTextCursor textCursor = m_textItem->search(pattern, qt_flags, pos);
    int position;
    QString latex;
    const QString repl = QString(QChar::ObjectReplacementCharacter);
    QTextCursor latexCursor = m_textItem->search(repl, qt_flags, pos);

    while (!latexCursor.isNull()) {
        latex = m_textItem->resolveImages(latexCursor);
        position = searchText(latex, pattern, qt_flags);
        if (position >= 0) {
            break;
        }
        WorksheetCursor c(this, m_textItem, latexCursor);
        latexCursor = m_textItem->search(repl, qt_flags, c);
    }

    if (latexCursor.isNull()) {
        if (textCursor.isNull())
            return WorksheetCursor();
        else
            return WorksheetCursor(this, m_textItem, textCursor);
    } else {
        if (textCursor.isNull() || latexCursor < textCursor) {
            int start = latexCursor.selectionStart();
            latexCursor.insertText(latex);
            QTextCursor c = m_textItem->textCursor();
            c.setPosition(start + position);
            c.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor,
                           pattern.length());
            return WorksheetCursor(this, m_textItem, c);
        } else {
            return WorksheetCursor(this, m_textItem, textCursor);
        }
    }
}

void LatexEntry::layOutForWidth(qreal w, bool force)
{
    if (size().width() == w && !force)
        return;

    m_textItem->setGeometry(0, 0, w);
    setSize(QSizeF(m_textItem->width(), m_textItem->height() + VerticalMargin));
}

bool LatexEntry::wantToEvaluate()
{
    return !isOneImageOnly();
}
