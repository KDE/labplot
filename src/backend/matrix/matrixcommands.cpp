/***************************************************************************
    File                 : matrixcommands.cpp
    Project              : LabPlot
    Description          : Commands used in Matrix (part of the undo/redo framework)
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2015 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2017 Stefan Gerlach (stefan.gerlach@uni.kn)

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

#include "matrixcommands.h"
#include "MatrixPrivate.h"

//Insert columns
MatrixInsertColumnsCmd::MatrixInsertColumnsCmd(MatrixPrivate* private_obj, int before, int count, QUndoCommand* parent)
 : QUndoCommand(parent), m_private_obj(private_obj), m_before(before), m_count(count) {
	setText(i18np("%1: insert %2 column", "%1: insert %2 columns", m_private_obj->name(), m_count));
}

void MatrixInsertColumnsCmd::redo() {
	m_private_obj->insertColumns(m_before, m_count);
	emit m_private_obj->q->columnCountChanged(m_private_obj->columnCount);
}

void MatrixInsertColumnsCmd::undo() {
	m_private_obj->removeColumns(m_before, m_count);
	emit m_private_obj->q->columnCountChanged(m_private_obj->columnCount);
}

//Insert rows
MatrixInsertRowsCmd::MatrixInsertRowsCmd(MatrixPrivate* private_obj, int before, int count, QUndoCommand* parent)
 : QUndoCommand(parent), m_private_obj(private_obj), m_before(before), m_count(count) {
	setText(i18np("%1: insert %2 row", "%1: insert %2 rows", m_private_obj->name(), m_count));
}

void MatrixInsertRowsCmd::redo() {
	m_private_obj->insertRows(m_before, m_count);
	emit m_private_obj->q->rowCountChanged(m_private_obj->rowCount);
}

void MatrixInsertRowsCmd::undo() {
	m_private_obj->removeRows(m_before, m_count);
	emit m_private_obj->q->rowCountChanged(m_private_obj->rowCount);
}

//set coordinates
MatrixSetCoordinatesCmd::MatrixSetCoordinatesCmd(MatrixPrivate* private_obj, double x1, double x2, double y1, double y2, QUndoCommand* parent)
		: QUndoCommand( parent ), m_private_obj(private_obj), m_new_x1(x1), m_new_x2(x2), m_new_y1(y1), m_new_y2(y2) {
	setText(i18n("%1: set matrix coordinates", m_private_obj->name()));
}

void MatrixSetCoordinatesCmd::redo() {
	m_old_x1 = m_private_obj->xStart;
	m_old_x2 = m_private_obj->xEnd;
	m_old_y1 = m_private_obj->yStart;
	m_old_y2 = m_private_obj->yEnd;
	m_private_obj->xStart = m_new_x1;
	m_private_obj->xEnd = m_new_x2;
	m_private_obj->yStart = m_new_y1;
	m_private_obj->yEnd = m_new_y2;
}

void MatrixSetCoordinatesCmd::undo() {
	m_private_obj->xStart = m_old_x1;
	m_private_obj->xEnd = m_old_x2;
	m_private_obj->yStart = m_old_y1;
	m_private_obj->yEnd = m_old_y2;
}

//set formula
MatrixSetFormulaCmd::MatrixSetFormulaCmd(MatrixPrivate* private_obj, QString formula)
	: m_private_obj(private_obj), m_other_formula(std::move(formula)) {
	setText(i18n("%1: set formula", m_private_obj->name()));
}

void MatrixSetFormulaCmd::redo() {
	QString tmp = m_private_obj->formula;
	m_private_obj->formula = m_other_formula;
	m_other_formula = tmp;
}

void MatrixSetFormulaCmd::undo() {
	redo();
}

//replace values
MatrixReplaceValuesCmd::MatrixReplaceValuesCmd(MatrixPrivate* private_obj, void* new_values, QUndoCommand* parent)
 : QUndoCommand(parent), m_private_obj(private_obj), m_old_values(nullptr), m_new_values(new_values) {
	setText(i18n("%1: replace values", m_private_obj->name()));
}

void MatrixReplaceValuesCmd::redo() {
	m_old_values = m_private_obj->data;
	m_private_obj->data = m_new_values;
	m_private_obj->emitDataChanged(0, 0, m_private_obj->rowCount -1, m_private_obj->columnCount-1);
}

void MatrixReplaceValuesCmd::undo() {
	m_new_values = m_private_obj->data;
	m_private_obj->data = m_old_values;
	m_private_obj->emitDataChanged(0, 0, m_private_obj->rowCount -1, m_private_obj->columnCount-1);
}
