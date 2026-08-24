/***************************************************************************
	File                 : Axis3D.h
	Project              : LabPlot
	Description          : 3D Axis
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2024 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#ifndef AXIS3D_H
#define AXIS3D_H

#include <backend/core/AbstractAspect.h>
#include "backend/lib/macros.h"

class QValue3DAxis;
class Axis3DPrivate;
class AbstractAspect;

class Axis3D : public AbstractAspect {
	Q_OBJECT
public:
	enum Format { Format_Decimal, Format_Scientific, Format_PowerOf10, Format_PowerOf2, Format_PowerOfE, Format_MultiplierOfPi };
	enum Type { X, Y, Z };
	explicit Axis3D(QString name, Axis3D::Type type);

	void setRange(float min, float max);
	BASIC_D_ACCESSOR_DECL(float, minRange, MinRange)
	BASIC_D_ACCESSOR_DECL(float, maxRange, MaxRange)
	BASIC_D_ACCESSOR_DECL(QString, title, Title)
	BASIC_D_ACCESSOR_DECL(qsizetype, segmentCount, SegmentCount)
	BASIC_D_ACCESSOR_DECL(qsizetype, subSegmentCount, SubSegmentCount)
	BASIC_D_ACCESSOR_DECL(Axis3D::Format, axisFormat, AxisFormat)
	BASIC_D_ACCESSOR_DECL(Axis3D::Type, type, Type)

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	QValue3DAxis* m_axis;
	typedef Axis3DPrivate Private;

protected:
	Axis3DPrivate* const d_ptr;
public Q_SLOTS:

private:
	Q_DECLARE_PRIVATE(Axis3D)
Q_SIGNALS:
	void typeChanged(Axis3D::Type);
	void titleChanged(QString);
	void maxRangeChanged(double);
	void minRangeChanged(double);
	void segmentCountChanged(qsizetype);
	void subSegmentCountChanged(qsizetype);
	void axisFormatChanged(Axis3D::Format);
};

#endif // AXIS3D_H
