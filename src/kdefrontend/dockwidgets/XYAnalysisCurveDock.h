#ifndef XYANALYSISCURVEDOCK_H
#define XYANALYSISCURVEDOCK_H

#include "XYCurveDock.h"
#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"

class TreeViewComboBox;

class XYAnalysisCurveDock : public XYCurveDock {
public:
	explicit XYAnalysisCurveDock(QWidget*);

protected:
	void showResult(const XYAnalysisCurve* curve, QTextEdit* teResult, QPushButton* pbRecalculate);
	virtual QString customText() const;

	virtual void setModel(const QList<AspectType>& list);

protected:
	TreeViewComboBox* cbDataSourceCurve{nullptr};
	TreeViewComboBox* cbXDataColumn{nullptr};
	TreeViewComboBox* cbYDataColumn{nullptr};
};

#endif // XYANALYSISCURVEDOCK_H
