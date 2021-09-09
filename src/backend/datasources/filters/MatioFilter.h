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

class QStringList;
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
	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace) override;
	QVector<QStringList> readCurrentVar(const QString& fileName, AbstractDataSource* = nullptr,
			AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace, int lines = -1);
	void write(const QString& fileName, AbstractDataSource*) override;

	void loadFilterSettings(const QString&) override;
	void saveFilterSettings(const QString&) const override;

	void setCurrentVarName(const QString&);
	const QString currentVarName() const;
//TODO: support importing multiple vars?
//	void setCurrentVarNames(const QStringList&);
//	const QStringList currentVarNames() const;
	size_t varCount() const;
	QVector<QStringList> varsInfo() const;

	//TODO: -> AbstractFileFilter?
	void setStartRow(const int);
	int startRow() const;
	void setEndRow(const int);
	int endRow() const;
	void setStartColumn(const int);
	int startColumn() const;
	void setEndColumn(const int);
	int endColumn() const;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*) override;

private:
	std::unique_ptr<MatioFilterPrivate> const d;
	friend class MatioFilterPrivate;
};

#endif
