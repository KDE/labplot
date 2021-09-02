/*
    File                 : CopyThroughFilter.h
    Project              : SciDAVis
    Description          : Filter which copies all provided inputs unaltered
    to an equal number of outputs.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007 Knut Franke <knut.franke*gmx.de (use @ for *)>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COPY_THROUGH_FILTER_H
#define COPY_THROUGH_FILTER_H

#include "AbstractFilter.h"

class CopyThroughFilter : public AbstractFilter {
public:
	virtual int inputCount() const;
	virtual int outputCount() const;
	virtual AbstractColumn* output(int port) const;
};

#endif // ifndef COPY_THROUGH_FILTER_H

