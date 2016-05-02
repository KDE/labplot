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
class FITSFilterPrivate;
class FITSFilter : public AbstractFileFilter{
    Q_OBJECT

  public:

    FITSFilter();
    ~FITSFilter();

    void read(const QString & fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode=AbstractFileFilter::Replace);
    void write(const QString & fileName, AbstractDataSource* dataSource);

    virtual void save(QXmlStreamWriter*) const;
    virtual bool load(XmlStreamReader*);
    QStringList extensionNames() const;

  private:
    FITSFilterPrivate* const d;
    friend class FITSFilterPrivate;
};
#endif // FITSFILTER_H
