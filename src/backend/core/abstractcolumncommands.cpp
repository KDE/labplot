/*
	File                 : abstractcolumncommands.cpp
	Project              : LabPlot
	Description          : Commands to be called by AbstractColumn to modify AbstractColumnPrivate
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2007-2009 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2010 Knut Franke <knut.franke@gmx.de>
	SPDX-FileCopyrightText: 2014-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "abstractcolumncommands.h"
#include <KLocalizedString>

/** ***************************************************************************
 * \class AbstractColumnClearMasksCmd
 * \brief Clear masking information
 ** ***************************************************************************/

/**
 * \var AbstractColumnClearMasksCmd::m_col
 * \brief The private column data to modify
 */

/**
 * \var AbstractColumnClearMasksCmd::m_masking
 * \brief The old masks
 */

/**
 * \var AbstractColumnClearMasksCmd::m_copied
 * \brief A status flag
 */

/**
 * \brief Ctor
 */
AbstractColumnClearMasksCmd::AbstractColumnClearMasksCmd(AbstractColumnPrivate* col)
	: m_col(col) {
	setText(i18n("%1: clear masks", col->name()));
	m_copied = false;
}

/**
 * \brief Dtor
 */
AbstractColumnClearMasksCmd::~AbstractColumnClearMasksCmd() = default;

/**
 * \brief Execute the command
 */
void AbstractColumnClearMasksCmd::redo() {
	if (!m_copied) {
		m_masking = m_col->m_masking;
		m_copied = true;
	}
	m_col->m_masking.clear();
	finalize();
}

/**
 * \brief Undo the command
 */
void AbstractColumnClearMasksCmd::undo() {
	m_col->m_masking = m_masking;
	finalize();
}

void AbstractColumnClearMasksCmd::finalize() const {
	// TODO: implement AbstractColumn::setDataChanged() instead of these two calls,
	// move the already available Column::setDataChanged to the base class.
	Q_EMIT m_col->owner()->dataChanged(m_col->owner());
	m_col->owner()->invalidateProperties();
}

/** ***************************************************************************
 * \class AbstractColumnSetMaskedCmd
 * \brief Mark an interval of rows as masked
 ** ***************************************************************************/

/**
 * \var AbstractColumnSetMaskedCmd::m_col
 * \brief The private AbstractColumn data to modify
 */

/**
 * \var AbstractColumnSetMaskedCmd::m_interval
 * \brief The interval
 */

/**
 * \var AbstractColumnSetMaskedCmd::m_masked
 * \brief Mask/unmask flag
 */

/**
 * \var AbstractColumnSetMaskedCmd::m_masking
 * \brief Interval attribute backup
 */

/**
 * \var AbstractColumnSetMaskedCmd::m_copied
 * \brief A status flag
 */

/**
 * \brief Ctor
 */
AbstractColumnSetMaskedCmd::AbstractColumnSetMaskedCmd(AbstractColumnPrivate* col, int startRow, int endRow, bool masked)
	: m_col(col)
	, m_startRow(startRow)
	, m_endRow(endRow)
	, m_masked(masked) {
	if (masked)
		setText(i18n("%1: mask cells", col->name()));
	else
		setText(i18n("%1: unmask cells", col->name()));
	m_copied = false;
}

/**
 * \brief Dtor
 */
AbstractColumnSetMaskedCmd::~AbstractColumnSetMaskedCmd() = default;

/**
 * \brief Execute the command
 */
void AbstractColumnSetMaskedCmd::redo() {
	if (!m_copied) {
		m_masking = m_col->m_masking;
		m_copied = true;
	}

	// Set or clear bits for the range
	if (m_masked) {
		// Ensure bitmap is large enough
		if (m_endRow >= m_col->m_masking.size())
			m_col->m_masking.resize(m_endRow + 1);

		// Set bits in range
		for (int row = m_startRow; row <= m_endRow; ++row)
			m_col->m_masking.setBit(row, true);
	} else {
		// Clear bits in range (only if within bounds)
		if (m_startRow < m_col->m_masking.size()) {
			int actualEnd = qMin(m_endRow, m_col->m_masking.size() - 1);
			for (int row = m_startRow; row <= actualEnd; ++row)
				m_col->m_masking.setBit(row, false);
		}
	}

	m_col->owner()->setDataChanged();
}

/**
 * \brief Undo the command
 */
void AbstractColumnSetMaskedCmd::undo() {
	m_col->m_masking = m_masking;
	m_col->owner()->setDataChanged();
}

/** ***************************************************************************
 * \class AbstractColumnInsertRowsCmd
 * \brief Insert empty rows into a column
 ** ***************************************************************************/

/**
 * \var AbstractColumnInsertRowsCmd::m_col
 * \brief Private object of AbstractColumn to be modified.
 */

/**
 * \var AbstractColumnInsertRowsCmd::m_before
 * \brief Row number before which to insert the new rows.
 */

/**
 * \var AbstractColumnInsertRowsCmd::m_count
 * \brief Number of rows to be inserted.
 */

/**
 * \brief Ctor
 */
AbstractColumnInsertRowsCmd::AbstractColumnInsertRowsCmd(AbstractColumn* col, int before, int count)
	: m_col(col->d)
	, m_before(before)
	, m_count(count) {
}

/**
 * \brief Dtor
 */
AbstractColumnInsertRowsCmd::~AbstractColumnInsertRowsCmd() = default;

void AbstractColumnInsertRowsCmd::redo() {
	// Insert rows into bitmap - shift bits after insertion point
	if (m_before < m_col->m_masking.size()) {
		QBitArray newMasking(m_col->m_masking.size() + m_count);
		// Copy bits before insertion point
		for (int i = 0; i < m_before; ++i)
			newMasking.setBit(i, m_col->m_masking.testBit(i));
		// New bits [m_before, m_before+m_count) are false by default
		// Copy bits after insertion point (shifted by m_count)
		for (int i = m_before; i < m_col->m_masking.size(); ++i)
			newMasking.setBit(i + m_count, m_col->m_masking.testBit(i));
		m_col->m_masking = newMasking;
	}
	// else: inserting at or beyond current size, nothing to do
	m_col->owner()->setChanged();
}

void AbstractColumnInsertRowsCmd::undo() {
	// Remove the inserted rows - shift bits back
	if (m_before < m_col->m_masking.size()) {
		QBitArray newMasking(qMax(0, m_col->m_masking.size() - m_count));
		// Copy bits before removal point
		for (int i = 0; i < m_before && i < newMasking.size(); ++i)
			newMasking.setBit(i, m_col->m_masking.testBit(i));
		// Copy bits after removal point (shifted back by m_count)
		for (int i = m_before + m_count; i < m_col->m_masking.size() && i - m_count < newMasking.size(); ++i)
			newMasking.setBit(i - m_count, m_col->m_masking.testBit(i));
		m_col->m_masking = newMasking;
	}
	m_col->owner()->setChanged();
}

/** ***************************************************************************
 * \class AbstractColumnRemoveRowsCmd
 * \brief Remove rows from a column
 *
 * See AbstractColumnInsertRowsCmd for a discussion of the design.
 ** ***************************************************************************/

/**
 * \var AbstractColumnRemoveRowsCmd::m_col
 * \brief Private object of AbstractColumn to be modified.
 */

/**
 * \var AbstractColumnRemoveRowsCmd::m_first
 * \brief First row number to be removed.
 */

/**
 * \var AbstractColumnRemoveRowsCmd::m_count
 * \brief Number of rows to be removed.
 */

/**
 * \brief Ctor
 */
AbstractColumnRemoveRowsCmd::AbstractColumnRemoveRowsCmd(AbstractColumn* col, int first, int count)
	: m_col(col->d)
	, m_first(first)
	, m_count(count) {
}

/**
 * \brief Dtor
 */
AbstractColumnRemoveRowsCmd::~AbstractColumnRemoveRowsCmd() = default;

void AbstractColumnRemoveRowsCmd::redo() {
	m_masking = m_col->m_masking;

	// Remove rows from bitmap - clear bits and shift remaining bits
	if (m_first < m_col->m_masking.size()) {
		int removeEnd = qMin(m_first + m_count, m_col->m_masking.size());
		int newSize = qMax(0, m_col->m_masking.size() - m_count);
		QBitArray newMasking(newSize);

		// Copy bits before removal point
		for (int i = 0; i < m_first && i < newSize; ++i)
			newMasking.setBit(i, m_col->m_masking.testBit(i));

		// Copy bits after removal point (shifted back by m_count)
		for (int i = removeEnd; i < m_col->m_masking.size() && i - m_count < newSize; ++i)
			newMasking.setBit(i - m_count, m_col->m_masking.testBit(i));

		m_col->m_masking = newMasking;
	}

	m_col->owner()->setChanged();
}

void AbstractColumnRemoveRowsCmd::undo() {
	m_col->m_masking = m_masking;
	m_col->owner()->setChanged();
}

/** ***************************************************************************
 * \class AbstractColumnSetHeatmapFormatCmd
 * \brief Set the heatmap format
 ** ***************************************************************************/
AbstractColumnSetHeatmapFormatCmd::AbstractColumnSetHeatmapFormatCmd(AbstractColumnPrivate* col, const AbstractColumn::HeatmapFormat& format)
	: m_col(col)
	, m_format(format) {
	setText(i18n("%1: set heatmap format", col->name()));
}

AbstractColumnSetHeatmapFormatCmd::~AbstractColumnSetHeatmapFormatCmd() = default;

void AbstractColumnSetHeatmapFormatCmd::redo() {
	if (!m_col->m_heatmapFormat)
		m_col->m_heatmapFormat = new AbstractColumn::HeatmapFormat();

	auto tmp = *(m_col->m_heatmapFormat);
	*(m_col->m_heatmapFormat) = m_format;
	m_format = std::move(tmp);

	Q_EMIT m_col->owner()->formatChanged(m_col->owner());
}

void AbstractColumnSetHeatmapFormatCmd::undo() {
	redo();
}

/** ***************************************************************************
 * \class AbstractColumnRemoveHeatmapFormatCmd
 * \brief Set the heatmap format
 ** ***************************************************************************/
AbstractColumnRemoveHeatmapFormatCmd::AbstractColumnRemoveHeatmapFormatCmd(AbstractColumnPrivate* col)
	: m_col(col) {
	setText(i18n("%1: remove heatmap format", col->name()));
}

AbstractColumnRemoveHeatmapFormatCmd::~AbstractColumnRemoveHeatmapFormatCmd() = default;

void AbstractColumnRemoveHeatmapFormatCmd::redo() {
	if (m_col->m_heatmapFormat) {
		m_format = *(m_col->m_heatmapFormat);
		delete m_col->m_heatmapFormat;
		m_col->m_heatmapFormat = nullptr;
	}

	Q_EMIT m_col->owner()->formatChanged(m_col->owner());
}

void AbstractColumnRemoveHeatmapFormatCmd::undo() {
	if (!m_col->m_heatmapFormat)
		m_col->m_heatmapFormat = new AbstractColumn::HeatmapFormat();

	*(m_col->m_heatmapFormat) = m_format;

	Q_EMIT m_col->owner()->formatChanged(m_col->owner());
}
