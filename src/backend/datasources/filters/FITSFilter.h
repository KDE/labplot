/***************************************************************************
File                 : FITSFilter.h
Project              : LabPlot
Description          : FITS I/O-filter
--------------------------------------------------------------------
Copyright            : (C) 2016 by Fabian Kristof (fkristofszabolcs@gmail.com)
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

#ifndef FITSFILTER_H
#define FITSFILTER_H

#include "backend/datasources/filters/AbstractFileFilter.h"
#include <QStringList>
#include <KLocale>
#include <QTableWidget>
#include <QTreeWidgetItem>
#include <QXmlStreamReader>

class FITSFilterPrivate;
class FITSFilter : public AbstractFileFilter{
    Q_OBJECT

  public:

    FITSFilter();
    ~FITSFilter();

    enum KeywordUpdateMode {
        UpdateComment = 0,
        UpdateValueComment,
        UpdateWithBlankValue,
        UpdateWithoutComment,
        UpdateKeyname
    };

    void read(const QString & fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode=AbstractFileFilter::Replace);
    void write(const QString & fileName, AbstractDataSource* dataSource);
    QString readChdu(const QString & fileName, int lines = -1);
    virtual void save(QXmlStreamWriter*) const;
    virtual bool load(XmlStreamReader*);
    struct Keyword {
        QString key;
        QString value;
        QString comment;
        bool operator==(const Keyword& other) const{
            return other.key == key &&
                    other.value == value &&
                     other.comment == comment;
        }
    };

    void updateKeyword(Keyword& keyword,const QString& newKey, const QString& newValue,
                       const QString& newComment, KeywordUpdateMode mode = UpdateValueComment);
    void addNewKeyword(const QString& filename, const QList<Keyword>& keywords);
    void deleteKeyword(const QString& fileName, const QList<Keyword>& keywords);
    void renameKeywordKey(const Keyword& keyword, const QString& newKey);
    void removeExtensions(const QStringList& extensions);
    void parseHeader(const QString &fileName, QTableWidget* headerEditTable,
                     bool readKeys = true,
                     const QList<Keyword> &keys = QList<Keyword>());
    void parseExtensions(const QString& fileName, QTreeWidget *tw, bool checkPrimary = false);
    QList<Keyword> chduKeywords(const QString &fileName);

    static QStringList standardKeywords();
    static QStringList mandatoryImageExtensionKeywords();
    static QStringList mandatoryTableExtensionKeywords();

    void loadFilterSettings(const QString&);
    void saveFilterSettings(const QString&) const;

    void setStartRow(const int);
    int startRow() const;
    void setEndRow(const int);
    int endRow() const;
    void setStartColumn(const int);
    int startColumn() const;
    void setEndColumn(const int);
    int endColumn() const;

private:
    FITSFilterPrivate* const d;
    friend class FITSFilterPrivate;
};
#endif // FITSFILTER_H
