//LabPlot: Axis.h

#ifndef AXIS_H
#define AXIS_H

#include <QFont>
#include <QColor>
#include "Label.h"
#include "scale.h"
#include "tickformat.h"
#include "defs.h" 

class Axis {
public:
	Axis();
//	QDomElement saveXML(QDomDocument doc, int id);
//	void openXML(QDomNode node);
	void centerX(int plotsize, double center);	//!< center label on x axis
	void centerY(int plotsize, double center);	//!< center label on y axis

// TODO	void setLabel(Label *l);
	Label *getLabel() { return label; }
	ACCESS(bool, enabled, Enabled);
	ACCESS(TScale, scale, Scale);
	ACCESS(double, scaling, Scaling);
	ACCESS(double, shift, Shift);
	ACCESS(int, position, Position);

	ACCESS(int, ticktype, TickType);
	ACCESSFLAG(ticklabel_enabled, TickLabel);
	ACCESS(double, ticklabelrotation, TickLabelRotation);
	ACCESS(QString, ticklabelprefix, TickLabelPrefix);
	ACCESS(QString, ticklabelsuffix, TickLabelSuffix);
	ACCESS(QFont, tickfont, TickLabelFont);
	ACCESS(QColor, tickcolor, TickColor);
	ACCESS(QColor, ticklabelcolor, TickLabelColor);
	ACCESS(TFormat, ticklabelformat, TickLabelFormat);
	ACCESS(QString, datetimeformat, DateTimeFormat);
	ACCESS(int, ticklabelprecision, TickLabelPrecision);
	ACCESS(int, gap, TickLabelPosition);

	ACCESS(double, majorticks, MajorTicks);
	ACCESS(int, minorticks, MinorTicks);
	ACCESSFLAG(majorticks_enabled, MajorTicks);
	ACCESSFLAG(minorticks_enabled, MinorTicks);
	ACCESS(int, tickposition, TickPosition);
	ACCESS(int, majortickwidth, majorTickWidth);
	ACCESS(int, minortickwidth, minorTickWidth);
	ACCESS(int, majorticklength, majorTickLength);
	ACCESS(int, minorticklength, minorTickLength);

	ACCESSFLAG(border_enabled, Border);
	ACCESS(QColor, bordercolor, BorderColor);
	ACCESS(int, borderwidth, BorderWidth);

	ACCESSFLAG(majorgrid_enabled, MajorGrid);
	ACCESSFLAG(minorgrid_enabled, MinorGrid);
	ACCESS(QColor, majorgridcolor, MajorGridColor);
	ACCESS(QColor, minorgridcolor, MinorGridColor);
	ACCESS(Qt::PenStyle, majorgridtype, MajorGridType);
	ACCESS(Qt::PenStyle, minorgridtype, MinorGridType);
	ACCESS(int, majorgridwidth, MajorGridWidth);
	ACCESS(int, minorgridwidth, MinorGridWidth);
private:
	bool enabled;
	Label *label;
	int position;				//!< position : 0: normal 1: center
	TScale scale;
	double scaling, shift;			//!< scaling and shift of axes tick values
	int ticktype;				//!< tick type. 0: NUMBER, 1: INCREMENT
	bool ticklabel_enabled;			//!< tick label enabled?
	double ticklabelrotation;		//!< tick label rotation
	QString ticklabelprefix, ticklabelsuffix;	//!< tick label prefix and suffix
	QFont tickfont;				//!< axis/tics label font
	QColor tickcolor, ticklabelcolor;
	TFormat ticklabelformat;
	QString datetimeformat;			//!< format for time tick label
	int ticklabelprecision;
	int tickposition;			//!< tick label position 0:out 1:in 2:in&out 3:none
	double majorticks;			//!< number of / increment for major ticks
       	int minorticks;				//!< number of minor ticks
	bool majorticks_enabled, minorticks_enabled;
	int gap;				//!< gap between tick label and axis
	bool border_enabled;
	QColor bordercolor;
	int borderwidth;
	bool majorgrid_enabled, minorgrid_enabled;
	QColor majorgridcolor, minorgridcolor;
	Qt::PenStyle majorgridtype;		//!< major grid style (solid,dashed,dotted,...)
	Qt::PenStyle minorgridtype;		//!< minor grid style (solid,dashed,dotted,...)
	int majortickwidth, minortickwidth;
	int majorgridwidth, minorgridwidth;
	double majorticklength, minorticklength;
};

#endif	//AXIS_H
