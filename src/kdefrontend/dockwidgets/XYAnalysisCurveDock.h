#ifndef XYANALYSISCURVEDOCK_H
#define XYANALYSISCURVEDOCK_H

#include "XYCurveDock.h"
#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"

class TreeViewComboBox;

class XYAnalysisCurveDock : public XYCurveDock {
public:
	enum class RequiredDataSource { XY, Y, YY2 };

	explicit XYAnalysisCurveDock(QWidget* parent, RequiredDataSource required = RequiredDataSource::XY);

protected:
	void showResult(const XYAnalysisCurve* curve, QTextEdit* teResult, QPushButton* pbRecalculate);
	virtual QString customText() const;

	void setAnalysisCurves(QList<XYCurve*>);
	void setModel(const QList<AspectType>& list);
	void setBaseWidgets(QLineEdit* nameLabel, ResizableTextEdit* commentLabel, QPushButton* recalculate, double commentHeightFactorNameLabel = 1.2);
	void enableRecalculate() const;

	QVector<XYAnalysisCurve*> m_analysisCurves;
	XYAnalysisCurve* m_analysisCurve{nullptr};
	RequiredDataSource m_requiredDataSource{RequiredDataSource::XY};
	QPushButton* m_recalculateButton{nullptr};
	TreeViewComboBox* cbDataSourceCurve{nullptr};
	TreeViewComboBox* cbXDataColumn{nullptr};
	TreeViewComboBox* cbYDataColumn{nullptr};
	TreeViewComboBox* cbY2DataColumn{nullptr};

protected Q_SLOTS:
	void yDataColumnChanged(const QModelIndex&);
};

#endif // XYANALYSISCURVEDOCK_H
