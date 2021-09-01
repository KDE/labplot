/*
    File                 : SimpleCopyThroughFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007 Knut Franke Tilman Benkert
    Email (use @ for *)  : knut.franke*gmx.de, thzs*gmx.net
    Description          : Filter which copies the provided input unaltered
    to the output

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/
#ifndef SIMPLE_COPY_THROUGH_FILTER_H
#define SIMPLE_COPY_THROUGH_FILTER_H

#include "backend/core/AbstractSimpleFilter.h"

/**
 * \brief Filter which copies the provided input unaltered to the output
 *
 * Most of the necessary methods for this filter are already implemented
 * in AbstractSimpleFilter.
 *
 * The difference between this filter and CopyThroughFilter is that
 * this inherits AbstractColumn and thus can be directly used
 * as input for other filters and plot functions.
 */
class SimpleCopyThroughFilter : public AbstractSimpleFilter {
	Q_OBJECT

protected:
	//! All types are accepted.
	bool inputAcceptable(int, const AbstractColumn *) override{
		return true;
	}
};

#endif // ifndef SIMPLE_COPY_THROUGH_FILTER_H
