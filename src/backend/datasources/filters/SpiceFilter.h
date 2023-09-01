/*
	File                 : SpiceFilter.h
	Project              : LabPlot
	Description          : Filters for reading spice files
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef SPICEFILTER_H
#define SPICEFILTER_H

#include "backend/core/AbstractColumn.h"
#include "backend/datasources/filters/AbstractFileFilter.h"

class QStringList;
class SpiceFilterPrivate;

// NgSpice/LtSpice Filter
class SpiceFilter : public AbstractFileFilter {
	Q_OBJECT

public:
	SpiceFilter();
	~SpiceFilter() override;

	static bool isSpiceFile(const QString& fileName, bool* binary = nullptr);
	static QString fileInfoString(const QString&);

	QVector<QStringList> preview(const QString& fileName, int lines);
	void
	readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace) override;
	void write(const QString& fileName, AbstractDataSource*) override;

	QStringList vectorNames() const;
	QVector<AbstractColumn::ColumnMode> columnModes();

	void setStartRow(const int);
	int startRow() const;
	void setEndRow(const int);
	int endRow() const;

#ifdef SPICEFILTERTEST_EN
	void setReaderBulkLineCount(int count) {
		mBulkLineCount = count;
	}
	int mBulkLineCount{100000};
#endif

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*) override;

	static const QString xmlElementName;

protected:
	std::unique_ptr<SpiceFilterPrivate> const d;
};

#endif // SPICEFILTER_H
