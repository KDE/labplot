/*
    File                 : DatapickerCurve.h
    Project              : LabPlot
    Description          : container for Curve-Point and Datasheet/Spreadsheet
    of datapicker
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Ankit Wagadre <wagadre.ankit@gmail.com>
    SPDX-FileCopyrightText: 2015-2021 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

	enum class ErrorType {NoError, SymmetricError, AsymmetricError};
	struct Errors {
		ErrorType x;
		ErrorType y;
	};

	QIcon icon() const override;
	void setPrinting(bool);
	void setSelectedInView(bool);
	void addDatasheet(DatapickerImage::GraphType);
	void updatePoints();
	void updatePoint(const DatapickerPoint*);

	Symbol* symbol() const;
	BASIC_D_ACCESSOR_DECL(Errors, curveErrorTypes, CurveErrorTypes)
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

private:
	Q_DECLARE_PRIVATE(DatapickerCurve)
	void init();
	void initAction();
	Column* appendColumn(const QString&);

	Spreadsheet* m_datasheet{nullptr};

Q_SIGNALS:
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
};
#endif // DATAPICKERCURVE_H
