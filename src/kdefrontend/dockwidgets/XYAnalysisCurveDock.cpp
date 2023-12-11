#include "XYAnalysisCurveDock.h"

#include "commonfrontend/widgets/TreeViewComboBox.h"

XYAnalysisCurveDock::XYAnalysisCurveDock(QWidget* parent, RequiredDataSource required)
	: XYCurveDock(parent)
	, m_requiredDataSource(required) {
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
}

void XYAnalysisCurveDock::setBaseWidgets(QLineEdit* nameLabel, ResizableTextEdit* commentLabel, QPushButton* recalculate, double commentHeightFactorNameLabel) {
	if (m_recalculateButton)
		disconnect(m_recalculateButton, nullptr, this, nullptr);

	m_recalculateButton = recalculate;
	Q_ASSERT(m_recalculateButton);

	BaseDock::setBaseWidgets(nameLabel, commentLabel, commentHeightFactorNameLabel);
}

void XYAnalysisCurveDock::setModel(const QList<AspectType>& topLevelClasses) {
	if (cbDataSourceCurve) {
		// The FourierTransformCurveDock and the XYHilbertTransformCurveDock don't use this datasource
		// choosing therefore cbDataSourceCurve will not be initialized
		QList<AspectType> list{AspectType::Folder,
							   AspectType::Datapicker,
							   AspectType::Worksheet,
							   AspectType::CartesianPlot,
							   AspectType::XYCurve,
							   AspectType::XYAnalysisCurve};
		cbDataSourceCurve->setTopLevelClasses(list);

		QList<const AbstractAspect*> hiddenAspects;
		for (auto* curve : m_curvesList)
			hiddenAspects << curve;
		cbDataSourceCurve->setHiddenAspects(hiddenAspects);
	}

	auto* model = aspectModel();
	if (cbY2DataColumn) {
		cbY2DataColumn->setTopLevelClasses(topLevelClasses);
		cbY2DataColumn->setModel(model);
	}

	cbXDataColumn->setTopLevelClasses(topLevelClasses);
	cbYDataColumn->setTopLevelClasses(topLevelClasses);

	if (cbDataSourceCurve)
		cbDataSourceCurve->setModel(model);
	cbXDataColumn->setModel(model);
	cbYDataColumn->setModel(model);

	XYCurveDock::setModel();
}

void XYAnalysisCurveDock::enableRecalculate() const {
	CONDITIONAL_RETURN_NO_LOCK;

	// enable the recalculate button if all required data source columns were provided, disable otherwise
	bool hasSourceData = false;
	if (m_analysisCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet) {
		AbstractAspect* aspectX = nullptr;
		AbstractAspect* aspectY = nullptr;
		AbstractAspect* aspectY2 = nullptr;
		switch(m_requiredDataSource) {
		case RequiredDataSource::XY: {
			aspectX = static_cast<AbstractAspect*>(cbXDataColumn->currentModelIndex().internalPointer());
			aspectY = static_cast<AbstractAspect*>(cbYDataColumn->currentModelIndex().internalPointer());
			hasSourceData = (aspectX != nullptr && aspectY != nullptr);
			break;
		}
		case RequiredDataSource::Y: {
			aspectY = static_cast<AbstractAspect*>(cbYDataColumn->currentModelIndex().internalPointer());
			hasSourceData = (aspectY != nullptr);
			break;
		}
		case RequiredDataSource::YY2: {
			aspectY = static_cast<AbstractAspect*>(cbYDataColumn->currentModelIndex().internalPointer());
			aspectY2 = static_cast<AbstractAspect*>(cbY2DataColumn->currentModelIndex().internalPointer());
			hasSourceData = (aspectY != nullptr && aspectY2 != nullptr);
			break;
		}
		}

		if (aspectX) {
			cbXDataColumn->useCurrentIndexText(true);
			cbXDataColumn->setInvalid(false);
		}

		if (aspectY) {
			cbYDataColumn->useCurrentIndexText(true);
			cbYDataColumn->setInvalid(false);
		}

		if (aspectY2) {
			cbY2DataColumn->useCurrentIndexText(true);
			cbY2DataColumn->setInvalid(false);
		}
	} else
		hasSourceData = (m_analysisCurve->dataSourceCurve() != nullptr);

	m_recalculateButton->setEnabled(hasSourceData);
}
