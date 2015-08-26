/***************************************************************************
    File                 : DataPickerCurve.h
    Project              : LabPlot
    Description          : container for Curve-Point and Datasheet/Spreadsheet
                           of datapicker
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)
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

#ifndef DATAPICKERCURVE_H
#define DATAPICKERCURVE_H

#include "backend/datapicker/Image.h"
#include "backend/core/AbstractAspect.h"

class CustomItem;
class QAction;
class DataPickerCurvePrivate;
class Column;
class Spreadsheet;
class AbstractColumn;

class DataPickerCurve: public AbstractAspect {
    Q_OBJECT

    public:
        explicit DataPickerCurve(const QString&);
        virtual ~DataPickerCurve();

        virtual QIcon icon() const;
        virtual QMenu* createContextMenu();
        void setPrinting(bool);

        void addCustomItem(const QPointF&);

        BASIC_D_ACCESSOR_DECL(bool, visible, Visible)
        BASIC_D_ACCESSOR_DECL(Image::Errors, curveErrorTypes, CurveErrorTypes)

        POINTER_D_ACCESSOR_DECL(AbstractColumn, posXColumn, PosXColumn)
        QString& posXColumnPath() const;
        POINTER_D_ACCESSOR_DECL(AbstractColumn, posYColumn, PosYColumn)
        QString& posYColumnPath() const;
        POINTER_D_ACCESSOR_DECL(AbstractColumn, plusDeltaXColumn, PlusDeltaXColumn)
        QString& plusDeltaXColumnPath() const;
        POINTER_D_ACCESSOR_DECL(AbstractColumn, minusDeltaXColumn, MinusDeltaXColumn)
        QString& minusDeltaXColumnPath() const;
        POINTER_D_ACCESSOR_DECL(AbstractColumn, plusDeltaYColumn, PlusDeltaYColumn)
        QString& plusDeltaYColumnPath() const;
        POINTER_D_ACCESSOR_DECL(AbstractColumn, minusDeltaYColumn, MinusDeltaYColumn)
        QString& minusDeltaYColumnPath() const;

        virtual void save(QXmlStreamWriter*) const;
        virtual bool load(XmlStreamReader*);

        typedef DataPickerCurvePrivate Private;

    protected:
        DataPickerCurve(const QString& name, DataPickerCurvePrivate* dd);
        DataPickerCurvePrivate* const d_ptr;

    private slots:
        void visibilityChanged();
        void updateDatasheet();

    public slots:
        void updateData(const CustomItem*);

    private:
        Q_DECLARE_PRIVATE(DataPickerCurve)
        void init();
        void initAction();
        Column *appendColumn(const QString&, Spreadsheet*);

        QAction* visibilityAction;
        QAction* updateDatasheetAction;
};
#endif // DATAPICKERCURVE_H
