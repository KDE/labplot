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

void FITSFilter::save(QXmlStreamWriter * writer) const {
    Q_UNUSED(writer)
}

bool FITSFilter::load(XmlStreamReader * loader) {
    Q_UNUSED(loader)
    return false;
}

QStringList FITSFilter::extensionNames(const QString &fileName) {
    return d->extensionNames(fileName);
}

//#####################################################################
//################### Private implementation ##########################
//#####################################################################

FITSFilterPrivate::FITSFilterPrivate(FITSFilter* owner) :
    q(owner) {
}

void FITSFilterPrivate::readCHDU(const QString &fileName, AbstractDataSource *dataSource, AbstractFileFilter::ImportMode importMode) {
    Q_UNUSED(fileName)
    Q_UNUSED(dataSource)
    Q_UNUSED(importMode)
}

void FITSFilterPrivate::writeCHDU(const QString &fileName, AbstractDataSource *dataSource) {
    Q_UNUSED(fileName)
    Q_UNUSED(dataSource)
}

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

    return extensionNames;
}

void FITSFilterPrivate::printError(int status) const {
    if (status) {
        char errorText[80];
        fits_get_errstatus(status, errorText );
    }
}


