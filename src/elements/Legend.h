#ifndef LEGEND_H
#define LEGEND_H

#include "../definitions.h"

#include <QFont>
#include <QColor>
#include <QPoint>
// class QPainter;
// class Symbol;
// #include "GraphList.h"

class Legend {
public:
	Legend();

	ACCESSFUNC(QPoint, m_position, position, Position);

	ACCESSFUNCFLAG(m_fillingEnabled, Filling);
	ACCESSFUNC(QColor, m_fillingColor, fillingColor, FillingColor);
	ACCESSFUNCFLAG(m_boxEnabled, Box);
	ACCESSFUNCFLAG(m_shadowEnabled, Shadow);

	ACCESSFUNC(QFont, m_textFont, textFont, TextFont);
	ACCESSFUNC(QColor, m_textColor, textColor, TextColor);

	/*
	void save(QTextStream *t);
	void open(QTextStream *t, int version);
	QDomElement saveXML(QDomDocument doc);
	void openXML(QDomNode node);
	void setFont(QFont f) { font = f; }
	QFont Font() { return font; }
	void enable(bool b=true) { enabled = b; }
	bool Enabled() { return enabled; }
	void setPosition(double X, double Y) { x = X; y = Y; }
	double X() { return x; }
	double Y() { return y; }
	void enableBorder(bool b=true) { border = b; }
	bool BorderEnabled() { return border; }
	void setColor(QColor c) { color = c; }
	void setColor(QString c) { color = QColor(c); }
	QColor Color() { return color; }
	void setTransparent(bool t=true) { transparent=t; }
	bool Transparent() { return transparent; }
	bool getOrientation() { return orientation; }
	void setOrientation(bool o) { orientation=o; }
	int drawGraphs(QPainter *p, GraphList *gl, PType type, Point size, QFont tmpfont);
	void draw(QPainter *p, PType type, GraphList *graphlist, Point pos, Point size,int w, int h);
	bool inside(int X, int Y);
	int TicLabelLength() { return ticlabellength; }
	void setTicLabelLength(int l) { ticlabellength = l; }
	*/
private:
	bool m_enabled;
	QPoint m_position;

	bool m_fillingEnabled;
	QColor m_fillingColor;
	bool m_boxEnabled;
	bool m_shadowEnabled;

	QFont m_textFont;
	QColor m_textColor;

	/*
	double x,y;			// position 0..1
	int x1, y1, x2, y2;		// legend box
	QFont font;				// legend font
	bool border;			// border enabled
	bool enabled;				// legend enabled
	QColor color;			// legend background color
	bool transparent;		// transparent ?
	int namelength;		// legend width
	bool orientation;		// 0: BottomTop, 1:LeftRight
	int ticlabellength;		// max length of tic label. used for border
	*/
};

#endif //LEGEND_H
