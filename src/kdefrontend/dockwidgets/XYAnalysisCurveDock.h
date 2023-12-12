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
	void showResult(const XYAnalysisCurve* curve, QTextEdit* teResult);
	virtual QString customText() const;

	void setAnalysisCurves(QList<XYCurve*>);
	void setModel(const QList<AspectType>& list);
	void setBaseWidgets(QLineEdit* nameLabel, ResizableTextEdit* commentLabel, QPushButton* recalculate, QComboBox* cbDataSourceType = nullptr);
	void enableRecalculate() const;

	QVector<XYAnalysisCurve*> m_analysisCurves;
	XYAnalysisCurve* m_analysisCurve{nullptr};
	RequiredDataSource m_requiredDataSource{RequiredDataSource::XY};
	QPushButton* m_recalculateButton{nullptr};
	QComboBox* cbDataSourceType{nullptr};
	TreeViewComboBox* cbDataSourceCurve{nullptr};
	TreeViewComboBox* cbXDataColumn{nullptr};
	TreeViewComboBox* cbYDataColumn{nullptr};
	TreeViewComboBox* cbY2DataColumn{nullptr};

protected Q_SLOTS:
	void yDataColumnChanged(const QModelIndex&);

	// SLOTs for changes triggered in Dock
	// General-Tab
	void curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType);
	void curveDataSourceCurveChanged(const XYCurve*);
	void curveXDataColumnChanged(const AbstractColumn*);
	void curveYDataColumnChanged(const AbstractColumn*);
};

#endif // XYANALYSISCURVEDOCK_H
