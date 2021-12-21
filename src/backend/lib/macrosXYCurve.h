/*
    File                 : macros.h
    Project              : LabPlot
    Description          : Various preprocessor macros for the curve class
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Martin Marmsoler <martin.marmsoler@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef MACROSXYCURVE_H
#define MACROSXYCURVE_H

#include "backend/lib/commandtemplates.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/core/AbstractColumn.h"
#include <QObject>


/*!
  This macro is used to connect the column to the XYCurveQ_SLOTS:
	- <column_prefix>ColumnAboutToBeRemoved
	- <column_prefix>ColumnNameChanged
	- <column_prefix>DataChanged
  This means these slots must be available when using this function.
  \param column pointer to a AbstractColumn
  \param column_prefix columnnames should have always the same style. For example xColumn -> column_prefix = x, xErrorPlusColumn -> column_prefix = xErrorPlus
  */
#define XYCURVE_COLUMN_CONNECT(column_prefix) \
void XYCurve::connect ## column_prefix ## Column(const AbstractColumn* column) { \
	connect(column->parentAspect(), &AbstractAspect::aspectAboutToBeRemoved, this, &XYCurve::column_prefix ## ColumnAboutToBeRemoved); \
	/* When the column is reused with different name, the curve should be informed to disconnect */ \
	connect(column, &AbstractColumn::reset, this, &XYCurve::column_prefix ## ColumnAboutToBeRemoved); \
	connect(column, &AbstractAspect::aspectDescriptionChanged, this, &XYCurve::column_prefix ## ColumnNameChanged); \
	/* after the curve was updated, emit the signal to update the plot ranges */ \
	connect(column, &AbstractColumn::dataChanged, this, &XYCurve::recalcLogicalPoints); /* must be before DataChanged*/ \
	connect(column, &AbstractColumn::dataChanged, this, &XYCurve::column_prefix ## DataChanged);\
}

#define XYCURVE_COLUMN_CONNECT_CALL(curve, column, column_prefix) \
	curve->connect ## column_prefix ## Column(column); \

/*!
 * This macro is used to connect and disconnect the column from the curve
 * The new column is connected to the curve and the old column is diconnected
 * The columnPath is updated
*/
#define XYCURVE_COLUMN_SETTER_CMD_IMPL_F_S(cmd_name, prefix, finalize_method) \
class XYCurve ## Set ## cmd_name ## ColumnCmd: public StandardSetterCmd<XYCurve::Private, const AbstractColumn*> { \
public: \
	XYCurve ## Set ## cmd_name ## ColumnCmd(XYCurve::Private *target, const AbstractColumn* newValue, const KLocalizedString &description) \
		: StandardSetterCmd<XYCurve::Private, const AbstractColumn*>(target, &XYCurve::Private::prefix ## Column, newValue, description), \
		m_private(target), \
		m_column(newValue) \
		  {} \
		virtual void finalize() override { m_target->finalize_method(); emit m_target->q->prefix ## ColumnChanged(m_target->*m_field); } \
		void redo() override { \
			m_columnOld = m_private->prefix ## Column; \
			if (m_columnOld) {\
				/* disconnect only when column valid, because otherwise all
				 * signals are disconnected */ \
				QObject::disconnect(m_columnOld, nullptr, m_private->q, nullptr); \
			}\
			m_private->prefix ## Column = m_column; \
			if (m_column) { \
				m_private->q->set ## cmd_name ## ColumnPath(m_column->path());\
				XYCURVE_COLUMN_CONNECT_CALL(m_private->q, m_column, prefix) \
			} else \
				m_private->q->set ## cmd_name ## ColumnPath("");\
			finalize();\
			emit m_private->q->prefix ## ColumnChanged(m_column);\
			/* emit DataChanged() in order to notify the plot about the changes */ \
			emit m_private->q->prefix ## DataChanged();\
		}\
		void undo() override { \
			if (m_private->prefix ## Column)\
				QObject::disconnect(m_private->prefix ## Column, nullptr, m_private->q, nullptr); \
			m_private->prefix ## Column = m_columnOld;\
			if (m_columnOld) {\
				m_private->q->set ## cmd_name ## ColumnPath(m_columnOld->path());\
				XYCURVE_COLUMN_CONNECT_CALL(m_private->q, m_column, prefix)\
			} else\
				m_private->q->set ## cmd_name ## ColumnPath("");\
			finalize();\
			emit m_private->q->prefix ## ColumnChanged(m_columnOld);\
			/* emit DataChanged() in order to notify the plot about the changes */ \
			emit m_private->q->prefix ## DataChanged();\
		} \
private: \
	XYCurvePrivate* m_private; \
	const AbstractColumn* m_column{nullptr}; \
	const AbstractColumn* m_columnOld{nullptr};	\
};

#endif // MACROSXYCURVE_H
