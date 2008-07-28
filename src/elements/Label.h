#ifndef LABEL_H
#define LABEL_H

#include <QString>
#include <QFont>
#include <QColor>
#include <QPoint>

#include "Point.h"
#include "../definitions.h"

class Label {

public:
	enum LabelPosition {LEFT, CENTER, RIGHT, CUSTOM};

	Label( QString text=QString("") );
	void draw(QPainter *p, const Point pos, const Point size, const int w, const int h, const double phi) const;

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

	ACCESS(LabelPosition, positionType, PositionType);
	ACCESS(QPointF, position, Position);
	ACCESS(float, rotation, Rotation);

	ACCESSFLAG(m_fillingEnabled, Filling);
	ACCESS(QColor, fillingColor, FillingColor);
	ACCESSFLAG(m_boxEnabled, Box);
	ACCESSFLAG(m_shadowEnabled, Shadow);

	ACCESS(QFont, textFont, TextFont);
	ACCESS(QColor, textColor, TextColor);
	ACCESSFLAG(m_texEnabled, Tex);
	ACCESS(QString, text, Text);

private:
	LabelPosition m_positionType;
	QPointF m_position;
	float m_rotation;		// label rotation

	bool m_fillingEnabled;
	QColor m_fillingColor;
	bool m_boxEnabled;
	bool m_shadowEnabled;

	QFont m_textFont;			// label font
	QColor m_textColor;
	QString m_text;			// label string
	bool m_texEnabled;
};

#endif //LABEL_H
