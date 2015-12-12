/***************************************************************************
    File                 : DatapickerCurve.h
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

#include "backend/core/AbstractAspect.h"
#include "backend/lib/macros.h"
#include "backend/datapicker/DatapickerImage.h"

class CustomPoint;
class QAction;
class DatapickerCurvePrivate;
class Column;
class Spreadsheet;
class AbstractColumn;

class DatapickerCurve: public AbstractAspect {
	Q_OBJECT

public:
	explicit DatapickerCurve(const QString&);
	virtual ~DatapickerCurve();

	enum ErrorType { NoError, SymmetricError, AsymmetricError };
	struct Errors {
		ErrorType x;
		ErrorType y;
	};

	virtual QIcon icon() const;
	virtual QMenu* createContextMenu();
	void setPrinting(bool);
	void setSelectedInView(const bool);
	void addDatasheet(const DatapickerImage::GraphType&);
    void updateData(const CustomPoint*);

	BASIC_D_ACCESSOR_DECL(Errors, curveErrorTypes, CurveErrorTypes)
	POINTER_D_ACCESSOR_DECL(AbstractColumn, posXColumn, PosXColumn)
	QString& posXColumnPath() const;
	POINTER_D_ACCESSOR_DECL(AbstractColumn, posYColumn, PosYColumn)
	QString& posYColumnPath() const;
	POINTER_D_ACCESSOR_DECL(AbstractColumn, posZColumn, PosZColumn)
	QString& posZColumnPath() const;
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

	typedef DatapickerCurvePrivate Private;

protected:
	DatapickerCurve(const QString& name, DatapickerCurvePrivate* dd);
	DatapickerCurvePrivate* const d_ptr;

private slots:
	void updateDatasheet();

private:
	Q_DECLARE_PRIVATE(DatapickerCurve)
	void init();
	void initAction();
	Column* appendColumn(const QString&);

	Spreadsheet* m_datasheet;
	QAction* updateDatasheetAction;

signals:
	void curveErrorTypesChanged(const DatapickerCurve::Errors&);
	void posXColumnChanged(const AbstractColumn*);
	void posYColumnChanged(const AbstractColumn*);
	void posZColumnChanged(const AbstractColumn*);
	void plusDeltaXColumnChanged(const AbstractColumn*);
	void minusDeltaXColumnChanged(const AbstractColumn*);
	void plusDeltaYColumnChanged(const AbstractColumn*);
	void minusDeltaYColumnChanged(const AbstractColumn*);
	friend class DatapickerCurveSetCurveErrorTypesCmd;
	friend class DatapickerCurveSetPosXColumnCmd;
	friend class DatapickerCurveSetPosYColumnCmd;
	friend class DatapickerCurveSetPosZColumnCmd;
	friend class DatapickerCurveSetPlusDeltaXColumnCmd;
	friend class DatapickerCurveSetMinusDeltaXColumnCmd;
	friend class DatapickerCurveSetPlusDeltaYColumnCmd;
	friend class DatapickerCurveSetMinusDeltaYColumnCmd;

};
#endif // DATAPICKERCURVE_H
