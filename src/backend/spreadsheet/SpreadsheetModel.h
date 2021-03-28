/***************************************************************************
    File                 : SpreadsheetModel.h
    Project              : LabPlot
    Description          : Model for the access to a Spreadsheet
    --------------------------------------------------------------------
    Copyright            : (C) 2007 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2009 Knut Franke (knut.franke@gmx.de)
    Copyright            : (C) 2013-2021 Alexander Semke (alexander.semke@web.de)

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

#ifndef SPREADSHEETMODEL_H
#define SPREADSHEETMODEL_H

#include <QAbstractItemModel>

class QStringList;
class Column;
class Spreadsheet;
class AbstractAspect;
class AbstractColumn;

class SpreadsheetModel : public QAbstractItemModel {
	Q_OBJECT

public:
	explicit SpreadsheetModel(Spreadsheet*);

	struct HeatmapFormat {
		double min = 0.0;
		double max = 1.0;
		QString name;
		bool fillBackground = true;
		QVector<QColor> colors;
	};

	enum class CustomDataRole {
		MaskingRole = Qt::UserRole, //!< bool determining whether the cell is masked
		FormulaRole = Qt::UserRole+1, //!< the cells formula
		CommentRole = Qt::UserRole+2, //!< the column comment (for headerData())
	};

	Qt::ItemFlags flags( const QModelIndex & index ) const override;
	QVariant data(const QModelIndex& index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation,int role) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role) override;
	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex& child) const override;
	bool hasChildren (const QModelIndex& parent = QModelIndex() ) const override;

	Column* column(int index);

	void activateFormulaMode(bool on);
	bool formulaModeActive() const;

	void updateHorizontalHeader();
	void suppressSignals(bool);

	void setHeatmapFormat(QVector<Column*>, const HeatmapFormat&);

private slots:
	void handleAspectAdded(const AbstractAspect*);
	void handleAspectAboutToBeRemoved(const AbstractAspect*);
	void handleAspectRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child);

	void handleDescriptionChange(const AbstractAspect*);
	void handleModeChange(const AbstractColumn*);
	void handleDigitsChange();
	void handlePlotDesignationChange(const AbstractColumn*);
	void handleDataChange(const AbstractColumn*);
	void handleRowsInserted(const AbstractColumn* col, int before, int count);
	void handleRowsRemoved(const AbstractColumn* col, int first, int count);

protected:
	void updateVerticalHeader();

private:
	Spreadsheet* m_spreadsheet;
	bool m_formula_mode{false};
	QVector<int> m_vertical_header_data;
	QStringList m_horizontal_header_data;
	int m_defaultHeaderHeight;
	bool m_suppressSignals{false};
	int m_rowCount{0};
	int m_columnCount{0};
	QMap<QString, HeatmapFormat> m_heatmapFormats;

	QVariant backgroundColor(const Column*, int row, bool fillBackground) const;
};

#endif
