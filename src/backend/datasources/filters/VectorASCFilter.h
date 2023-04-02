/*
	File                 : VectorASCFilter.h
	Project              : LabPlot
	Description          : Vector ASC I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef VECTORASCFILTER_H
#define VECTORASCFILTER_H

#include "backend/datasources/filters/CANFilter.h"

class VectorASCFilterPrivate;

class VectorASCFilter : public CANFilter {
	Q_OBJECT

public:
	VectorASCFilter();
	~VectorASCFilter() override;

	static bool isValid(const QString& filename);
	/*!
	 * \brief fileInfoString
	 * Information about the file content
	 * \return
	 */
	static QString fileInfoString(const QString&);

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*) override;

private:
	friend class ASCFilterTest;
};

#endif // VECTORASCFILTER_H
