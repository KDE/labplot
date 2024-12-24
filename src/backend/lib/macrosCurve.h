/*
	File                 : macros.h
	Project              : LabPlot
	Description          : Various preprocessor macros for curve classes
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MACROSCURVE_H
#define MACROSCURVE_H

#include "backend/core/AbstractColumn.h"
#include "backend/lib/commandtemplates.h"
#include <QObject>

/*!
  This macro is used to connect the column to the XYCurveQ_SLOTS:
	- <column_prefix>ColumnAboutToBeRemoved
	- <column_prefix>DataChanged
  This means these slots must be available when using this function.
  \param column pointer to a AbstractColumn
  \param prefix columnnames should have always the same style. For example xColumn -> prefix = x, xErrorPlusColumn -> prefix = xErrorPlus
  */
#define CURVE_COLUMN_CONNECT(class_name, Prefix, prefix, recalc_func)                                                                                          \
	void class_name::connect##Prefix##Column(const AbstractColumn* column) {                                                                                   \
		connect(column, &AbstractAspect::aspectAboutToBeRemoved, this, &class_name::prefix##ColumnAboutToBeRemoved);                                           \
		/* When the column is reused with different name, the curve should be informed to disconnect */                                                        \
		connect(column, &AbstractColumn::aboutToReset, this, &class_name::prefix##ColumnAboutToBeRemoved);                                                     \
		/* after the curve was updated, emit the signal to update the plot ranges */                                                                           \
		connect(column, &AbstractColumn::dataChanged, this, &class_name::recalc_func); /* must be before DataChanged*/                                         \
		connect(column, &AbstractColumn::dataChanged, this, &class_name::prefix##DataChanged); /* triggers a retransform in the plot and in its children */    \
	}

#define CURVE_COLUMN_CONNECT_CALL(curve, column, Prefix) curve->connect##Prefix##Column(column);

/*!
 * This macro is used to connect and disconnect the column from the curve
 * The new column is connected to the curve and the old column is diconnected
 * The columnPath is updated
 */
#define CURVE_COLUMN_SETTER_CMD_IMPL_F_S(class_name, Prefix, prefix, finalize_method)                                                                          \
	class class_name##Set##Prefix##ColumnCmd : public StandardSetterCmd<class_name::Private, const AbstractColumn*> {                                          \
	public:                                                                                                                                                    \
		class_name##Set##Prefix##ColumnCmd(class_name::Private* target, const AbstractColumn* newValue, const KLocalizedString& description)                   \
			: StandardSetterCmd<class_name::Private, const AbstractColumn*>(target, &class_name::Private::prefix##Column, newValue, description) {             \
		}                                                                                                                                                      \
		virtual void finalize() override {                                                                                                                     \
			m_target->finalize_method();                                                                                                                       \
		}                                                                                                                                                      \
		void redo() override {                                                                                                                                 \
			const auto columnOld = m_target->prefix##Column;                                                                                                   \
			if (columnOld) {                                                                                                                                   \
				/* disconnect only when column valid, because otherwise all                                                                                    \
				 * signals are disconnected */                                                                                                                 \
				QObject::disconnect(columnOld, nullptr, m_target->q, nullptr);                                                                                 \
			}                                                                                                                                                  \
			m_target->prefix##Column = m_otherValue;                                                                                                           \
			m_otherValue = columnOld;                                                                                                                          \
			if (m_target->prefix##Column) {                                                                                                                    \
				m_target->q->set##Prefix##ColumnPath(m_target->prefix##Column->pa                                                                              \
				CURVE_COLUMN_CONNECT_CALL(m_target->q, m_target->prefix##Column, Prefix)                                                                       \
			} else                                                                                                                                             \
				m_target->q->set##Prefix##ColumnPath(QStringLiteral(""));                                                                                      \
			finalize();                                                                                                                                        \
			/* emit DataChanged() in order to notify the plot about the changes */                                                                             \
			Q_EMIT m_target->q->prefix##ColumnChanged(m_target->*m_field);                                                                                     \
			Q_EMIT m_target->q->prefix##DataChanged();                                                                                                         \
		}                                                                                                                                                      \
		void undo() override {                                                                                                                                 \
			redo();                                                                                                                                            \
		}                                                                                                                                                      \
	};

#define CURVE_COLUMN_REMOVED(prefix)                                                                                                                           \
	Q_EMIT prefix##ColumnChanged(d->prefix##Column);                                                                                                           \
	/* emit DataChanged() in order to notify the plot about the changes */                                                                                     \
	Q_EMIT prefix##DataChanged();

#define CURVE_COLUMN_LIST_SETTER_CMD_IMPL_F_S(class_name, Prefix, prefix, finalize_method)                                                                     \
class class_name##Set##Prefix##ColumnsCmd : public StandardSetterCmd<class_name::Private, QVector<const AbstractColumn*>> {                                    \
		public:                                                                                                                                                \
		class_name##Set##Prefix##ColumnsCmd(class_name::Private* target, const QVector<const AbstractColumn*> newValue, const KLocalizedString& description)   \
		: StandardSetterCmd<class_name::Private, QVector<const AbstractColumn*>>(target, &class_name::Private::prefix##Columns, newValue, description)         \
	{	}                                                                                                                                                      \
		virtual void finalize() override {                                                                                                                     \
			m_target->finalize_method();                                                                                                                       \
		}                                                                                                                                                      \
		void redo() override {                                                                                                                                 \
			const auto columns_old = m_target->prefix##Columns;                                                                                                \
			for (auto col: columns_old) {                                                                                                                      \
				if (col) {                                                                                                                                     \
					/* disconnect only when column valid, because otherwise all                                                                                \
					 * signals are disconnected */                                                                                                             \
					QObject::disconnect(col, nullptr, m_target->q, nullptr);                                                                                   \
			}                                                                                                                                                  \
		}                                                                                                                                                      \
			m_target->prefix##Columns = m_otherValue;                                                                                                          \
			m_otherValue = columns_old;                                                                                                                        \
			m_target->prefix##ColumnPaths.clear();                                                                                                             \
			for (auto col: m_target->prefix##Columns) {                                                                                                        \
				if (col) {                                                                                                                                     \
					m_target->prefix##ColumnPaths.append(col->path());                                                                                         \
					CURVE_COLUMN_CONNECT_CALL(m_target->q, col, Prefix)                                                                                        \
			} else                                                                                                                                             \
				m_target->prefix##ColumnPaths.append(QStringLiteral(""));                                                                                      \
		}                                                                                                                                                      \
			finalize();                                                                                                                                        \
			Q_EMIT m_target->q->prefix##ColumnsChanged(m_target->*m_field);                                                                                    \
			/* emit DataChanged() in order to notify the plot about the changes */                                                                             \
		}                                                                                                                                                      \
		void undo() override {                                                                                                                                 \
			redo();                                                                                                                                            \
	}                                                                                                                                                          \
};

#endif // MACROSXYCURVE_H
