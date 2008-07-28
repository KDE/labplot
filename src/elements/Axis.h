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
	//scale type of the axis
	enum ScaleType {SCALETYPE_LINEAR, SCALETYPE_LOG10, SCALETYPE_LOG2, SCALETYPE_LN,
							SCALETYPE_SQRT, SCALETYPE_SX2};
	//position of the axis ticks
	enum TicksPosition {TICKSPOSITION_IN, TICKSPOSITION_OUT, TICKSPOSITION_INANDOUT, TICKSPOSITION_NONE};
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

	ACCESSFLAG(m_enabled, AxisEnabled);
	ACCESS(Axis::ScaleType, scaleType, ScaleType);
	ACCESS(int, position, Position);
	ACCESS(float, lowerLimit, LowerLimit);
	ACCESS(float, upperLimit, UpperLimit);
	ACCESS(float, offset, Offset);
	ACCESS(float, scaleFactor, ScaleFactor);
	ACCESSFLAG(m_borderEnabled, Border);
	ACCESS(QColor, borderColor, BorderColor);
	ACCESS(int, borderWidth, BorderWidth);

	Label* label();

	ACCESS(Axis::TicksPosition, ticksPosition, TickPosition);
	ACCESS(short, ticksType, TicksType);
	ACCESS(QColor, ticksColor, TickColor);
	ACCESSFLAG(m_majorTicksEnabled, MajorTicks);
	ACCESS(int, majorTicksNumber, MajorTicksNumber);
	ACCESS(short, majorTicksWidth, MajorTicksWidth);
	ACCESS(short, majorTicksLength, MajorTicksLength);
	ACCESS(bool, minorTicksEnabled, MinorTicksEnabled);
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
	ScaleType m_scaleType;
	short m_position;				//!< position : 0: normal 1: center
	float m_lowerLimit;
	float m_upperLimit;
	float m_offset;
	float m_scaleFactor;
	bool m_borderEnabled;
	QColor m_borderColor;
	short m_borderWidth;

	Label m_label;

	TicksPosition m_ticksPosition;
	short m_ticksType;//tick type: 0 - NUMBER, 1- INCREMENT
	QColor m_ticksColor;
	bool m_majorTicksEnabled;
	int m_majorTicksNumber;
	short m_majorTicksWidth;
	short m_majorTicksLength;
	bool m_minorTicksEnabled;
	int m_minorTicksNumber;
	short m_minorTicksWidth;
	short m_minorTicksLength;

	bool m_labelsEnabled;
	int m_labelsPosition;
	float m_labelsRotation;
	int m_labelsPrecision;
	LabelsFormat m_labelsFormat;
	QString m_labelsDateFormat;
	QFont m_labelsFont;
	QColor m_labelsColor;
	QString m_labelsPrefix;
	QString m_labelsSuffix;

	bool m_majorGridEnabled;
	Qt::PenStyle m_majorGridStyle; //major grid style (solid,dashed,dotted,...)
	QColor m_majorGridColor;
	short m_majorGridWidth;
	bool m_minorGridEnabled;
	Qt::PenStyle m_minorGridStyle;
	QColor m_minorGridColor;
	short m_minorGridWidth;
};

#endif	//AXIS_H
