/*
	File                 : InfoElementTest.h
	Project              : LabPlot
	Description          : Tests for InfoElement
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>

 SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INFOELEMENTTEST_H
#define INFOELEMENTTEST_H

#include "../../CommonTest.h"

class InfoElementTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:

	void addPlot();
	/*!
	 * \brief addRemoveCurve
	 * Precondition: Infoelement contains 2 curves
	 * Removing first curve from the infoelement
	 */
	void removeCurve();
	/*!
	 * \brief deleteCurveRenameAddedAutomatically
	 * Curve got deleted and added with a new name, after renaming the curve,
	 * the curve will be automatically assigned to the infoelement
	 */
	void deleteCurveRenameAddedAutomatically();
	/*!
	 * \brief deleteCurveRenameAddedAutomaticallyCustomPointInvisible
	 * Same as deleteCurveRenameAddedAutomatically(), but the custompoint
	 * is always invisible
	 */
	void deleteCurveRenameAddedAutomaticallyCustomPointInvisible();
	/*!
	 * \brief addRemoveColumn
	 * Removing a column from a xycurve which is
	 * used in an infoelement, shall lead in invaliding the
	 * infoelement if only one curve exists
	 */
	void removeColumn();
	/*!
	 * \brief changeColumn
	 * Changing the column of the curve
	 */
	void changeColumn();

	/*!
	 * \brief columnValueChanged
	 * Changing the column data after adding the curve
	 */
	void columnValueChanged();

	/*!
	 * \brief addRemoveRenameColumn
	 * First part same as in removeColumn().
	 * The column will be renamed after removed from the spreadsheet
	 * and added again to the spreadsheet. After renaming the column
	 * again to the original name, the infoelement gets valid again
	 */
	void addRemoveRenameColumn();
	/*!
	 * \brief moveDuringMissingCurve
	 * Change the logical position of the infoelement after
	 * the curve was removed. The application shall not crash
	 */
	void moveDuringMissingCurve();
	/*!
	 * \brief moveCurve
	 * Moving the order of the elements shall not
	 * remove the curve
	 */
	void moveCurve();
	void saveLoad();

	void saveLoadInvisiblePoint();
};

#endif // INFOELEMENTTEST_H
