/*
	File                 : Spreadsheet.h
	Project              : LabPlot
	Description          : Aspect providing a spreadsheet table with column logic
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2010-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2006-2008 Tilman Benkert <thzs@gmx.net>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SPREADSHEET_H
#define SPREADSHEET_H

#include "backend/datasources/AbstractDataSource.h"
#include "backend/lib/macros.h"

class AbstractFileFilter;
class Column;
class SpreadsheetView;
class SpreadsheetModel;
class SpreadsheetPrivate;
class StatisticsSpreadsheet;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT Spreadsheet : public AbstractDataSource {
#else
class Spreadsheet : public AbstractDataSource {
#endif
	Q_OBJECT

public:
	explicit Spreadsheet(const QString& name, bool loading = false, AspectType type = AspectType::Spreadsheet);
	~Spreadsheet() override;

	QIcon icon() const override;
	QString caption() const override;
	QMenu* createContextMenu() override;
	void fillColumnContextMenu(QMenu*, Column*);
	void fillColumnsContextMenu(QMenu*);
	QWidget* view() const override;
	StatisticsSpreadsheet* statisticsSpreadsheet() const;

	bool exportView() const override;
	bool printView() override;
	bool printPreview() const override;

	void setModel(SpreadsheetModel*);
	SpreadsheetModel* model() const;

	QVector<AspectType> pasteTypes() const override;
	QVector<AspectType> dropableOn() const override;

	void updateHorizontalHeader();
	void updateLocale();

	int columnCount() const;
	int columnCount(AbstractColumn::PlotDesignation) const;
	Column* column(int index) const;
	Column* column(const QString&) const;
	int rowCount() const; // TODO: should be size_t?

	void removeRows(int first, int count);
	void insertRows(int before, int count);
	void removeColumns(int first, int count);
	void insertColumns(int before, int count);

	QString text(int row, int col) const;

	BASIC_D_ACCESSOR_DECL(bool, readOnly, ReadOnly)
	BASIC_D_ACCESSOR_DECL(bool, showComments, ShowComments)
	BASIC_D_ACCESSOR_DECL(bool, showSparklines, ShowSparklines)
	BASIC_D_ACCESSOR_DECL(bool, linking, Linking)

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	void setColumnSelectedInView(int index, bool selected);

	// used from model to inform dock
	void emitRowCountChanged() {
		Q_EMIT rowCountChanged(rowCount());
	}
	void emitColumnCountChanged() {
		Q_EMIT columnCountChanged(columnCount());
	}

	// data import
	int prepareImport(std::vector<void*>& dataContainer,
					  AbstractFileFilter::ImportMode,
					  int rows,
					  int cols,
					  const QStringList& colNameList,
					  const QVector<AbstractColumn::ColumnMode>&,
					  bool& ok,
					  bool initializeContainer) override;
	void finalizeImport(size_t columnOffset, size_t startColumn, size_t endColumn, const QString& dateTimeFormat, AbstractFileFilter::ImportMode) override;
	int resize(AbstractFileFilter::ImportMode, const QStringList& colNameList, int cols);

	struct Linking {
		bool linking{false};
		const Spreadsheet* linkedSpreadsheet{nullptr};
		QString linkedSpreadsheetPath;

		QString spreadsheetPath() const {
			if (linkedSpreadsheet)
				return linkedSpreadsheet->path();
			return linkedSpreadsheetPath;
		}
	};

	typedef SpreadsheetPrivate Private;

public Q_SLOTS:
	void appendRows(int);
	void appendRow();
	void removeEmptyRows();
	void maskEmptyRows();
	void appendColumns(int);
	void appendColumn();
	void prependColumns(int);

	void setColumnCount(int);
	void setRowCount(int);

	const Spreadsheet* linkedSpreadsheet() const;
	void setLinkedSpreadsheet(const Spreadsheet*, bool skipUndo = false);
	QString linkedSpreadsheetPath() const;

	void clear();
	void clear(const QVector<Column*>&);
	void clearMasks();
	void transpose();

	void moveColumn(int from, int to);
	void sortColumns(Column* leading, const QVector<Column*>&, bool ascending);

	void toggleStatisticsSpreadsheet(bool);

private:
	void init();
	void initConnectionsLinking(const Spreadsheet* sender, const Spreadsheet* receiver);
	QVector<int> rowsWithMissingValues() const;
	Q_DECLARE_PRIVATE(Spreadsheet)

	SpreadsheetPrivate* const d_ptr;
	SpreadsheetModel* m_model{nullptr};

protected:
	mutable SpreadsheetView* m_view{nullptr};
	void setSuppressSetCommentFinalizeImport(bool);

private Q_SLOTS:
	void childSelected(const AbstractAspect*) override;
	void childDeselected(const AbstractAspect*) override;
	void initConnectionsRowCountChanges();
	void linkedSpreadsheetDeleted();
	void linkedSpreadsheetNewRowCount(int);
	void handleAspectUpdated(const QString& aspectPath, const AbstractAspect*);

Q_SIGNALS:
	void requestProjectContextMenu(QMenu*);
	void columnSelected(int);
	void columnDeselected(int);

	// for spreadsheet dock
	void rowCountChanged(int);
	void columnCountChanged(int);
	void aboutToResize();
	void resizeFinished();

	void aspectsAboutToBeInserted(int first, int last);
	void aspectsInserted(int first, int last);
	void aspectsAboutToBeRemoved(int first, int last);
	void aspectsRemoved();

	void manyAspectsAboutToBeInserted();
	void rowsAboutToBeInserted(int before, int last);
	void rowsInserted(int newRowCount);
	void rowsAboutToBeRemoved(int first, int count);
	void rowsRemoved(int newRowCount);

	void showCommentsChanged(bool);
	void showSparklinesChanged(bool);
	void linkingChanged(bool);
	void linkedSpreadsheetChanged(const Spreadsheet*);

	friend class SpreadsheetSetLinkingCmd;
	friend class SpreadsheetSetColumnCountCommand;
	friend class Project; // handleAspectUpdated required
};

#endif
