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
#include <QMultiMap>
#include <QString>
#include <QHeaderView>
#include <QTableWidgetItem>

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

QString FITSFilter::readChdu(const QString &fileName, int lines) {
    return d->readCHDU(fileName, NULL, AbstractFileFilter::Replace, lines);
}

void FITSFilter::write(const QString &fileName, AbstractDataSource *dataSource) {
    d->writeCHDU(fileName, dataSource);
}

void FITSFilter::addNewKeyword(const QString &filename, const QList<Keyword> &keywords) {
    d->addNewKeyword(filename, keywords);
}

void FITSFilter::updateKeyword(Keyword &keyword, const QString &newKey, const QString &newValue, const QString &newComment, KeywordUpdateMode mode) {
    d->updateKeyword(keyword, newKey, newValue, newComment, mode);
}

void FITSFilter::deleteKeyword(const QString &fileName, const QList<Keyword>& keywords) {
    d->deleteKeyword(fileName, keywords);
}

void FITSFilter::renameKeywordKey(const Keyword &keyword, const QString &newKey) {
    d->renameKeywordKey(keyword, newKey);
}

void FITSFilter::parseHeader(const QString &fileName, QTableWidget *headerEditTable, bool readKeys, const QList<Keyword> &keys){
    d->parseHeader(fileName, headerEditTable, readKeys, keys);
}

void FITSFilter::parseExtensions(const QString &fileName, QTreeWidget *tw, bool checkPrimary) {
    d->parseExtensions(fileName, tw, checkPrimary);
}

QList<FITSFilter::Keyword> FITSFilter::chduKeywords(const QString &fileName) {
    return d->chduKeywords(fileName);
}

void FITSFilter::loadFilterSettings(const QString& fileName) {
    Q_UNUSED(fileName)
}

void FITSFilter::saveFilterSettings(const QString& fileName) const {
    Q_UNUSED(fileName)
}

/*!
 * \brief FITSFilter::standardKeywords
 *      contains the {StandardKeywords \ MandatoryKeywords} keywords
 * \return A list of keywords
 */
QStringList FITSFilter::standardKeywords() {
    return QStringList() << QLatin1String("(blank)") << QLatin1String("CROTAn")   << QLatin1String("EQUINOX")  << QLatin1String("NAXISn")   << QLatin1String("TBCOLn") << QLatin1String("TUNITn")
                         << QLatin1String("AUTHOR")  << QLatin1String("CRPIXn")   << QLatin1String("EXTEND")   << QLatin1String("OBJECT")   << QLatin1String("TDIMn")  << QLatin1String("TZEROn")
                         << QLatin1String("BITPIX")  << QLatin1String("CRVALn")   << QLatin1String("EXTLEVEL") << QLatin1String("OBSERVER") << QLatin1String("TDISPn") << QLatin1String("XTENSION")
                         << QLatin1String("BLANK")   << QLatin1String("CTYPEn")   << QLatin1String("EXTNAME")  << QLatin1String("ORIGIN")   << QLatin1String("TELESCOP")
                         << QLatin1String("BLOCKED") << QLatin1String("DATAMAX")  << QLatin1String("EXTVER")
                         << QLatin1String("BSCALE")  << QLatin1String("DATAMIN")  << QLatin1String("PSCALn")  << QLatin1String("TFORMn")
                         << QLatin1String("BUNIT")   << QLatin1String("DATE")     << QLatin1String("GROUPS")   << QLatin1String("PTYPEn")   << QLatin1String("THEAP")
                         << QLatin1String("BZERO")   << QLatin1String("DATE-OBS") << QLatin1String("HISTORY")  << QLatin1String("PZEROn")   << QLatin1String("TNULLn")
                         << QLatin1String("CDELTn")  << QLatin1String("INSTRUME") << QLatin1String("REFERENC") << QLatin1String("TSCALn")
                         << QLatin1String("COMMENT") << QLatin1String("EPOCH")    << QLatin1String("NAXIS")    << QLatin1String("SIMPLE")   << QLatin1String("TTYPEn");
}

QStringList FITSFilter::mandatoryImageExtensionKeywords() {
    return QStringList() << QLatin1String("XTENSION") << QLatin1String("BITPIX")
                         << QLatin1String("NAXIS") << QLatin1String("PCOUNT")
                         << QLatin1String("GCOUNT") << QLatin1String("END");
}

QStringList FITSFilter::mandatoryTableExtensionKeywords() {
    return QStringList() << QLatin1String("XTENSION") << QLatin1String("BITPIX")
                         << QLatin1String("NAXIS") << QLatin1String("NAXIS1")
                         << QLatin1String("NAXIS2") << QLatin1String("PCOUNT")
                         << QLatin1String("GCOUNT") << QLatin1String("TFIELDS")
                         << QLatin1String("END");
}

void FITSFilter::setStartColumn(const int column) {
    d->startColumn = column;
}

int FITSFilter::startColumn() const {
    return d->startColumn;
}

void FITSFilter::setEndColumn(const int column) {
    d->endColumn = column;
}

int FITSFilter::endColumn() const {
    return d->endColumn;
}

void FITSFilter::setStartRow(const int row) {
    d->startRow = row;
}

int FITSFilter::startRow() const {
    return d->startRow;
}

void FITSFilter::setEndRow(const int row) {
    d->endRow = row;
}

int FITSFilter::endRow() const {
    return d->endRow;
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
QString FITSFilterPrivate::readCHDU(const QString &fileName, AbstractDataSource *dataSource, AbstractFileFilter::ImportMode importMode, int lines) {

    #ifdef HAVE_FITS
    int status = 0;

    if(fits_open_file(&fitsFile, fileName.toLatin1(), READONLY, &status)) {
        qDebug() << fileName;
        printError(status);
        return QString();
    }

    int chduType;

    if (fits_get_hdu_type(fitsFile, &chduType, &status)) {
        printError(status);
        return QString();
    }

    long actualRows;
    int actualCols;
    int columnOffset = 0;

    QStringList dataString;
    int noDataSource = (dataSource == NULL);

    if(chduType == IMAGE_HDU) {
        int bitpix;
        int naxis;
        int maxdim = 2;
        long naxes[2];

        long pixelCount;
        double* data;
        if (fits_get_img_param(fitsFile, maxdim,&bitpix, &naxis, naxes, &status)) {
            printError(status);
            return QString();
        }

        actualRows = naxes[1];
        actualCols = naxes[0];
        if (lines == -1) {
                lines = actualRows;
        } else {
            if (lines > actualRows) {
                lines = actualRows;
            }
        }

        pixelCount = lines * actualCols;
        data = new double[pixelCount];

        if (!data) {
            qDebug() << i18n("Not enough memory for data");
            return QString();
        }

        if (fits_read_img(fitsFile, TDOUBLE, 1, pixelCount, NULL, data, NULL, &status)) {
            printError(status);
            return QString();
        }

        QVector<QVector<double>*> dataPointers;
        dataPointers.reserve(actualCols);

        dataString.reserve(lines * actualCols);
        if (!noDataSource) {
            columnOffset = dataSource->create(dataPointers, importMode, lines, actualCols);
        }
        QLatin1String ws = QLatin1String(" ");
        QLatin1String nl = QLatin1String("\n");
        //TODO startColumn/end..
        for (int i = 0; i < lines; ++i) {
            for (int j = 0; j < actualCols; ++j) {
                if (!noDataSource) {
                    dataPointers[j]->operator [](i) = data[i * actualCols + j];
                } else {
                    dataString << QString::number(data[i*actualCols +j]) << ws;
                }
            }
            dataString << nl;
        }

        delete[] data;

        Spreadsheet* spreadsheet = dynamic_cast<Spreadsheet*>(dataSource);
        if (spreadsheet) {
            QString comment = i18np("numerical data, %1 element", "numerical data, %1 elements", actualRows);
            for ( int n=0; n<actualCols; n++ ){
                Column* column = spreadsheet->column(columnOffset+n);
                column->setComment(comment);
                column->setUndoAware(true);
                if (importMode==AbstractFileFilter::Replace) {
                    column->setSuppressDataChangedSignal(false);
                    column->setChanged();
                }
            }
            spreadsheet->setUndoAware(true);
            fits_close_file(fitsFile, &status);

            return dataString.join(QLatin1String(""));
        }

        Matrix* matrix = dynamic_cast<Matrix*>(dataSource);
        if (matrix) {
            matrix->setSuppressDataChangedSignal(false);
            matrix->setChanged();
            matrix->setUndoAware(true);
        }

    } else if ((chduType == ASCII_TBL) || (chduType == BINARY_TBL)) {
        fits_get_num_cols(fitsFile, &actualCols, &status);
        fits_get_num_rows(fitsFile, &actualRows, &status);
        QStringList columnNames;
        QList<int> columnsWidth;
        columnsWidth.reserve(actualCols);
        columnNames.reserve(actualCols);
        int colWidth;
        char ttypenKeyword[FLEN_KEYWORD];
        char columnName[FLEN_VALUE];
        for (int col = 1; col <=actualCols; ++col) {
            fits_make_keyn("TTYPE", col, ttypenKeyword, &status);
            fits_read_key(fitsFile, TSTRING, ttypenKeyword, columnName, NULL, &status);
            fits_get_col_display_width(fitsFile, col, &colWidth, &status);

            columnsWidth.append(colWidth);
            columnNames.append(QLatin1String(columnName));
        }

        status = 0;
        if (lines == -1) {
            lines = actualRows;
        } else if (lines > actualRows) {
            lines = actualRows;
        }

        QVector<QStringList*> dataPointers;
        dataPointers.reserve(actualCols);

        if (!noDataSource) {
            Spreadsheet* spreadsheet = dynamic_cast<Spreadsheet*>(dataSource);

            if(spreadsheet) {
                spreadsheet->setUndoAware(false);
                columnOffset = spreadsheet->resize(importMode,columnNames,actualCols);

                if (importMode==AbstractFileFilter::Replace) {
                    spreadsheet->clear();
                    spreadsheet->setRowCount(lines);
                }else{
                    if (spreadsheet->rowCount() < lines)
                        spreadsheet->setRowCount(lines);
                }
                for (int n=0; n<actualCols; n++ ){
                    spreadsheet->column(columnOffset+ n)->setColumnMode(AbstractColumn::Text);
                    QStringList* list = static_cast<QStringList* >(spreadsheet->column(n + columnOffset)->data());
                    list->reserve(lines);
                    dataPointers.push_back(list);
                }
            }
            else {
                return dataString.join(QLatin1String(""));
            }
        }

        char* array = new char[1000];
        //TODO startColumn/end..

        for (int row = 1; row <= lines; ++row) {
            for (int col = 1; col <= actualCols; ++col) {

                if(fits_read_col_str(fitsFile, col, row, 1, 1, NULL, &array, 0, &status)) {
                    qDebug() << "error on table reading";
                    printError(status);
                    dataString << QLatin1String(" ");
                }
                if (!noDataSource) {
                    QString str = QString::fromLatin1(array);
                    dataPointers[col-1]->append(str.simplified());
                } else {
                    //TODO - whitespaces can appear in cells too
                    QString tmpColstr = QString::fromLatin1(array);
                    tmpColstr = tmpColstr.simplified();
                    tmpColstr.replace(QLatin1String(" "), QLatin1String(""));

                    dataString << tmpColstr << QLatin1String(" ");
                }
            }
            dataString << QLatin1String("\n");
        }

        delete[] array;

        if (!noDataSource) {
            Spreadsheet* spreadsheet = dynamic_cast<Spreadsheet*>(dataSource);
            if (spreadsheet) {
                QString comment = i18np("numerical data, %1 element", "numerical data, %1 elements", actualRows);
                for ( int n=0; n<actualCols; n++ ){
                    Column* column = spreadsheet->column(columnOffset+n);
                    column->setComment(comment);
                    column->setUndoAware(true);
                    if (importMode==AbstractFileFilter::Replace) {
                        column->setSuppressDataChangedSignal(false);
                        column->setChanged();
                    }
                }
                spreadsheet->setUndoAware(true);
            }
        }
    } else {
        qDebug() << i18n("Incorrect header type!");
    }

    fits_close_file(fitsFile, &status);

#else
    Q_UNUSED(fileName)
    Q_UNUSED(dataSource)
    Q_UNUSED(importMode)
    return QString();
    #endif
    return dataString.join(QLatin1String(""));
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
 * \brief Return a map of the available extensions names in file \a filename
 *        The keys of the map are the extension types, the values are the names
 * \param fileName
 */
 QMultiMap<QString, QString> FITSFilterPrivate::extensionNames(const QString& fileName) {
#ifdef HAVE_FITS
    QMultiMap<QString, QString> extensions;
    int status = 0;

    if (fits_open_file(&fitsFile, fileName.toLatin1(), READONLY, &status )) {
        printError(status);
        return QMultiMap<QString, QString>();
    }
    int hduCount;

    if (fits_get_num_hdus(fitsFile, &hduCount, &status)) {
        printError(status);
        return QMultiMap<QString, QString>();
    }
    int imageCount = 0;
    int asciiTableCount = 0;
    int binaryTableCount = 0;
    for (int currentHDU = 1; (currentHDU <= hduCount) && !status; ++currentHDU) {
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
            extName = QLatin1String(keyVal);
            extName = extName.mid(1, extName.length() -2).simplified();  
        }
        else {
            status = 0;
            if (!fits_read_keyword(fitsFile,"HDUNAME", keyVal, NULL, &status)) {
                extName = QLatin1String(keyVal);
                extName = extName.mid(1, extName.length() -2).simplified();
            } else {
                status = 0;
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
        extName = extName.trimmed();
        switch (hduType) {
        case IMAGE_HDU:
            extensions.insert(QLatin1String("IMAGES"), extName);
            break;
        case ASCII_TBL:
            extensions.insert(QLatin1String("TABLES"), extName);
            break;
        case BINARY_TBL:
            extensions.insert(QLatin1String("TABLES"), extName);
            break;
        }
        fits_movrel_hdu(fitsFile, 1, NULL, &status);
    }

    if (status == END_OF_FILE) {
        status = 0;
    }

    fits_close_file(fitsFile, &status);
    return extensions;
#else
    Q_UNUSED(fileName)
    return QMultiMap<QString, QString>();
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

void FITSFilterPrivate::addNewKeyword(const QString& fileName, const QList<FITSFilter::Keyword>& keywords) {
#ifdef HAVE_FITS
    int status = 0;
    if (fits_open_file(&fitsFile, fileName.toLatin1(), READWRITE, &status )) {
        printError(status);
        return;
    }
    foreach (const FITSFilter::Keyword& keyword, keywords) {
        status = 0;
        if (!keyword.key.compare(QLatin1String("COMMENT"))) {
            if (fits_write_comment(fitsFile, keyword.value.toLatin1(), &status)) {
                printError(status);
            }
        } else if (!keyword.key.compare(QLatin1String("HISTORY"))) {
            if (fits_write_history(fitsFile, keyword.value.toLatin1(), &status)) {
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
                bool ok;
                double val = keyword.value.toDouble(&ok);
                if (ok) {
                    if (fits_write_key(fitsFile,
                                       TDOUBLE,
                                       keyword.key.toLatin1().data(),
                                       &val,
                                       keyword.comment.toLatin1().data(),
                                       &status)) {
                        printError(status);
                    }
                } else {
                    if (fits_write_key(fitsFile,
                                       TSTRING,
                                       keyword.key.toLatin1().data(),
                                       keyword.value.toLatin1().data(),
                                       keyword.comment.toLatin1().data(),
                                       &status)) {
                        printError(status);
                    }
                }
            } else if ( ok == 2) {
                //comment too long
            } else if ( ok == 1) {
                //value too long
            } else {
                //keyword too long
            }
        }
    }
    fits_close_file(fitsFile, &status);
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

void FITSFilterPrivate::deleteKeyword(const QString& fileName, const QList<FITSFilter::Keyword> &keywords) {
#ifdef HAVE_FITS
    int status = 0;
    if (fits_open_file(&fitsFile, fileName.toLatin1(), READWRITE, &status )) {
        printError(status);
        return;
    }
    foreach (const FITSFilter::Keyword& keyword, keywords) {
        if (!keyword.key.isEmpty()) {
            status = 0;
            if (fits_delete_key(fitsFile, keyword.key.toLatin1(), &status)) {
                printError(status);
            }
        }
    }
    fits_close_file(fitsFile, &status);
#else
    Q_UNUSED(keyword)
    return false;
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
        recordValues << QLatin1String(key) << QLatin1String(value) << QLatin1String(comment);

        keyword.key = recordValues[0].simplified();
        keyword.value = recordValues[1].simplified();
        keyword.comment = recordValues[2].simplified();

        keywords.append(keyword);
    }
    delete[] key;
    delete[] value;
    delete[] comment;

    fits_close_file(fitsFile, &status);

    return keywords;
#else
    Q_UNUSED(fileName)
    return QList<FITSFilter::Keyword>();
#endif
}

void FITSFilterPrivate::parseHeader(const QString &fileName, QTableWidget *headerEditTable,
                                     bool readKeys, const QList<FITSFilter::Keyword>& keys) {

    QList<FITSFilter::Keyword> keywords;
    if (readKeys) {
         keywords = chduKeywords(fileName);
    } else {
        keywords = keys;
    }

    headerEditTable->setRowCount(keywords.size());
    QTableWidgetItem* item;
    for (int i = 0; i < keywords.size(); ++i) {
        bool mandatory = FITSFilter::mandatoryImageExtensionKeywords().contains(keywords.at(i).key) ||
                FITSFilter::mandatoryTableExtensionKeywords().contains(keywords.at(i).key);
        item = new QTableWidgetItem(keywords.at(i).key);
        item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        if (mandatory) {
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        }
        headerEditTable->setItem(i, 0, item );

        item = new QTableWidgetItem(keywords.at(i).value);
        item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        if (mandatory) {
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        }
        headerEditTable->setItem(i, 1, item );

        item = new QTableWidgetItem(keywords.at(i).comment);
        item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        headerEditTable->setItem(i, 2, item );
    }

    headerEditTable->resizeColumnsToContents();
}

const QString FITSFilterPrivate::valueOf(const QString& fileName, const char *key) {
    int status = 0;
    if (fits_open_file(&fitsFile, fileName.toLatin1(), READONLY, &status )) {
        printError(status);
        return QString ();
    }

    char* keyVal = new char[FLEN_VALUE];
    QString keyValue;
    if (!fits_read_keyword(fitsFile,key, keyVal, NULL, &status)) {
        keyValue = QLatin1String(keyVal);
        keyValue = keyValue.simplified();
    } else {
        printError(status);
        delete[] keyVal;
        fits_close_file(fitsFile, &status);
        return QString();
    }

    delete[] keyVal;
    status = 0;
    fits_close_file(fitsFile, &status);
    return keyValue;
}

void FITSFilterPrivate::parseExtensions(const QString &fileName, QTreeWidget *tw, bool checkPrimary) {
    QMultiMap<QString, QString> extensions = extensionNames(fileName);
    QStringList imageExtensions = extensions.values(QLatin1String("IMAGES"));
    QStringList tableExtensions = extensions.values(QLatin1String("TABLES"));

    QTreeWidgetItem* root = tw->invisibleRootItem();
    QTreeWidgetItem* treeNameItem = new QTreeWidgetItem((QTreeWidgetItem*)0, QStringList() << fileName);
    root->addChild(treeNameItem);
    treeNameItem->setExpanded(true);

    QTreeWidgetItem* imageExtensionItem = new QTreeWidgetItem((QTreeWidgetItem*)0, QStringList() << i18n("Images"));
    QString primaryHeaderNaxis = valueOf(fileName, "NAXIS");
    int naxis = primaryHeaderNaxis.toInt();
    foreach (const QString& ext, imageExtensions) {
        QTreeWidgetItem* treeItem = new QTreeWidgetItem((QTreeWidgetItem*)0, QStringList() << ext);
        if (ext == QLatin1String("Primary header")) {
            if (checkPrimary && naxis == 0) {
                continue;
            }
        }
        imageExtensionItem->addChild(treeItem);
    }
    if (imageExtensionItem->childCount() > 0) {
        treeNameItem->addChild(imageExtensionItem);
        imageExtensionItem->setExpanded(true);
        imageExtensionItem->child(0)->setSelected(true);
        tw->setCurrentItem(imageExtensionItem->child(0));
    }

    if (tableExtensions.size() > 0) {
        QTreeWidgetItem* tableExtensionItem = new QTreeWidgetItem((QTreeWidgetItem*)0, QStringList() << i18n("Tables"));
        treeNameItem->addChild(tableExtensionItem);
        foreach (const QString& ext, tableExtensions) {
            QTreeWidgetItem* treeItem = new QTreeWidgetItem((QTreeWidgetItem*)0, QStringList() << ext);
            tableExtensionItem->addChild(treeItem);
        }
        tableExtensionItem->setExpanded(true);
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
