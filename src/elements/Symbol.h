#ifndef SYMBOL_H
#define SYMBOL_H

#include <QPainter>
#include "../definitions.h"

/**
 * @brief Contains all the information needed to define/describe a symbol.
*	Provides the static function \c draw() for drawing a symbol.
*/
class Symbol {
public:

	enum SType {SNONE,SCROSS,SDOT,SPLUS,SCIRCLE,STRIANGLE,SUTRIANGLE,SRECT,SSTAR,SDIAMOND,SMINUS,SPIPE,
		SLTRIANGLE,SRTRIANGLE,STRIANGLE1,STRIANGLE2,STRIANGLE3,STRIANGLE4,SUCIRCLE,SDCIRCLE,SSTAR2,
		SVBAR,SHBAR,SDIAG1,SDIAG2,SCROSS2,SDIAG3,SDIAG4,SCROSS3,
		SPARRIGHT,SPARLEFT,SHLEFTCIRCLE,SHRIGHTCIRCLE,SSMALLDIAMOND,SROTDIAMOND,SPENTA,SPENTALEFT,
		SPENTABOTTOM,SPENTARIGHT,SHEXAGON,SVHEXAGON,SSTAR3,SUARROW,SLARROW,SDARROW,SRARROW,
		SUHOUSE,SLHOUSE,SDHOUSE,SRHOUSE};

	// symbol fill type
	enum FType {FNONE,FFULL,FBOTTOM,FTOP,FLEFT,FRIGHT,FURIGHT,FDLEFT,FULEFT,FDRIGHT};

	Symbol(SType t=SNONE, QColor c="blue", int s=5, FType f=FNONE, QColor fc="red", int b=1);
//	void save(QTextStream *t);
//	void open(QTextStream *t,int version);
//	QDomElement saveXML(QDomDocument doc);
//	void openXML(QDomNode node);
	void draw(QPainter *p, QPoint point);
	static void draw( QPainter *p, const QPoint point, const SType t=SNONE, const QColor color=Qt::blue,
					   const int size=5, const FType fillType=FNONE, const QColor fillColor=Qt::transparent, const Qt::BrushStyle fillBrushStyle=Qt::NoBrush);

	static int styleCount(){ return 6; } //the total number in SType is 50. At the moment only 5 types are implemented.
	static int fillingTypeCount(){ return 10; }

	ACCESS(SType, type, Type);
	ACCESS(QColor, color, Color);
	ACCESS(int, size, Size);
	ACCESS(FType, fillType, FillType);
	ACCESS(QColor, fillColor, FillColor);
	ACCESS(int, brush, Brush);
//	void setErrorbar(Errorbar *e) { errorbar=e; }
//	Errorbar *errorBar() { return errorbar; }
//	EType errorbarType() { return etype; }
//	void setErrorbarType(EType e) { etype=e; }
//	int errorbarSize() { return esize; }
//	void setErrorbarSize(int s) { esize=s; }
private:
	SType m_type;
	QColor m_color;
	int m_size;
	FType m_fillType;
	QColor m_fillColor;
	int m_brush;	// TODO: use QBrush ?
//	Errorbar *errorbar;
};

#endif //SYMBOL_H
