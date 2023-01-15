#include "XYAnalysisCurveDock.h"

XYAnalysisCurveDock::XYAnalysisCurveDock(QWidget* parent)
	: XYCurveDock(parent) {
}

/*!
 * show the result and details of the transform
 */
void XYAnalysisCurveDock::showResult(const XYAnalysisCurve* curve, QTextEdit* teResult, QPushButton* pbRecalculate) {
	const auto& result = curve->result();
	if (!result.available) {
		teResult->clear();
		return;
	}

	QString str = i18n("status: %1", result.status) + QStringLiteral("<br>");

	if (!result.valid) {
		teResult->setText(str);
		return; // result is not valid, there was an error which is shown in the status-string, nothing to show more.
	}

	const auto numberLocale = QLocale();
	if (result.elapsedTime > 1000)
		str += i18n("calculation time: %1 s", numberLocale.toString(result.elapsedTime / 1000)) + QStringLiteral("<br>");
	else
		str += i18n("calculation time: %1 ms", numberLocale.toString(result.elapsedTime)) + QStringLiteral("<br>");

	str += customText();

	str += QStringLiteral("<br><br>");

	teResult->setText(str);

	// enable the "recalculate"-button if the source data was changed since the last calculation
	pbRecalculate->setEnabled(curve->isSourceDataChangedSinceLastRecalc());
}

QString XYAnalysisCurveDock::customText() const {
	return QStringLiteral("");
};
