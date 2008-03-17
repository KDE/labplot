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
	Qt::PenStyle translateOriginLineStyle(int linestyle) const;
	QColor translateOriginColor(int color) const;
	QString parseOriginText(const QString &str) const;
	QString parseOriginTags(const QString &str) const;
private:
	MainWin *mw;
	QString filename;
};

#endif //FILTEROPJ_H
