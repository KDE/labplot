#ifndef XYANALYSISCURVEDOCK_H
#define XYANALYSISCURVEDOCK_H

#include "XYCurveDock.h"
#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"

class XYAnalysisCurveDock : public XYCurveDock {
public:
	explicit XYAnalysisCurveDock(QWidget*);

protected:
	void showResult(const XYAnalysisCurve* curve, QTextEdit* teResult, QPushButton* pbRecalculate);
	virtual QString customText() const;
};

#endif // XYANALYSISCURVEDOCK_H
