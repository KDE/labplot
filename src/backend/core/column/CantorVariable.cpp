/***************************************************************************
    File                 : CantorVariable.cpp
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

#include "CantorVariable.h"

CantorVariable::CantorVariable(QString name) : Column(name, AbstractColumn::Numeric) {

}

/**
 * \brief Return an icon to be used for decorating the views and spreadsheet column headers
 */
QIcon CantorVariable::icon() const {
    return QIcon::fromTheme("system-rum");
}

bool CantorVariable::isReadOnly() const {
    return true;
}

AbstractColumn::ColumnMode CantorVariable::columnMode() const {
    return Column::columnMode();
}

bool CantorVariable::copy(const AbstractColumn* other) {
    return Column::copy(other);
}

bool CantorVariable::copy(const AbstractColumn* source, int source_start, int dest_start, int num_rows) {
    return Column::copy(source, source_start, dest_start, num_rows);
}

int CantorVariable::rowCount() const {
    return Column::rowCount();
}

AbstractColumn::PlotDesignation CantorVariable::plotDesignation() const {
    return Column::plotDesignation();
}

void CantorVariable::setPlotDesignation(AbstractColumn::PlotDesignation pd)
{
    Column::setPlotDesignation(pd);
}

void CantorVariable::clear()
{
    Column::clear();
}

void CantorVariable::setWidth(int value)
{
    Column::setWidth(value);
}

int CantorVariable::width() const
{
    return Column::width();
}

ColumnStringIO* CantorVariable::asStringColumn() const
{
    return Column::asStringColumn();
}

void CantorVariable::clearFormulas()
{
    Column::clearFormulas();
}

QString CantorVariable::formula(int row) const
{
    return Column::formula(row);
}

QList< Interval< int > > CantorVariable::formulaIntervals() const
{
    return Column::formulaIntervals();
}

AbstractSimpleFilter* CantorVariable::outputFilter() const
{
    return Column::outputFilter();
}

void CantorVariable::setFormula(int row, QString formula)
{
    Column::setFormula(row, formula);
}

void CantorVariable::setFormula(Interval< int > i, QString formula)
{
    Column::setFormula(i, formula);
}

void* CantorVariable::data() const
{
    return Column::data();
}

void CantorVariable::setValueAt(int row, double new_value)
{
    Column::setValueAt(row, new_value);
}

double CantorVariable::valueAt(int row) const
{
    return Column::valueAt(row);
}

void CantorVariable::replaceValues(int first, const QVector< double >& new_values)
{
    Column::replaceValues(first, new_values);
}

bool CantorVariable::load(XmlStreamReader* reader)
{
    return Column::load(reader);
}

void CantorVariable::save(QXmlStreamWriter* writer) const
{
    Column::save(writer);
}

void CantorVariable::setChanged()
{
    Column::setChanged();
}

void CantorVariable::setSuppressDataChangedSignal(bool b)
{
    Column::setSuppressDataChangedSignal(b);
}

CantorVariable::~CantorVariable() {

}
