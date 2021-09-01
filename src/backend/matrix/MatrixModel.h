/*
    File                 : MatrixModel.h
    Project              : LabPlot
    Description          : Matrix data model
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2008-2009 Tilman Benkert <thzs@gmx.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef MATRIXMODEL_H
#define MATRIXMODEL_H

#include <QAbstractItemModel>

class Matrix;

class MatrixModel : public QAbstractItemModel {
	Q_OBJECT

public:
	explicit MatrixModel(Matrix*);

	//! \name Overloaded functions from QAbstractItemModel
	//@{
	Qt::ItemFlags flags(const QModelIndex&) const override;
	QVariant data(const QModelIndex& index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role) override;
	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex& child) const override;
	//@}

	void updateHeader();
	void setSuppressDataChangedSignal(bool);
	void setChanged();

private slots:
	void handleColumnsAboutToBeInserted(int before, int count);
	void handleColumnsInserted(int first, int count);
	void handleColumnsAboutToBeRemoved(int first, int count);
	void handleColumnsRemoved(int first, int count);
	void handleRowsAboutToBeInserted(int before, int count);
	void handleRowsInserted(int first, int count);
	void handleRowsAboutToBeRemoved(int first, int count);
	void handleRowsRemoved(int first, int count);
	void handleDataChanged(int top, int left, int bottom, int right);
	void handleCoordinatesChanged();
	void handleFormatChanged();

private:
	Matrix* m_matrix;
	bool m_suppressDataChangedSignal{false};

signals:
	void changed();
};

#endif
