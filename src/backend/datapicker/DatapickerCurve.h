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
#include "backend/worksheet/plots/cartesian/Symbol.h"

class DatapickerPoint;
class QAction;
class DatapickerCurvePrivate;
class Column;
class Spreadsheet;
class AbstractColumn;

class DatapickerCurve: public AbstractAspect {
	Q_OBJECT

public:
	explicit DatapickerCurve(const QString&);
	~DatapickerCurve() override;

	enum ErrorType { NoError, SymmetricError, AsymmetricError };
	struct Errors {
		ErrorType x;
		ErrorType y;
	};

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	void setPrinting(bool);
	void setSelectedInView(bool);
	void addDatasheet(DatapickerImage::GraphType);
	void updateData(const DatapickerPoint*);

	BASIC_D_ACCESSOR_DECL(Errors, curveErrorTypes, CurveErrorTypes)
	BASIC_D_ACCESSOR_DECL(Symbol::Style, pointStyle, PointStyle)
	BASIC_D_ACCESSOR_DECL(qreal, pointOpacity, PointOpacity)
	BASIC_D_ACCESSOR_DECL(qreal, pointRotationAngle, PointRotationAngle)
	BASIC_D_ACCESSOR_DECL(qreal, pointSize, PointSize)
	CLASS_D_ACCESSOR_DECL(QBrush, pointBrush, PointBrush)
	CLASS_D_ACCESSOR_DECL(QPen, pointPen, PointPen)
	BASIC_D_ACCESSOR_DECL(qreal, pointErrorBarSize, PointErrorBarSize)
	CLASS_D_ACCESSOR_DECL(QBrush, pointErrorBarBrush, PointErrorBarBrush)
	CLASS_D_ACCESSOR_DECL(QPen, pointErrorBarPen, PointErrorBarPen)
	BASIC_D_ACCESSOR_DECL(bool, pointVisibility, PointVisibility)

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

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

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
	void pointStyleChanged(Symbol::Style);
	void pointSizeChanged(qreal);
	void pointRotationAngleChanged(qreal);
	void pointOpacityChanged(qreal);
	void pointBrushChanged(QBrush);
	void pointPenChanged(const QPen&);
	void pointErrorBarSizeChanged(qreal);
	void pointErrorBarBrushChanged(QBrush);
	void pointErrorBarPenChanged(const QPen&);
	void pointVisibilityChanged(bool);
	friend class DatapickerCurveSetCurveErrorTypesCmd;
	friend class DatapickerCurveSetPosXColumnCmd;
	friend class DatapickerCurveSetPosYColumnCmd;
	friend class DatapickerCurveSetPosZColumnCmd;
	friend class DatapickerCurveSetPlusDeltaXColumnCmd;
	friend class DatapickerCurveSetMinusDeltaXColumnCmd;
	friend class DatapickerCurveSetPlusDeltaYColumnCmd;
	friend class DatapickerCurveSetMinusDeltaYColumnCmd;
	friend class DatapickerCurveSetPointStyleCmd;
	friend class DatapickerCurveSetPointSizeCmd;
	friend class DatapickerCurveSetPointRotationAngleCmd;
	friend class DatapickerCurveSetPointOpacityCmd;
	friend class DatapickerCurveSetPointBrushCmd;
	friend class DatapickerCurveSetPointPenCmd;
	friend class DatapickerCurveSetPointErrorBarSizeCmd;
	friend class DatapickerCurveSetPointErrorBarPenCmd;
	friend class DatapickerCurveSetPointErrorBarBrushCmd;
	friend class DatapickerCurveSetPointVisibilityCmd;
};
#endif // DATAPICKERCURVE_H
