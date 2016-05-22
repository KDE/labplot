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
#include "backend/datasources/FileDataSource.h"
#include "backend/core/column/Column.h"

#include <QDebug>

/*!
 * \class FITSFilter
 * \brief Manages the import/export of data from/to a FITS file.
 * \since 2.2.0
 * \ingroup datasources
 */
FITSFilter::FITSFilter():AbstractFileFilter(), d(new FITSFilterPrivate(this)) {
}

FITSFilter::~FITSFilter() {
    delete d;
}

void FITSFilter::read(const QString &fileName, AbstractDataSource *dataSource, AbstractFileFilter::ImportMode importMode) {
    d->readCHDU(fileName, dataSource, importMode);
}

QString FITSFilter::readChdu(const QString &fileName) {
    return d->readCHDU(fileName, NULL, AbstractFileFilter::Replace);
}

void FITSFilter::write(const QString &fileName, AbstractDataSource *dataSource) {
    d->writeCHDU(fileName, dataSource);
}

void FITSFilter::addNewKeyword(const FITSFilter::Keyword& keyword) {
    d->addNewKeyword(keyword);
}

void FITSFilter::updateKeyword(Keyword &keyword, const QString &newKey, const QString &newValue, const QString &newComment, KeywordUpdateMode mode) {
    d->updateKeyword(keyword, newKey, newValue, newComment, mode);
}

void FITSFilter::deleteKeyword(const Keyword &keyword) {
    d->deleteKeyword(keyword);
}

void FITSFilter::renameKeywordKey(const Keyword &keyword, const QString &newKey) {
    d->renameKeywordKey(keyword, newKey);
}

void FITSFilter::parseHeader(const QString &fileName, QTableWidget *headerEditTable){
    d->parseHeader(fileName, headerEditTable);
}

void FITSFilter::parseExtensions(const QString &fileName, QTreeWidgetItem *root) {
    d->parseExtensions(fileName, root);
}

void FITSFilter::loadFilterSettings(const QString& fileName) {
    Q_UNUSED(fileName)
}

void FITSFilter::saveFilterSettings(const QString& fileName) const {
    Q_UNUSED(fileName)
}

QStringList FITSFilter::standardKeywords() {
    return QStringList() << "(blank)" << "CROTAn"   << "EQUINOX"  << "NAXISn"   << "TBCOLn" << "TUNITn"
                         << "AUTHOR"  << "CRPIXn"   << "EXTEND"   << "OBJECT"   << "TDIMn"  << "TZEROn"
                         << "BITPIX"  << "CRVALn"   << "EXTLEVEL" << "OBSERVER" << "TDISPn" << "XTENSION"
                         << "BLANK"   << "CTYPEn"   << "EXTNAME"  << "ORIGIN"   << "TELESCOP"
                         << "BLOCKED" << "DATAMAX"  << "EXTVER"   << "PCOUNT"   << "TFIELDS"
                         << "BSCALE"  << "DATAMIN"  << "GCOUNT"   << "PSCALn"   << "TFORMn"
                         << "BUNIT"   << "DATE"     << "GROUPS"   << "PTYPEn"   << "THEAP"
                         << "BZERO"   << "DATE-OBS" << "HISTORY"  << "PZEROn"   << "TNULLn"
                         << "CDELTn"  << "END"      << "INSTRUME" << "REFERENC" << "TSCALn"
                         << "COMMENT" << "EPOCH"    << "NAXIS"    << "SIMPLE"   << "TTYPEn";
}

//#####################################################################
//################### Private implementation ##########################
//#####################################################################

FITSFilterPrivate::FITSFilterPrivate(FITSFilter* owner) :
    q(owner) {
}

/*!
 * \brief Read the current header data unit from file \a filename in data source \a dataSource in
    \a importMode import mode
 * \param fileName the name of the file to be read
 * \param dataSource the data source to be filled
 * \param importMode
 */
QString FITSFilterPrivate::readCHDU(const QString &fileName, AbstractDataSource *dataSource, AbstractFileFilter::ImportMode importMode) {

    #ifdef HAVE_FITS
    int status = 0;

    if(fits_open_file(&fitsFile, fileName.toLatin1(), READONLY, &status)) {
        printError(status);
        return QString();
    }

    int chduType;

    if (fits_get_hdu_type(fitsFile, &chduType, &status)) {
        printError(status);
        return QString();
    }

    int bitpix;
    int naxis;
    int maxdim = 2;
    long naxes[2];
    long actualRows;
    int actualCols;
    int columnOffset;

    long pixelCount;
    double* data;
    QStringList columnNames;
    QList<int> columnsWidth;

    status = 0;

    if(chduType == IMAGE_HDU) {
        if (fits_get_img_param(fitsFile, maxdim,&bitpix, &naxis, naxes, &status)) {
            printError(status);
            return QString();
        }

        pixelCount = naxes[0] * naxes[1];
        data = new double[pixelCount];

        if (!data) {
            qDebug() << "Not enough memory for data";
            return QString();
        }
        int anynull;
        int nullval = 0;

        if (fits_read_img(fitsFile, TDOUBLE, 1, pixelCount, &nullval, data, &anynull, &status)) {
            printError(status);
            return QString();
        }

        QVector<QVector<double>*> dataPointers;
        actualRows = naxes[1];
        actualCols = naxes[0];

        if (dataSource!=NULL) {
            columnOffset = dataSource->create(dataPointers, importMode, actualRows, actualCols);
        }
        if (dataSource!= NULL)
        for (int i = 0; i < actualRows; ++i) {
            for (int j = 0; j < actualCols; ++j) {
                dataPointers[j]->operator [](i) = data[i * actualCols + j];
            }
        }

        delete data;

        //TODO
        //dataString

    } else if ((chduType == ASCII_TBL) || (chduType == BINARY_TBL)) {
        fits_get_num_cols(fitsFile, &actualCols, &status);
        fits_get_num_rows(fitsFile, &actualRows, &status);
        columnsWidth.reserve(actualCols);
        int colWidth;
        char ttypenKeyword[FLEN_KEYWORD];
        char columnName[FLEN_VALUE];
        for (int col = 1; col <=actualCols; ++col) {
            fits_make_keyn("TTYPE", col, ttypenKeyword, &status);
            fits_read_key(fitsFile, TSTRING, ttypenKeyword, columnName, NULL, &status);
            fits_get_col_display_width(fitsFile, col, &colWidth, &status);

            columnsWidth.append(colWidth);
            columnNames.append(QString(columnName));
        }
        char* array;
        for (int row = 1; row <= actualRows; ++row) {
            for (int col = 1; col <= actualCols; ++col) {
                if(fits_read_col_str(fitsFile, col, row, 1, 1, NULL, &array, 0, &status)) {
                    printError(status);
                }
                //TODO
            }
        }

    } else {
        qDebug() << "Incorrect header type!";
    }

    // make everything undo/redo-able again
    // set column comments in spreadsheet
    Spreadsheet* spreadsheet = dynamic_cast<Spreadsheet*>(dataSource);
    if (spreadsheet) {
        QString comment = i18np("numerical data, %1 element", "numerical data, %1 elements", actualRows);
        for ( int n=0; n<actualCols; n++ ){
            Column* column = spreadsheet->column(columnOffset+n);
            column->setComment(comment);
            if ((chduType == ASCII_TBL) || (chduType == BINARY_TBL)) {
                column->setName(columnNames.at(n));
                column->setWidth(columnsWidth.at(n));
            }
            column->setUndoAware(true);
            if (importMode==AbstractFileFilter::Replace) {
                column->setSuppressDataChangedSignal(false);
                column->setChanged();
            }
        }
        spreadsheet->setUndoAware(true);
        return QString();
    }

    Matrix* matrix = dynamic_cast<Matrix*>(dataSource);
    if (matrix) {
        matrix->setSuppressDataChangedSignal(false);
        matrix->setChanged();
        matrix->setUndoAware(true);
    }

    fits_close_file(fitsFile, &status);

    #else
    Q_UNUSED(fileName)
    Q_UNUSED(dataSource)
    Q_UNUSED(importMode)
    #endif
    return QString();
}

/*!
 * \brief Export from data source \a dataSource to file \a fileName
 * \param fileName
 * \param dataSource
 */

void FITSFilterPrivate::writeCHDU(const QString &fileName, AbstractDataSource *dataSource) {
    Q_UNUSED(fileName)
    Q_UNUSED(dataSource)
}

/*!
 * \brief Return a list of the available extensions names in file \a filename
 * \param fileName
 */
QStringList FITSFilterPrivate::extensionNames(const QString& fileName) {
#ifdef HAVE_FITS
    QStringList extensionNames;
    int status = 0;

    if (fits_open_file(&fitsFile, fileName.toLatin1(), READONLY, &status )) {
        printError(status);
        return QStringList();
    }
    int hduCount;

    if (fits_get_num_hdus(fitsFile, &hduCount, &status)) {
        printError(status);
        return QStringList();
    }
    int imageCount = 0;
    int asciiTableCount = 0;
    int binaryTableCount = 0;
    for (int currentHDU = 1; currentHDU <= hduCount; ++currentHDU) {
        int hduType;
        status = 0;

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
        char* keyVal = new char[FLEN_VALUE];
        QString extName;
        if (!fits_read_keyword(fitsFile,"EXTNAME", keyVal, NULL, &status)) {
            extName = QString(keyVal);
            extName = extName.mid(1, extName.length() -2).simplified();
        }
        else {
            printError(status);
            status = 0;
            if (!fits_read_keyword(fitsFile, "HDUNAME", keyVal, NULL, &status)) {
                extName = QString(keyVal);
                extName = extName.mid(1, extName.length() -2).simplified();
            } else {
                status = 0;
                printError(status);
                switch (hduType) {
                case IMAGE_HDU:
                    if (imageCount == 1) {
                        extName = i18n("Primary header");
                    } else {
                        extName = i18n("IMAGE #%1").arg(imageCount);
                    }
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
        delete keyVal;
        status = 0;
        extensionNames << extName.trimmed();
        if(fits_movrel_hdu(fitsFile, 1, NULL, &status)) {
            printError(status);
        }
    }

    if (status == END_OF_FILE) {
        status = 0;
    }

    fits_close_file(fitsFile, &status);
    return extensionNames;
#else
    Q_UNUSED(fileName)
    return QStringList();
#endif
}

/*!
 * \brief Prints the error text corresponding to the status code \a status
 * \param status the status code
 */
void FITSFilterPrivate::printError(int status) const {
#ifdef HAVE_FITS
    if (status) {
        char errorText[FLEN_ERRMSG];
        fits_get_errstatus(status, errorText );
        qDebug() << QLatin1String(errorText);
    }
#else
    Q_UNUSED(status)
#endif
}

/*!
 * \brief Add the keyword \a keyword to the current header unit
 * \param keyword keyword to be added
 */

void FITSFilterPrivate::addNewKeyword(const FITSFilter::Keyword& keyword) {
#ifdef HAVE_FITS
    int status = 0;
    if (!keyword.key.compare(QLatin1String("COMMENT"))) {
        if (fits_write_comment(fitsFile, keyword.key.toLatin1(), &status)) {
            printError(status);
        }
    } else if (!keyword.key.compare(QLatin1String("HISTORY"))) {
        if (fits_write_history(fitsFile, keyword.key.toLatin1(), &status)) {
            printError(status);
        }
    } else if (!keyword.key.compare(QLatin1String("DATE"))) {
        if (fits_write_date(fitsFile, &status)) {
            printError(status);
        }
    } else {
        int ok = 0;
        if (keyword.key.length() <= FLEN_KEYWORD) {
            ok++;
            if (keyword.value.length() <= FLEN_VALUE) {
                ok++;
                if(keyword.comment.length() <= FLEN_COMMENT) {
                 ok++;
                }
            }
        }
        if (ok == 3) {
            //add key
        } else if ( ok == 2) {
            //comment too long
        } else if ( ok == 1) {
            //value too long
        } else {
            //keyword too long
        }
    }
#else
    Q_UNUSED(keyword)
#endif
}
/*!
 * \brief Update the value and/or comment of keyword \a keyword in the current header unit
 * \param keyword the keyword to be updated
 * \param newKey the new key
 * \param newValue the new value
 * \param newComment the new comment
 * \param updateMode the update mode in which the keyword is updated
 */
void FITSFilterPrivate::updateKeyword(FITSFilter::Keyword& keyword,const QString& newKey, const QString& newValue,
                                      const QString& newComment, FITSFilter::KeywordUpdateMode updateMode) {
#ifdef HAVE_FITS
    int status = 0;
    bool updated = false;

    switch (updateMode) {
    case FITSFilter::UpdateValueComment:{
        bool ok;
        int intValue;
        int doubleValue;

        doubleValue = keyword.value.toDouble(&ok);
        if (ok) {
            if (fits_update_key(fitsFile,TDOUBLE, keyword.key.toLatin1(), &doubleValue,
                                keyword.comment.toLatin1(), &status)) {
                updated = true;
            } else {
                printError(status);
            }
        }
        if (!updated) {
            intValue = keyword.value.toInt(&ok);
            if (ok) {
                if (fits_update_key(fitsFile,TINT, keyword.key.toLatin1(), &intValue,
                                    keyword.comment.toLatin1(), &status)) {
                    updated = true;
                } else {
                    printError(status);
                }
            }
        }
        if (!updated) {
            intValue = keyword.value.toInt(&ok);
            if (ok) {
                if (fits_update_key(fitsFile,TSTRING, keyword.key.toLatin1(), keyword.value.toLatin1().data(),
                                    keyword.comment.toLatin1(), &status)) {
                    updated = true;
                } else {
                    printError(status);
                }
            }
        }
        if (updated) {
            keyword.value = newValue;
            keyword.comment = newComment;
        }
        break;
    }
    case FITSFilter::UpdateKeyname: {
        if (fits_modify_name(fitsFile, keyword.key.toLatin1(), newKey.toLatin1(), &status )) {
            printError(status);
        } else {
            updated = true;
            keyword.key = newKey;
        }
        break;
    }
    case FITSFilter::UpdateComment: {
        if (fits_modify_comment(fitsFile, keyword.key.toLatin1(), newComment.toLatin1(), &status)) {
            printError(status);
        } else {
            updated = true;
            keyword.comment = newComment;
        }
        break;
    }
    case FITSFilter::UpdateWithBlankValue: {
        if (fits_update_key_null(fitsFile, keyword.key.toLatin1(), NULL, &status)) {
            printError(status);
        } else {
            updated = true;
            keyword.value = "";
        }
        break;
    }

    case FITSFilter::UpdateWithoutComment: {
        break;
    }
    }

    if (updated) {
        //TODO
        //messagebox?
        qDebug() << "Keyword updated successfully!";
    } else {
        qDebug() << "Failed to update keyword!";
    }
#else
    Q_UNUSED(newKey)
    Q_UNUSED(newComment)
    Q_UNUSED(newValue)
    Q_UNUSED(updateMode)
#endif
}

/*!
 * \brief Delete the keyword \a keyword from the current header unit
 * \param keyword the keyword to delete
 */

//TODO return bool
void FITSFilterPrivate::deleteKeyword(const FITSFilter::Keyword &keyword) {
#ifdef HAVE_FITS
    if (!keyword.key.isEmpty()) {
        int status = 0;
        if (fits_delete_key(fitsFile, keyword.key.toLatin1(), &status)) {
            printError(status);
        }
    }
#else
    Q_UNUSED(keyword)
#endif
}

/*!
 * \brief Rename the keyname of \a keyword, preserving the value and comment fields
 * \param keyword the keyword to rename
 * \param newKey the new keyname of \a keyword
 */
void FITSFilterPrivate::renameKeywordKey(const FITSFilter::Keyword &keyword, const QString &newKey) {
    Q_UNUSED(keyword)
    Q_UNUSED(newKey)
}

/*!
 * \brief Returns a list of keywords in the current header
 * \param fileName the file to open
 * \return
 */
QList<FITSFilter::Keyword> FITSFilterPrivate::chduKeywords(const QString& fileName) {
#ifdef HAVE_FITS
    int status = 0;

    if (fits_open_file(&fitsFile, fileName.toLatin1(), READONLY, &status )) {
        printError(status);
        return QList<FITSFilter::Keyword> ();
    }
    int numberOfKeys;
    if (fits_get_hdrspace(fitsFile, &numberOfKeys, NULL, &status)){
        printError(status);
        return QList<FITSFilter::Keyword> ();
    }

    QList<FITSFilter::Keyword> keywords;
    keywords.reserve(numberOfKeys);
    char* key = new char[FLEN_KEYWORD];
    char* value = new char[FLEN_VALUE];
    char* comment = new char[FLEN_COMMENT];
    for (int i = 1; i <= numberOfKeys; ++i) {
        QStringList recordValues;
        FITSFilter::Keyword keyword;

        if (fits_read_keyn(fitsFile, i, key, value, comment, &status)) {
            printError(status);
            status = 0;
            continue;
        }
        recordValues << QString(key) << QString(value) << QString(comment);

        keyword.key = recordValues[0].simplified();
        keyword.value = recordValues[1].simplified();
        keyword.comment = recordValues[2].simplified();

        keywords.append(keyword);
    }
    delete key;
    delete value;
    delete comment;

    fits_close_file(fitsFile, &status);

    return keywords;
#else
    Q_UNUSED(fileName)
    return QList<FITSFilter::Keyword>();
#endif
}

void FITSFilterPrivate::parseHeader(const QString &fileName, QTableWidget *headerEditTable) {
    QList<FITSFilter::Keyword> keywords = chduKeywords(fileName);

    headerEditTable->setRowCount(keywords.size());
    QTableWidgetItem* item;
    for (int i = 0; i < keywords.size(); ++i) {
        item = new QTableWidgetItem(keywords.at(i).key);
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

void FITSFilterPrivate::parseExtensions(const QString &fileName, QTreeWidgetItem *root) {
    QStringList extensions = extensionNames(fileName);
    QTreeWidgetItem* treeNameItem = new QTreeWidgetItem((QTreeWidgetItem*)0, QStringList() << fileName);
    root->addChild(treeNameItem);
    foreach (const QString& ext, extensions) {
        QTreeWidgetItem* treeItem = new QTreeWidgetItem((QTreeWidgetItem*)0, QStringList() << ext);
        treeNameItem->addChild(treeItem);
    }
}

/*!
 * \brief FITSFilterPrivate::~FITSFilterPrivate
 */

FITSFilterPrivate::~FITSFilterPrivate() {
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
