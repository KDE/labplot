/*
	File                 : VectorBLFFilter.h
	Project              : LabPlot
	Description          : Vector BLF I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef VECTORBLFFILTER_H
#define VECTORBLFFILTER_H

#include "backend/datasources/filters/CANFilter.h"

class VectorBLFFilterPrivate;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT VectorBLFFilter : public CANFilter {
#else
class VectorBLFFilter : public CANFilter {
#endif
	Q_OBJECT

public:
	VectorBLFFilter();
	~VectorBLFFilter() override;

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
	friend class BLFFilterTest;
};

#endif // VECTORBLFFILTER_H
