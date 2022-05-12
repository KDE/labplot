/***************************************************************************
File                 : ExcelFilter.h
Project              : LabPlot
Description          : Excel I/O-filter
--------------------------------------------------------------------
Copyright            : (C) 2021 by Fabian Kristof (fkristofszabolcs@gmail.com)
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#ifndef EXCELFILTER_H
#define EXCELFILTER_H
#include "backend/datasources/filters/AbstractFileFilter.h"
#include <memory>

#include <QObject>

#ifdef HAVE_EXCEL
#include "3rdparty/QXlsx/src/QXlsx/QXlsx/header/xlsxcellrange.h"
#include "3rdparty/QXlsx/src/QXlsx/QXlsx/header/xlsxcellreference.h"
#include "3rdparty/QXlsx/src/QXlsx/QXlsx/header/xlsxdocument.h"
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
