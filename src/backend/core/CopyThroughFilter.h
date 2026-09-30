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
	explicit CopyThroughFilter(const QString& name)
		: AbstractFilter(name) {
	}

	virtual int inputCount() const override;
	virtual int outputCount() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	AbstractColumn* output(int port) override;
	virtual AbstractColumn* output(int port) const override;
};

#endif // ifndef COPY_THROUGH_FILTER_H
