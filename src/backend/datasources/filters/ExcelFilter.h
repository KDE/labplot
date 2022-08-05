/*
	File                 : ExcelFilter.h
	Project              : LabPlot
	Description          : Excel I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Fabian Kristof (fkristofszabolcs@gmail.com)
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EXCELFILTER_H
#define EXCELFILTER_H
#include "backend/datasources/filters/AbstractFileFilter.h"
#include <memory>

#include <QObject>

#ifdef HAVE_EXCEL
#include "3rdparty/QXlsx/header/xlsxcellrange.h"
#include "3rdparty/QXlsx/header/xlsxcellreference.h"
#include "3rdparty/QXlsx/header/xlsxdocument.h"
#endif

class ExcelFilterPrivate;
class QTreeWidgetItem;

class ExcelFilter : public AbstractFileFilter {
	Q_OBJECT
public:
	explicit ExcelFilter();
	virtual ~ExcelFilter() override;
	static QString fileInfoString(const QString& fileName);
	static QStringList sheets(const QString& fileName, bool* ok = nullptr);
	static bool isValidCellReference(const QString& cellRefString);

#ifdef HAVE_EXCEL
	QVector<QStringList> previewForDataRegion(const QString& sheet, const QXlsx::CellRange& region, bool* okToMatrix, int lines);
#endif
	QVector<QStringList> previewForCurrentDataRegion(int lines, bool* okToMatrix);
	QStringList sheets() const;

	void setExportAsNewSheet(const bool);
	void setSheetToAppendTo(const QString& sheetName);
	void setOverwriteData(const bool);
	void setDataExportStartPos(const QString&);
	void setFirstRowAsColumnNames(const bool);

	void parse(const QString& fileName, QTreeWidgetItem* root);
	static QString convertFromNumberToExcelColumn(int n);

#ifdef HAVE_EXCEL
	QVector<QXlsx::CellRange> dataRegions(const QString& fileName, const QString& sheetName);
	QXlsx::CellRange dimension() const;
#endif
	void setCurrentRange(const QString& range);

	void setCurrentSheet(const QString& sheet);
	virtual void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, ImportMode = ImportMode::Replace) override;
	virtual void write(const QString& fileName, AbstractDataSource*) override;

	virtual void loadFilterSettings(const QString& filterName) override;
	virtual void saveFilterSettings(const QString& filterName) const override;

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

private:
	std::unique_ptr<ExcelFilterPrivate> const d;
	friend class ExcelFilterPrivate;
};

#endif // EXCELFILTER_H
