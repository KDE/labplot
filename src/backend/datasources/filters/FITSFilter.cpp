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

QStringList FITSFilter::extensionNames() const {
    return d->extensionNames();
}

//#####################################################################
//################### Private implementation ##########################
//#####################################################################

FITSFilterPrivate::FITSFilterPrivate(FITSFilter* owner) :
    q(owner), status(0) {
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

QStringList FITSFilterPrivate::extensionNames() const {
    return QStringList();
}


