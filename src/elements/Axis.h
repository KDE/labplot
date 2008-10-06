/***************************************************************************
    File                 : Axis.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach, Alexander Semke
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de
    Description          : axis class
                           
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
#ifndef AXIS_H
#define AXIS_H

#include <QFont>
#include <QColor>
#include "Label.h"
#include "../definitions.h"

/**
 * @brief Contains all the information needed to define/describe an axis.
 */
class Axis {
public:

	//position of the axis
	enum Position{POSITION_NORMAL, POSITION_CENTER, POSITION_CUSTOM};

	//scale type of the axis
	enum ScaleType {SCALETYPE_LINEAR, SCALETYPE_LOG10, SCALETYPE_LOG2, SCALETYPE_LN,
							SCALETYPE_SQRT, SCALETYPE_SX2};
	//position of the axis ticks
	enum TicksPosition {TICKSPOSITION_IN, TICKSPOSITION_OUT, TICKSPOSITION_INANDOUT, TICKSPOSITION_NONE};

	enum TicksType {TICKSTYPE_NUMBER, TICKSTYPE_INCREMENT};
	enum TicksNumberType {TICKSNUMBERTYPE_AUTO, TICKSNUMBERTYPE_CUSTOM};

	// format of the axis tick labels
	enum LabelsFormat {LABELSFORMAT_AUTO, LABELSFORMAT_NORMAL, LABELSFORMAT_SCIENTIFIC,
							 LABELSFORMAT_POWER10, LABELSFORMAT_POWER2, LABELSFORMAT_POWERE,
							LABELSFORMAT_FSQRT, LABELSFORMAT_TIME, LABELSFORMAT_DATE,
							LABELSFORMAT_DATETIME, LABELSFORMAT_DEGREE};
	Axis();

//TODO
//	QDomElement save(QDomDocument doc, int id);
//	void open(QDomNode node);
	//void centerX(int plotsize, float center);	//!< center label on x axis
	//void centerY(int plotsize, float center);	//!< center label on y axis

	ACCESSFLAG(m_enabled, Enabled);
	ACCESS(Axis::ScaleType, scaleType, ScaleType);
	ACCESS(Axis::Position, position, Position);
	ACCESS(float, positionOffset, PositionOffset);
	ACCESS(float, lowerLimit, LowerLimit);
	ACCESS(float, upperLimit, UpperLimit);
	ACCESS(float, offset, Offset);
	ACCESS(float, scaleFactor, ScaleFactor);
	ACCESSFLAG(m_borderEnabled, Border);
	ACCESS(QColor, borderColor, BorderColor);
	ACCESS(int, borderWidth, BorderWidth);

	Label* label();

	ACCESS(Axis::TicksPosition, ticksPosition, TicksPosition);
	ACCESS(Axis::TicksType, ticksType, TicksType);
	ACCESS(QColor, ticksColor, TicksColor);
	ACCESSFLAG(m_majorTicksEnabled, MajorTicks);
	ACCESS(Axis::TicksNumberType, majorTicksNumberType, MajorTicksNumberType);
	ACCESS(int, majorTicksNumber, MajorTicksNumber);
	ACCESS(short, majorTicksWidth, MajorTicksWidth);
	ACCESS(short, majorTicksLength, MajorTicksLength);
	ACCESSFLAG(m_minorTicksEnabled, MinorTicks);
	ACCESS(Axis::TicksNumberType, minorTicksNumberType, MinorTicksNumberType);
	ACCESS(int, minorTicksNumber, MinorTicksNumber);
	ACCESS(short, minorTicksWidth, MinorTicksWidth);
	ACCESS(short, minorTicksLength, MinorTicksLength);

	ACCESSFLAG(m_labelsEnabled, Labels);
	ACCESS(int, labelsPosition, LabelsPosition);
	ACCESS(float, labelsRotation, LabelsRotation);
	ACCESS(int, labelsPrecision, LabelsPrecision);
	ACCESS(Axis::LabelsFormat, labelsFormat, LabelsFormat);
	ACCESS(QString, labelsDateFormat, LabelsDateFormat);
	ACCESS(QFont, labelsFont, LabelsFont);
	ACCESS(QColor, labelsColor, LabelsColor);
	ACCESS(QString, labelsPrefix, LabelsPrefix);
	ACCESS(QString, labelsSuffix, LabelsSuffix);

	ACCESSFLAG(m_majorGridEnabled, MajorGrid);
	ACCESS(Qt::PenStyle, majorGridStyle, MajorGridStyle);
	ACCESS(QColor, majorGridColor, MajorGridColor);
	ACCESS(short, majorGridWidth, MajorGridWidth);
	ACCESSFLAG(m_minorGridEnabled, MinorGrid);
	ACCESS(Qt::PenStyle, minorGridStyle, MinorGridStyle);
	ACCESS(QColor, minorGridColor, MinorGridColor);
	ACCESS(short, minorGridWidth, MinorGridWidth);

private:
	bool m_enabled;
	Axis::ScaleType m_scaleType;
	Axis::Position m_position;
	float m_positionOffset;
	float m_lowerLimit;
	float m_upperLimit;
	float m_offset;
	float m_scaleFactor;
	bool m_borderEnabled;
	QColor m_borderColor;
	short m_borderWidth;

	Label m_label;

	Axis::TicksPosition m_ticksPosition;
	Axis::TicksType m_ticksType;
	QColor m_ticksColor;
	bool m_majorTicksEnabled;
	Axis::TicksNumberType m_majorTicksNumberType;
	int m_majorTicksNumber;
	short m_majorTicksWidth;
	short m_majorTicksLength;
	bool m_minorTicksEnabled;
	Axis::TicksNumberType m_minorTicksNumberType;
	int m_minorTicksNumber;
	short m_minorTicksWidth;
	short m_minorTicksLength;

	bool m_labelsEnabled;
	int m_labelsPosition;
	float m_labelsRotation;
	int m_labelsPrecision;
	Axis::LabelsFormat m_labelsFormat;
	QString m_labelsDateFormat;
	QFont m_labelsFont;
	QColor m_labelsColor;
	QString m_labelsPrefix;
	QString m_labelsSuffix;

	bool m_majorGridEnabled;
	Qt::PenStyle m_majorGridStyle;
	QColor m_majorGridColor;
	short m_majorGridWidth;
	bool m_minorGridEnabled;
	Qt::PenStyle m_minorGridStyle;
	QColor m_minorGridColor;
	short m_minorGridWidth;
};

#endif	//AXIS_H
