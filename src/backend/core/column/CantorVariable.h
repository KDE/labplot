/***************************************************************************
    File                 : CantorVariable.h
    Project              : LabPlot
    Description          : Aspect providing a Cantor Worksheets for Multiple backends
    --------------------------------------------------------------------
    Copyright            : (C) 2015 Garvit Khatri (garvitdelhi@gmail.com)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#ifndef CANTORVARIABLE_H
#define CANTORVARIABLE_H

#include "backend/core/column/Column.h"
#include <QIcon>

class CantorVariable : public Column {
    Q_OBJECT
    
    public:
	CantorVariable(QString);
	~CantorVariable();
	
    virtual QIcon icon() const;
    
	bool isReadOnly() const;
	AbstractColumn::ColumnMode columnMode() const;
	bool copy(const AbstractColumn * other);
	bool copy(const AbstractColumn * source, int source_start, int dest_start, int num_rows);
	int rowCount() const;
	AbstractColumn::PlotDesignation plotDesignation() const;
	void setPlotDesignation(AbstractColumn::PlotDesignation pd);
	int width() const;
	void setWidth(int value);
	void clear();
	AbstractSimpleFilter *outputFilter() const;
	ColumnStringIO *asStringColumn() const;

	QString formula(int row) const;
	QList< Interval<int> > formulaIntervals() const;
	void setFormula(Interval<int> i, QString formula);
	void setFormula(int row, QString formula);
	void clearFormulas();
	
	void* data() const;
	double valueAt(int row) const;
	void setValueAt(int row, double new_value);
	virtual void replaceValues(int first, const QVector<double>& new_values);
	void setChanged();
	void setSuppressDataChangedSignal(bool b);

	void save(QXmlStreamWriter * writer) const;
	bool load(XmlStreamReader * reader);

    signals:
	void widthAboutToChange(const Column*);
	void widthChanged(const Column*);
};

#endif // CANTORVARIABLE_H
