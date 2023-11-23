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
	- <column_prefix>ColumnNameChanged
	- <column_prefix>DataChanged
  This means these slots must be available when using this function.
  \param column pointer to a AbstractColumn
  \param prefix columnnames should have always the same style. For example xColumn -> prefix = x, xErrorPlusColumn -> prefix = xErrorPlus
  */
#define CURVE_COLUMN_CONNECT(class_name, Prefix, prefix, recalc_func)                                                                                          \
	void class_name::connect##Prefix##Column(const AbstractColumn* column) {                                                                                   \
		connect(column->parentAspect(), &AbstractAspect::childAspectAboutToBeRemoved, this, &class_name::prefix##ColumnAboutToBeRemoved);                      \
		/* When the column is reused with different name, the curve should be informed to disconnect */                                                        \
		connect(column, &AbstractColumn::reset, this, &class_name::prefix##ColumnAboutToBeRemoved);                                                            \
		connect(column, &AbstractAspect::aspectDescriptionChanged, this, &class_name::prefix##ColumnNameChanged);                                              \
		/* after the curve was updated, emit the signal to update the plot ranges */                                                                           \
		connect(column, &AbstractColumn::dataChanged, this, &class_name::recalc_func); /* must be before DataChanged*/                                         \
		connect(column, &AbstractColumn::dataChanged, this, &class_name::prefix##DataChanged);                                                                 \
	}

#define CURVE_COLUMN_CONNECT_CALL(curve, column, Prefix)                                                                                                       \
	do {                                                                                                                                                       \
		curve->connect##Prefix##Column(column);                                                                                                                \
	} while (false);

/*!
 * This macro is used to connect and disconnect the column from the curve
 * The new column is connected to the curve and the old column is diconnected
 * The columnPath is updated
 */
#define CURVE_COLUMN_SETTER_CMD_IMPL_F_S(class_name, Prefix, prefix, finalize_method)                                                                          \
	class class_name##Set##Prefix##ColumnCmd : public StandardSetterCmd<class_name::Private, const AbstractColumn*> {                                          \
	public:                                                                                                                                                    \
		class_name##Set##Prefix##ColumnCmd(class_name::Private* target, const AbstractColumn* newValue, const KLocalizedString& description)                   \
			: StandardSetterCmd<class_name::Private, const AbstractColumn*>(target, &class_name::Private::prefix##Column, newValue, description)               \
			, m_private(target)                                                                                                                                \
			, m_column(newValue) {                                                                                                                             \
		}                                                                                                                                                      \
		virtual void finalize() override {                                                                                                                     \
			m_target->finalize_method();                                                                                                                       \
			Q_EMIT m_target->q->prefix##ColumnChanged(m_target->*m_field);                                                                                     \
		}                                                                                                                                                      \
		void redo() override {                                                                                                                                 \
			m_columnOld = m_private->prefix##Column;                                                                                                           \
			if (m_columnOld) {                                                                                                                                 \
				/* disconnect only when column valid, because otherwise all                                                                                    \
				 * signals are disconnected */                                                                                                                 \
				QObject::disconnect(m_columnOld, nullptr, m_private->q, nullptr);                                                                              \
			}                                                                                                                                                  \
			m_private->prefix##Column = m_column;                                                                                                              \
			if (m_column) {                                                                                                                                    \
				m_private->q->set##Prefix##ColumnPath(m_column->path());                                                                                       \
				CURVE_COLUMN_CONNECT_CALL(m_private->q, m_column, Prefix)                                                                                      \
			} else                                                                                                                                             \
				m_private->q->set##Prefix##ColumnPath(QStringLiteral(""));                                                                                     \
			finalize();                                                                                                                                        \
			Q_EMIT m_private->q->prefix##ColumnChanged(m_column);                                                                                              \
			/* emit DataChanged() in order to notify the plot about the changes */                                                                             \
			Q_EMIT m_private->q->prefix##DataChanged();                                                                                                        \
		}                                                                                                                                                      \
		void undo() override {                                                                                                                                 \
			if (m_private->prefix##Column)                                                                                                                     \
				QObject::disconnect(m_private->prefix##Column, nullptr, m_private->q, nullptr);                                                                \
			m_private->prefix##Column = m_columnOld;                                                                                                           \
			if (m_columnOld) {                                                                                                                                 \
				m_private->q->set##Prefix##ColumnPath(m_columnOld->path());                                                                                    \
				CURVE_COLUMN_CONNECT_CALL(m_private->q, m_column, Prefix)                                                                                      \
			} else                                                                                                                                             \
				m_private->q->set##Prefix##ColumnPath(QStringLiteral(""));                                                                                     \
			finalize();                                                                                                                                        \
			Q_EMIT m_private->q->prefix##ColumnChanged(m_columnOld);                                                                                           \
			/* emit DataChanged() in order to notify the plot about the changes */                                                                             \
			Q_EMIT m_private->q->prefix##DataChanged();                                                                                                        \
		}                                                                                                                                                      \
                                                                                                                                                               \
	private:                                                                                                                                                   \
		class_name::Private* m_private;                                                                                                                        \
		const AbstractColumn* m_column{nullptr};                                                                                                               \
		const AbstractColumn* m_columnOld{nullptr};                                                                                                            \
	};

// No recalc function
#define HEATMAP_COLUMN_CONNECT(class_name, Prefix, prefix)                                                                                                     \
	void class_name::connect##Prefix##Column(const AbstractColumn* column) {                                                                                   \
		connect(column->parentAspect(), &AbstractAspect::childAspectAboutToBeRemoved, this, &class_name::prefix##ColumnAboutToBeRemoved);                      \
		/* When the column is reused with different name, the curve should be informed to disconnect */                                                        \
		connect(column, &AbstractColumn::reset, this, &class_name::prefix##ColumnAboutToBeRemoved);                                                            \
		connect(column, &AbstractAspect::aspectDescriptionChanged, this, &class_name::prefix##ColumnNameChanged);                                              \
		/* after the curve was updated, emit the signal to update the plot ranges */                                                                           \
		connect(column, &AbstractColumn::dataChanged, this, &class_name::prefix##DataChanged);                                                                 \
	}

#define HEATMAP_COLUMN_SETTER_CMD_IMPL_S(class_name, Prefix, prefix)                                                                                           \
	class class_name##Set##Prefix##ColumnCmd : public StandardSetterCmd<class_name::Private, const AbstractColumn*> {                                          \
	public:                                                                                                                                                    \
		class_name##Set##Prefix##ColumnCmd(class_name::Private* target, const AbstractColumn* newValue, const KLocalizedString& description)                   \
			: StandardSetterCmd<class_name::Private, const AbstractColumn*>(target, &class_name::Private::prefix##Column, newValue, description)               \
			, m_private(target)                                                                                                                                \
			, m_column(newValue) {                                                                                                                             \
		}                                                                                                                                                      \
		virtual void finalize() override {                                                                                                                     \
			emit m_target->q->prefix##ColumnChanged(m_target->*m_field);                                                                                       \
			emit m_private->q->prefix##ColumnChanged(m_private->prefix##Column);                                                                               \
			if (m_private->dataSource == class_name::DataSource::Spreadsheet) {                                                                                \
				/* emit DataChanged() in order to notify the plot about the changes */                                                                         \
				emit m_private->q->prefix##DataChanged();                                                                                                      \
			}                                                                                                                                                  \
		}                                                                                                                                                      \
		void redo() override {                                                                                                                                 \
			const AbstractColumn* columnOld = m_private->prefix##Column;                                                                                       \
			if (columnOld) {                                                                                                                                   \
				/* disconnect only when column valid, because otherwise all                                                                                    \
				 * signals are disconnected */                                                                                                                 \
				QObject::disconnect(columnOld, nullptr, m_private->q, nullptr);                                                                                \
			}                                                                                                                                                  \
			m_private->prefix##Column = m_column;                                                                                                              \
			if (m_column) {                                                                                                                                    \
				m_private->q->set##Prefix##ColumnPath(m_column->path());                                                                                       \
				if (m_private->dataSource == class_name::DataSource::Spreadsheet) {                                                                            \
					CURVE_COLUMN_CONNECT_CALL(m_private->q, m_column, Prefix)                                                                                  \
				}                                                                                                                                              \
			} else                                                                                                                                             \
				m_private->q->set##Prefix##ColumnPath(QStringLiteral(""));                                                                                     \
			m_column = columnOld;                                                                                                                              \
			finalize();                                                                                                                                        \
		}                                                                                                                                                      \
		void undo() override {                                                                                                                                 \
			redo();                                                                                                                                            \
		}                                                                                                                                                      \
                                                                                                                                                               \
	private:                                                                                                                                                   \
		class_name::Private* m_private;                                                                                                                        \
		const AbstractColumn* m_column{nullptr};                                                                                                               \
	};

#endif // MACROSXYCURVE_H
