//LabPlot: Label.h

#ifndef LABEL_H
#define LABEL_H

#include <QString>
#include <QFont>
#include <QColor>
#include "Point.h"

class Label {
public:
	Label(QString t = QString(""), QFont f = QFont(QString("Adobe Times"),14),QColor c = QColor("black"));
	void draw(QPainter *p, Point pos, Point size, int w, int h, double phi);	
/*	QStringList Info();
	void save(QTextStream *ts);
	void open(QTextStream *ts, int version, bool skip=true);
	QDomElement saveXML(QDomDocument doc);
	void openXML(QDomNode node);
	void saveSettings(KConfig *config, QString entry);
	void readSettings(KConfig *config, QString entry);
	void draw(class Worksheet *ws, QPainter *p, Point pos,Point size, int w, int h, double phi);	
										// draw the label with given width and height
	bool inside(int x0, int y0, Point pos, Point size, int w, int h);
	bool insideY(int x0, int y0, Point pos, Point size, int w, int h);
	bool insideZ(int x0, int y0, Point pos, Point size, int w, int h);	// for 3d y-axes
*/
	void setPosition(double X, double Y) { x=X; y=Y; }
	double X() { return x;}
	double Y() { return y;}
	void setX(double X) { x=X; }
	void setY(double Y) { y=Y; }

	QString Text() { return text; }		
	QString simpleText();		// all html tags removed from title		
	void setText(QString t) { text = t; }
	void setColor(QColor c) { color = c; }
	void setColor(QString c) { color = QColor(c); }
	QColor Color() { return color; }
	void setBackgroundColor(QColor c) { bgcolor = c; }
	void setBackgroundColor(QString c) { bgcolor = QColor(c); }
	QColor BackgroundColor() { return bgcolor; }
	void setFont(QFont f) { font = f; }
	QFont Font() { return font; }
	void setBoxed(bool b=true) { boxed = b; }
	bool Boxed() { return boxed; }
	void setTransparent(bool t) { transparent = t; }
	bool Transparent() { return transparent; }
	double Rotation() { return rotation; }
	void setRotation(double r) { rotation = r; }
/*	bool isTeXLabel() { return is_texlabel; }
	void setTeXLabel(bool t) { is_texlabel=t; }
	int Length();						// calculate length of richtext
*/
private:
	double x, y;			// position 0..1
	QString text;			// label string
	QFont font;			// label font
	QColor color, bgcolor;		// label + background color
	bool boxed;			// surrounding box
	bool transparent;		// transparent ?
	double rotation;		// label rotation
/*	QSimpleRichText *richtext;
	bool is_texlabel;		// if it is a tex label
*/
};

#endif //LABEL_H
