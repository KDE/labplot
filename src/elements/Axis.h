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

public:
	Axis();


//TODO
//	QDomElement saveXML(QDomDocument doc, int id);
//	void openXML(QDomNode node);
	//void centerX(int plotsize, float center);	//!< center label on x axis
	//void centerY(int plotsize, float center);	//!< center label on y axis

	ACCESSFUNCFLAG(m_enabled, Axis);
	ACCESSFUNC(Axis::ScaleType, m_scaleType, scaleType, ScaleType);
	ACCESSFUNC(int, m_position, position, Postition);
	ACCESSFUNC(float, m_lowerLimit, lowerLimit, LowerLimit);
	ACCESSFUNC(float, m_upperLimit, upperLimit, UpperLimit);
	ACCESSFUNC(float, m_offset, offset, Offset);
	ACCESSFUNC(float, m_scaleFactor, scaleFactor, ScaleFactor);
	ACCESSFUNCFLAG(m_borderEnabled, Border);
	ACCESSFUNC(QColor, m_borderColor, borderColor, BorderColor);
	ACCESSFUNC(int, m_borderWidth, borderWidth, BorderWidth);

	Label* label(){ return &m_label; }
// 	ACCESSFUNC(Label, m_label, label);

	ACCESSFUNC(Axis::TicksPosition, m_ticksPosition, ticksPosition, TickPosition);
	ACCESSFUNC(short, m_ticksType, ticksType, TicksType);
	ACCESSFUNC(QColor, m_ticksColor, ticksColor, TickColor);
	ACCESSFUNC(bool, m_majorTicksEnabled, majorTicksEnabled, MajorTicksEnabled);
	ACCESSFUNC(int, m_majorTicksNumber, majorTicksNumber, MajorTicksNumber);
	ACCESSFUNC(short, m_majorTicksWidth, majorTicksWidth, MajorTicksWidth);
	ACCESSFUNC(short, m_majorTicksLength, majorTicksLength, MajorTicksLength);
	ACCESSFUNC(bool, m_minorTicksEnabled, minorTicksEnabled, MinorTicksEnabled);
	ACCESSFUNC(int, m_minorTicksNumber, minorTicksNumber, MinorTicksNumber);
	ACCESSFUNC(short, m_minorTicksWidth, minorTicksWidth, MinorTicksWidth);
	ACCESSFUNC(short, m_minorTicksLength, minorTicksLength, MinorTicksLength);

	ACCESSFUNC(bool, m_labelsEnabled, labelsEnabled, LabelsEnabled);
	ACCESSFUNC(int, m_labelsPosition, labelsPosition, LabelsPosition);
	ACCESSFUNC(float, m_labelsRotation, labelsRotation, LabelsRotation);
	ACCESSFUNC(int, m_labelsPrecision, labelsPrecision, LabelsPrecision);
	ACCESSFUNC(Axis::LabelsFormat, m_labelsFormat, labelsFormat, LabelsFormat);
	ACCESSFUNC(QString, m_labelsDateFormat, labelsDateFormat, LabelsDateFormat);
	ACCESSFUNC(QFont, m_labelsFont, labelsFont, LabelsFont);
	ACCESSFUNC(QColor, m_labelsColor, labelsColor, LabelsColor);
	ACCESSFUNC(QString, m_labelsPrefix, labelsPrefix, LabelsPrefix);
	ACCESSFUNC(QString, m_labelsSuffix, labelsSuffix, LabelsSuffix);

	ACCESSFUNCFLAG(m_majorGridEnabled, MajorGrid);
	ACCESSFUNC(Qt::PenStyle, m_majorGridStyle, majorGridStyle, MajorGridStyle);
	ACCESSFUNC(QColor, m_majorGridColor, majorGridColor, MajorGridColor);
	ACCESSFUNC(short, m_majorGridWidth, majorGridWidth, MajorGridWidth);
	ACCESSFUNCFLAG(m_minorGridEnabled, MinorGrid);
	ACCESSFUNC(Qt::PenStyle, m_majorGridStyle, minorGridStyle, MinorGridStyle);
	ACCESSFUNC(QColor, m_minorGridColor, minorGridColor, MinorGridColor);
	ACCESSFUNC(short, m_minorGridWidth, minorGridWidth, MinorGridWidth);

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
