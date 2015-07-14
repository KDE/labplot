
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

#ifndef DATAPICKER_H
#define DATAPICKER_H

#include "backend/core/AbstractPart.h"
#include "backend/core/AbstractScriptingEngine.h"

class Spreadsheet;
class Column;
class Image;
class QXmlStreamWriter;
class XmlStreamReader;

class Datapicker : public AbstractPart, public scripted {
	Q_OBJECT

	public:
		Datapicker(AbstractScriptingEngine* engine, const QString& name);
        enum DataColumnType { PositionX, PositionY, PlusDeltaX, MinusDeltaX, PlusDeltaY, MinusDeltaY };

		virtual QIcon icon() const;
		virtual QMenu* createContextMenu();
		virtual QWidget* view() const;
        void initDefault();

		Spreadsheet* currentSpreadsheet() const;
        Image* currentImage() const;
		void setChildSelectedInView(int index, bool selected);
        void addDataToSheet(double, int, const Datapicker::DataColumnType&);

        Spreadsheet* m_datasheet;
        Image* m_image;

		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);

    public slots:
        virtual void childSelected(const AbstractAspect*);

    private:
        static QString columnNameFromType(Datapicker::DataColumnType);
        int* columnIndexFromType(Datapicker::DataColumnType);
        void addDataToColumn(int, double, DataColumnType);
        void addDatasheet();

        //column index
        int positionXIndex;
        int positionYIndex;
        int plusDeltaXIndex;
        int minusDeltaXIndex;
        int plusDeltaYIndex;
        int minusDeltaYIndex;

    private slots:
		virtual void childDeselected(const AbstractAspect*);
        void handleColumnRemoved(const AbstractAspect*);

	signals:
        void datapickerItemSelected(int);
};

#endif
