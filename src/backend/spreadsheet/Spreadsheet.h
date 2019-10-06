/***************************************************************************
    File                 : Spreadsheet.h
    Project              : LabPlot
    Description          : Aspect providing a spreadsheet table with column logic
    --------------------------------------------------------------------
    Copyright            : (C) 2010-2017 Alexander Semke(alexander.semke@web.de)
    Copyright            : (C) 2006-2008 Tilman Benkert (thzs@gmx.net)

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
#ifndef SPREADSHEET_H
#define SPREADSHEET_H

#include "backend/datasources/AbstractDataSource.h"
#include "backend/core/column/ColumnStringIO.h"

class AbstractFileFilter;
class SpreadsheetView;
class SpreadsheetModel;
template <class T> class QVector;

class Spreadsheet : public AbstractDataSource {
	Q_OBJECT

public:
	explicit Spreadsheet(const QString& name, bool loading = false, AspectType type = AspectType::Spreadsheet);

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QWidget* view() const override;

	bool exportView() const override;
	bool printView() override;
	bool printPreview() const override;

	void setModel(SpreadsheetModel*);
	SpreadsheetModel* model();

	int columnCount() const;
	int columnCount(AbstractColumn::PlotDesignation) const;
	Column* column(int index) const;
	Column* column(const QString&) const;
	int rowCount() const;

	void removeRows(int first, int count);
	void insertRows(int before, int count);
	void removeColumns(int first, int count);
	void insertColumns(int before, int count);

	int colX(int col);
	int colY(int col);
	QString text(int row, int col) const;

	void copy(Spreadsheet* other);

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	void setColumnSelectedInView(int index, bool selected);

	// used from model to inform dock
	void emitRowCountChanged() { emit rowCountChanged(rowCount()); }
	void emitColumnCountChanged() { emit columnCountChanged(columnCount()); }

	void registerShortcuts() override;
	void unregisterShortcuts() override;

	//data import
	int prepareImport(std::vector<void*>& dataContainer, AbstractFileFilter::ImportMode,
		int rows, int cols, QStringList colNameList, QVector<AbstractColumn::ColumnMode>) override;
	void finalizeImport(int columnOffset, int startColumn , int endColumn,
		const QString& dateTimeFormat, AbstractFileFilter::ImportMode) override;
	int resize(AbstractFileFilter::ImportMode, QStringList colNameList, int cols);

public slots:
	void appendRows(int);
	void appendRow();
	void appendColumns(int);
	void appendColumn();
	void prependColumns(int);

	void setColumnCount(int);
	void setRowCount(int);

	void clear();
	void clearMasks();

	void moveColumn(int from, int to);
	void sortColumns(Column* leading, QVector<Column*>, bool ascending);

private:
	void init();
	SpreadsheetModel* m_model{nullptr};

protected:
	mutable SpreadsheetView* m_view{nullptr};

private slots:
	void childSelected(const AbstractAspect*) override;
	void childDeselected(const AbstractAspect*) override;

signals:
	void requestProjectContextMenu(QMenu*);
	void columnSelected(int);
	void columnDeselected(int);

	// for spreadsheet dock
	void rowCountChanged(int);
	void columnCountChanged(int);
};

#endif
