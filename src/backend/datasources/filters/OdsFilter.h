/*
	File                 : OdsFilter.h
	Project              : LabPlot
	Description          : Ods I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ODSFILTER_H
#define ODSFILTER_H
#include "backend/datasources/filters/AbstractFileFilter.h"

#include <QObject>

#ifdef HAVE_ORCUS
//#include "3rdparty/QXlsx/header/xlsxdocument.h"
#endif

class OdsFilterPrivate;
class QTreeWidgetItem;

class OdsFilter : public AbstractFileFilter {
	Q_OBJECT
public:
	explicit OdsFilter();
	virtual ~OdsFilter() override;
	static QString fileInfoString(const QString& fileName);
	QVector<QStringList> preview(const QString& sheetName, int lines);
	/*	static QStringList sheets(const QString& fileName, bool* ok = nullptr);
		static bool isValidCellReference(const QString& cellRefString);

		QStringList sheets() const;

		void setExportAsNewSheet(const bool);
		void setSheetToAppendTo(const QString& sheetName);
		void setOverwriteData(const bool);
		void setDataExportStartPos(const QString&);
	*/
	void setFirstRowAsColumnNames(const bool);
	void parse(const QString& fileName, QTreeWidgetItem* root);
	void setCurrentSheetName(const QString&);
	virtual void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, ImportMode = ImportMode::Replace) override;
	virtual void write(const QString& fileName, AbstractDataSource*) override;

	void setSelectedSheetNames(const QStringList&);
	const QStringList selectedSheetNames() const;

	virtual void save(QXmlStreamWriter*) const override;
	virtual bool load(XmlStreamReader*) override;

	void setStartRow(int);
	int startRow() const;
	void setEndRow(int);
	int endRow() const;
	void setStartColumn(int);
	int startColumn() const;
	void setEndColumn(int);
	int endColumn() const;

private:
	std::unique_ptr<OdsFilterPrivate> const d;

	friend class OdsFilterPrivate;
};

#endif