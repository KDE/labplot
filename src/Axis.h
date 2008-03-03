//LabPlot: Axis.h

#ifndef AXIS_H
#define AXIS_H

#include <QFont>
#include <QColor>
#include "scale.h"
#include "tickformat.h"
#include "Label.h"

class Axis {
public:
	Axis();
/*	~Axis();
	QDomElement saveXML(QDomDocument doc, int id);
	void openXML(QDomNode node);
*/
	void Enable(bool b=true) { enabled = b; }
	bool Enabled() { return enabled; }
	void setScale(TScale s) { scale=s; }
	TScale Scale() { return scale; }
//	void setLabel(Label *l);
	Label *getLabel() { return label; }
	void setScaling(double s) { scaling=s; }
	double Scaling() { return scaling; }
	void setShift(double s) { shift=s; }
	double Shift() { return shift; }
	void setPosition(int p) { position = p; }
	int Position() { return position; }
	void centerX(int plotsize, double center);	//!< center label on x axis
	void centerY(int plotsize, double center);	//!< center label on y axis

	bool tickType() { return ticktype; }
	void setTickType(bool t) { ticktype = t; } 
	bool tickLabelEnabled() { return ticklabel_enabled; }
	void enableTickLabel(bool e=true) { ticklabel_enabled = e; } 
	void setTickLabelRotation(double r) { ticklabelrotation = r; }
	double TickLabelRotation() { return ticklabelrotation; }
	void setTickLabelPrefix(QString p) { ticklabelprefix=p; }
	QString TickLabelPrefix() { return ticklabelprefix; }
	void setTickLabelSuffix(QString s) { ticklabelsuffix=s; }
	QString TickLabelSuffix() { return ticklabelsuffix; }
	void setTickLabelFont(QFont a) { tickfont = a; }
	QFont TickLabelFont() { return tickfont; }
	void setTickColor(QColor col) { tickcolor = col; }
	QColor TickColor() { return tickcolor; }
	void setTickLabelColor(QColor col) { ticklabelcolor = col; }
	QColor TickLabelColor() { return ticklabelcolor; }
	void setTickLabelFormat(TFormat t) { ticklabelformat = t; }
	TFormat TickLabelFormat() { return ticklabelformat; }
	void setDateTimeFormat(QString f) { datetimeformat = f; }
	QString DateTimeFormat() { return datetimeformat; }
	void setTickLabelPrecision(int p) { ticklabelprecision = p; }
	int TickLabelPrecision() { return ticklabelprecision; }
	void setTickLabelPosition(int g) { gap=g; }
	int TickLabelPosition() { return gap; }

	void setMajorTicks(double t) { majorticks = t; }	//!< set number of /increment for major ticks
	double MajorTicks() { return majorticks; }		//!< get number/increment of major ticks
	void setMinorTicks(int t) { minorticks = t; }	//!< set number of minor ticks
	int MinorTicks() { return minorticks; }		//!< get number of minor ticks
	void enableMajorTicks(bool b=true) { majorticks_enabled = b; }
	void enableMinorTicks(bool b=true) { minorticks_enabled = b; }
	bool MajorTicksEnabled() { return majorticks_enabled; }
	bool MinorTicksEnabled() { return minorticks_enabled; }
	void setTickPos(int tp) { tickpos = tp; }
	int TickPos() { return tickpos; }
	void setMajorTickWidth(int w) { majortickwidth = w; }
	int majorTickWidth() { return majortickwidth; }
	void setMinorTickWidth(int w) { minortickwidth = w; }
	int minorTickWidth() { return minortickwidth; }
	double majorTickLength() { return majorticklength; }
	void setMajorTickLength(double v) {majorticklength=v; }
	double minorTickLength() { return minorticklength; }
	void setMinorTickLength(double v) {minorticklength=v; }

	bool BorderEnabled() { return border_enabled; }
	void enableBorder(bool b=true) { border_enabled=b; }
	void setBorderColor(QColor c) { bordercolor = c; }
	QColor BorderColor() { return bordercolor; }
	void setBorderWidth(int b) { borderwidth = b; }
	int borderWidth() { return borderwidth; }

	bool MajorGridEnabled() { return majorgrid_enabled; }
	void enableMajorGrid(bool b=true) { majorgrid_enabled=b; }
	bool MinorGridEnabled() { return minorgrid_enabled; }
	void enableMinorGrid(bool b=true) { minorgrid_enabled=b; }
	void setMajorGridColor(QColor c) { majorgridcolor = c; }
	QColor majorGridColor() { return majorgridcolor; }
	void setMinorGridColor(QColor c) { minorgridcolor = c; }
	QColor minorGridColor() { return minorgridcolor; }
	void setMajorGridType(Qt::PenStyle t) { majorgridtype = t; }
	Qt::PenStyle MajorGridType() { return majorgridtype; }
	void setMinorGridType(Qt::PenStyle t) { minorgridtype = t; }
	Qt::PenStyle MinorGridType() { return minorgridtype; }
	void setMajorGridWidth(int w) { majorgridwidth = w; }
	int majorGridWidth() { return majorgridwidth; }
	void setMinorGridWidth(int w) { minorgridwidth = w; }
	int minorGridWidth() { return minorgridwidth; }

private:
	bool enabled;				//!< axis enabled ?
	Label *label;
	int position;				//!< position : 0: normal 1: center
	TScale scale;
	double scaling, shift;	//!< scaling and shift of axes tic values
	int ticktype;			//!< tick type. 0: NUMBER, 1: INCREMENT
	bool ticklabel_enabled;		//!< tick label enabled?
	double ticklabelrotation;	//!< tick label rotation
	QString ticklabelprefix, ticklabelsuffix;	//!< tick label prefix and suffix
	QFont tickfont;			//!< axis/tics label font
	QColor tickcolor, ticklabelcolor;
	TFormat ticklabelformat;	//!< tick label format
	QString datetimeformat;		//!< format for time tick label
	int ticklabelprecision;		//!< tick label precision
	int tickpos;			//!< tick label position 0:out 1:in 2:in&out 3:none
	double majorticks;		//!< number of / increment for major ticks
       	int minorticks;			//!< number of minor ticks
	bool majorticks_enabled;		//!< major ticks enabled ?
	bool minorticks_enabled;		//!< minor ticks enabled ?
	int gap;				//!< gap between tick label and axis
	bool border_enabled;			//!< border enabled
	QColor bordercolor;			//!< border color
	int borderwidth;
	bool majorgrid_enabled, minorgrid_enabled;	//!< border enabled
	QColor majorgridcolor, minorgridcolor;	//<! major/minor grid color
	Qt::PenStyle majorgridtype;		//!< major grid style (solid,dashed,dotted,...)
	Qt::PenStyle minorgridtype;		//!< minor grid style (solid,dashed,dotted,...)
	int majortickwidth, minortickwidth;
	int majorgridwidth, minorgridwidth;
	double majorticklength, minorticklength;
};

#endif	//AXIS_H
