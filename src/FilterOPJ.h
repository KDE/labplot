//LabPlot : FilterOPJ.h

#ifndef FILTEROPJ_H
#define FILTEROPJ_H

#include <qstring.h>
#include "MainWin.h"
#include "Symbol.h"

class FilterOPJ
{
public:
	FilterOPJ(MainWin *mw, QString filename);
	int import();
	void setSymbolType(Symbol *symbol,int type);
	Qt::PenStyle translateOriginLineStyle(int linestyle);
	QColor translateOriginColor(int color);
	QString parseOriginText(const QString &str);
	QString parseOriginTags(const QString &str);
private:
	MainWin *mw;
	QString filename;
};

#endif //FILTEROPJ_H
