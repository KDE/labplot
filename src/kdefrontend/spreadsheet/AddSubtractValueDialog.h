/*
	File                 : AddSubtractDialog.h
	Project              : LabPlot
	Description          : Dialog for adding/subtracting a value from column values
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ADDSUBTRACTVALUEDIALOG_H
#define ADDSUBTRACTVALUEDIALOG_H

#include "ui_addsubtractvaluewidget.h"
#include <QDialog>

class Column;
class Spreadsheet;
class Matrix;
class Project;
class TextLabel;
class XYCurve;

class QSpacerItem;
class QPushButton;

class AddSubtractValueDialog : public QDialog {
	Q_OBJECT

public:
	enum Operation { Add, Subtract, Multiply, Divide, SubtractBaseline };

	explicit AddSubtractValueDialog(Spreadsheet*, const QVector<Column*>&, Operation, QWidget* parent = nullptr);
	explicit AddSubtractValueDialog(Matrix*, Operation, QWidget* parent = nullptr);
	~AddSubtractValueDialog() override;

private:
	void init();
	void initValuesSpreadsheet();
	void initValuesMatrix();
	void generateForColumns();
	void generateForColumn(Column* col, int colIndex);
	void generateForMatrices();
	void subtractBaseline(QVector<double>&);
	QString getMessage(const QString&);
	void updateSpacer(bool);

	bool setIntValue(int& value, int columnIndex = 0) const;
	bool setBigIntValue(qint64& value, int columnIndex = 0) const;
	bool setDoubleValue(double& value, int columnIndex = 0) const;
	bool setDateTimeValue(qint64& value, int columnIndex = 0) const;

	Ui::AddSubtractValueWidget ui;
	QSpacerItem* m_verticalSpacer{nullptr};
	Spreadsheet* m_spreadsheet{nullptr};
	QVector<Column*> m_columns;
	Matrix* m_matrix{nullptr};
	QPushButton* m_okButton{nullptr};
	Operation m_operation;
	bool m_numeric{false};

	// preview related members
	bool m_previewDirty{true};
	bool m_xColumnBaselineDirty{true};
	Project* m_project{nullptr};
	XYCurve* m_curveOrigin{nullptr};
	XYCurve* m_curveBaseline{nullptr};
	XYCurve* m_curveResult{nullptr};
	Column* m_xColumnBaseline{nullptr};
	Column* m_yColumnBaseline{nullptr};
	Column* m_yColumnResult{nullptr};
	TextLabel* m_previewPlotTitle{nullptr};
	double m_arplsRatio{0.0};

private Q_SLOTS:
	void generate();
	void typeChanged(int);
	void previewChanged(bool);
	void initPreview();
	void invalidatePreview();
	void updatePreview();
};

#endif
