/*
	File                 : XLSXFilter.h
	Project              : LabPlot
	Description          : XLSX I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Fabian Kristof (fkristofszabolcs@gmail.com)
	SPDX-FileCopyrightText: 2022-2023 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XLSXFILTER_H
#define XLSXFILTER_H
#include "backend/datasources/filters/AbstractFileFilter.h"

#include <QObject>

#include <memory>

class XLSXFilterPrivate;
class QTreeWidgetItem;

class XLSXFilter : public AbstractFileFilter {
	Q_OBJECT
public:
	explicit XLSXFilter();
	virtual ~XLSXFilter() override;
	static QString fileInfoString(const QString& fileName);
	static QStringList sheets(const QString& fileName, bool* ok = nullptr);
	static bool isValidCellReference(const QString& cellRefString);

#ifdef HAVE_QXLSX
	QVector<QStringList> previewForDataRegion(const QString& sheet, const QString& region, bool* okToMatrix, int lines);
#endif
	QVector<QStringList> previewForCurrentDataRegion(int lines, bool* okToMatrix);
	QStringList sheets() const;

	void setExportAsNewSheet(bool);
	void setSheetToAppendTo(const QString& sheetName);
	void setOverwriteData(bool);
	void setDataExportStartPos(const QString&);
	void setFirstRowAsColumnNames(bool);
	void setColumnNamesAsFirstRow(bool);

	void parse(const QString& fileName, QTreeWidgetItem* root);

	void setCurrentRange(const QString&);

	void setCurrentSheet(const QString&);
	virtual void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, ImportMode = ImportMode::Replace) override;
	virtual void write(const QString& fileName, AbstractDataSource*) override;

	virtual void save(QXmlStreamWriter*) const override;
	virtual bool load(XmlStreamReader*) override;

	void setStartRow(const int);
	int startRow() const;
	void setEndRow(const int);
	int endRow() const;
	void setStartColumn(const int);
	int startColumn() const;
	void setEndColumn(const int);
	int endColumn() const;
	int firstColumn() const;

private:
	std::unique_ptr<XLSXFilterPrivate> const d;
	friend class XLSXFilterPrivate;
};

#endif // XLSXFILTER_H
