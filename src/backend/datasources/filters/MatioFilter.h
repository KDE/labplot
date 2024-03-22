/*
	File                 : MatioFilter.h
	Project              : LabPlot
	Description          : Matio I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef MATIOFILTER_H
#define MATIOFILTER_H

#include "backend/datasources/filters/AbstractFileFilter.h"

class MatioFilterPrivate;

// Docu:
//      matio_user_guide.pdf
//      http://na-wiki.csc.kth.se/mediawiki/index.php/MatIO
//      https://github.com/NJannasch/matio-examples
// Example data:
//      https://github.com/cran/R.matlab/tree/master/inst/mat-files
//      https://github.com/scipy/scipy/tree/master/scipy/io/matlab/tests/data/
class MatioFilter : public AbstractFileFilter {
	Q_OBJECT

public:
	MatioFilter();
	~MatioFilter() override;

	static QString fileInfoString(const QString&);

	void parse(const QString& fileName);
	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, ImportMode = ImportMode::Replace) override;
	QVector<QStringList> readCurrentVar(const QString& fileName, AbstractDataSource* = nullptr, ImportMode = ImportMode::Replace, int lines = -1);
	void write(const QString& fileName, AbstractDataSource*) override;

	void setCurrentVarName(const QString&);
	void setSelectedVarNames(const QStringList&);
	const QStringList selectedVarNames() const;
	size_t varCount() const;
	QVector<QStringList> varsInfo() const;

	// TODO: -> AbstractFileFilter?
	void setStartRow(int);
	int startRow() const;
	void setEndRow(int);
	int endRow() const;
	void setStartColumn(int);
	int startColumn() const;
	void setEndColumn(int);
	int endColumn() const;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*) override;

private:
	std::unique_ptr<MatioFilterPrivate> const d;

	friend class MatioFilterPrivate;
};

#endif
