/***************************************************************************
File                 : FITSFilter.cpp
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


#include "FITSFilter.h"
#include "FITSFilterPrivate.h"
#include <QDebug>

FITSFilter::FITSFilter():AbstractFileFilter(), d(new FITSFilterPrivate(this)) {
}

FITSFilter::~FITSFilter() {
    delete d;
}

void FITSFilter::read(const QString &fileName, AbstractDataSource *dataSource, AbstractFileFilter::ImportMode importMode) {
    d->readCHDU(fileName, dataSource, importMode);
}

void FITSFilter::write(const QString &fileName, AbstractDataSource *dataSource) {
    d->writeCHDU(fileName, dataSource);
}

QStringList FITSFilter::extensionNames(const QString &fileName) {
    return d->extensionNames(fileName);
}

void FITSFilter::addNewKeyword(const FITSFilter::Keyword& keyword) {
    d->addNewKeyword(keyword);
}

void FITSFilter::updateKeywordValue(Keyword &keyword) {
    d->updateKeywordValue(keyword);
}

void FITSFilter::deleteKeyword(const Keyword &keyword) {
    d->deleteKeyword(keyword);
}

void FITSFilter::parseHeader(const QString &fileName, QTableWidget *headerEditTable){
    d->parseHeader(fileName, headerEditTable);
}

//#####################################################################
//################### Private implementation ##########################
//#####################################################################

FITSFilterPrivate::FITSFilterPrivate(FITSFilter* owner) :
    q(owner) {
}

/*!
    Read the current header data unit from file \c filename in data source \c dataSource in
    \c importMode import mode
*/
void FITSFilterPrivate::readCHDU(const QString &fileName, AbstractDataSource *dataSource, AbstractFileFilter::ImportMode importMode) {
    Q_UNUSED(fileName)
    Q_UNUSED(dataSource)
    Q_UNUSED(importMode)
}

/*!
    Export
*/

void FITSFilterPrivate::writeCHDU(const QString &fileName, AbstractDataSource *dataSource) {
    Q_UNUSED(fileName)
    Q_UNUSED(dataSource)
}

/*!
    Return a list of the available extensions names in file \c filename
*/

QStringList FITSFilterPrivate::extensionNames(const QString& fileName) {
    QStringList extensionNames;
    int status = 0;
    if (!fits_open_file(&fitsFile, fileName.toLatin1(), READONLY, &status )) {
        int hduCount;

        if (!fits_get_num_hdus(fitsFile, &hduCount, &status)) {
            int imageCount = 0;
            int asciiTableCount = 0;
            int binaryTableCount = 0;
            for (int currentHDU = 1; currentHDU <= hduCount; ++currentHDU) {
                char extensionName[80];
                int hduType;

                fits_get_hdu_type(fitsFile, &hduType, &status);

                switch (hduType) {
                case IMAGE_HDU:
                    imageCount++;
                    break;
                case ASCII_TBL:
                    asciiTableCount++;
                    break;
                case BINARY_TBL:
                    binaryTableCount++;
                    break;
                }
                QString extName;
                if (!fits_read_keyword(fitsFile, "EXTNAME", extensionName, NULL, &status)){
                    extName = QString(extensionName);
                } else {
                    status = 0;
                    if (!fits_read_keyword(fitsFile, "HDUNAME", extensionName, NULL, &status)) {
                        extName = QString(extensionName);
                    } else {
                        switch (hduType) {
                        case IMAGE_HDU:
                            extName = i18n("IMAGE #%1").arg(imageCount);
                            break;
                        case ASCII_TBL:
                            extName = i18n("ASCII_TBL #%1").arg(asciiTableCount);
                            break;
                        case BINARY_TBL:
                            extName = i18n("BINARY_TBL #%1").arg(binaryTableCount);
                            break;
                        }
                    }
                }
                extensionNames << extName.trimmed();
                fits_movrel_hdu(fitsFile, 1, NULL, &status);
            }
        } else {
            printError(status);
        }
    } else {
        printError(status);
    }

    if (status == END_OF_FILE) {
        status = 0;
    }

    if (fits_close_file(fitsFile, &status)) {
        printError(status);
    }

    return extensionNames;
}

/*!
  Prints the error text corresponding to the status code \c status
*/
void FITSFilterPrivate::printError(int status) const {
    if (status) {
        char errorText[80];
        fits_get_errstatus(status, errorText );
        qDebug() << QLatin1String(errorText);
    }
}

/*!
    Add the keyword \c keyword to the current header unit
*/
void FITSFilterPrivate::addNewKeyword(const FITSFilter::Keyword& keyword) {
    Q_UNUSED(keyword)
}

/*!
    Update the value of keyword \c keyword in the current header unit
*/
void FITSFilterPrivate::updateKeywordValue(FITSFilter::Keyword& keyword) {
    Q_UNUSED(keyword)
}

/*!
    Delete the keyword \c keyword from the current header unit
*/
void FITSFilterPrivate::deleteKeyword(const FITSFilter::Keyword &keyword) {
    Q_UNUSED(keyword)
}

/*!
    Returns a list of keywords
*/
QList<FITSFilter::Keyword> FITSFilterPrivate::chduKeywords(const QString& fileName) {
    int status = 0;

    if (fitsFile) {
        fits_close_file(fitsFile, &status);
    }

    if (status != 0) {
        printError(status);
    }

    status = 0;

    if (fits_open_file(&fitsFile, fileName.toLatin1(), READONLY, &status )) {
        printError(status);
        return QList<FITSFilter::Keyword> ();
    }
    char* headerKeywords;
    int numberOfKeys;
    if (fits_hdr2str(fitsFile, 0, NULL, 0, &headerKeywords, &numberOfKeys, &status)) {
        printError(status);
        free(headerKeywords);
        return QList<FITSFilter::Keyword> ();
    }

    QString keywordString = QString(headerKeywords);
    free(headerKeywords);

    QList<FITSFilter::Keyword> keywords;
    keywords.reserve(numberOfKeys);
    FITSFilter::Keyword keyword;
    for (int i = 0; i < numberOfKeys; ++i) {
        QString card = keywordString.mid(i* 80, 80);
        QStringList recordValues = card.split(QRegExp("[=/]"));

        if (recordValues.size() == 3) {
            keyword.key = recordValues[0].simplified();
            keyword.value = recordValues[1].simplified();
            keyword.comment = recordValues[2].simplified();
        } else if (recordValues.size() == 2) {
            keyword.key = recordValues[0].simplified();
            keyword.value = recordValues[1].simplified();
        } else {
            keyword.key = recordValues[0].simplified();
        }
        keywords.append(keyword);
    }

    return keywords;
}

void FITSFilterPrivate::parseHeader(const QString &fileName, QTableWidget *headerEditTable) {
    QList<FITSFilter::Keyword> keywords = chduKeywords(fileName);

    headerEditTable->setRowCount(keywords.size());
    headerEditTable->setColumnCount(3);

    for (int i = 0; i < keywords.size(); ++i) {
        QTableWidgetItem* item = new QTableWidgetItem(keywords.at(i).key);
        item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        headerEditTable->setItem(i, 0, item );

        item = new QTableWidgetItem(keywords.at(i).value);
        item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        headerEditTable->setItem(i, 1, item );

        item = new QTableWidgetItem(keywords.at(i).comment);
        item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        headerEditTable->setItem(i, 2, item );
    }

    headerEditTable->resizeColumnsToContents();
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

/*!
  Saves as XML.
*/

void FITSFilter::save(QXmlStreamWriter * writer) const {
    Q_UNUSED(writer)
}

/*!
  Loads from XML.
*/

bool FITSFilter::load(XmlStreamReader * loader) {
    Q_UNUSED(loader)
    return false;
}


