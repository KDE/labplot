#include "XYAnalysisCurveDock.h"

#include "frontend/widgets/TreeViewComboBox.h"

#include <QKeyEvent>

XYAnalysisCurveDock::XYAnalysisCurveDock(QWidget* parent, RequiredDataSource requiredDataSource)
	: XYCurveDock(parent)
	, m_requiredDataSource(requiredDataSource) {
}

/*!
 * show the result and details of the transform
 */
void XYAnalysisCurveDock::showResult(const XYAnalysisCurve* curve, QTextEdit* teResult) {
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
	m_recalculateButton->setEnabled(curve->isSourceDataChangedSinceLastRecalc());

	installEventFilter(this);
}

bool XYAnalysisCurveDock::eventFilter(QObject* /* watched */, QEvent* event) {
	if (event->type() == QEvent::KeyPress) {
		const auto* keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent->key() == Qt::Key_Return && keyEvent->modifiers() == Qt::ShiftModifier && m_recalculateButton->isEnabled()) {
			recalculateClicked();
			return true;
		}
	}

	return false;
}

QString XYAnalysisCurveDock::customText() const {
	return QStringLiteral("");
}

void XYAnalysisCurveDock::setBaseWidgets(TimedLineEdit* nameLabel, ResizableTextEdit* commentLabel, QPushButton* recalculate, QComboBox* dataSourceType) {
	if (m_recalculateButton)
		disconnect(m_recalculateButton, nullptr, this, nullptr);

	m_recalculateButton = recalculate;
	Q_ASSERT(m_recalculateButton);
	m_recalculateButton->setIcon(QIcon::fromTheme(QStringLiteral("run-build")));
	m_recalculateButton->setToolTip(i18n("Click this button or press Shift+Enter to recalculate the result."));

	cbDataSourceType = dataSourceType;
	if (cbDataSourceType) {
		cbDataSourceType->addItem(i18n("Spreadsheet"));
		cbDataSourceType->addItem(i18n("XY-Curve"));
	}

	BaseDock::setBaseWidgets(nameLabel, commentLabel);
}

void XYAnalysisCurveDock::setAnalysisCurves(const QList<XYCurve*>& curves) {
	m_analysisCurves.clear();
	m_analysisCurve = nullptr;

	for (auto* curve : curves)
		m_analysisCurves << static_cast<XYAnalysisCurve*>(curve);

	if (!curves.isEmpty())
		m_analysisCurve = m_analysisCurves.first();

	setModel();
}

void XYAnalysisCurveDock::setModelCurve(TreeViewComboBox* cb) {
	if (!cb)
		return;
	// The FourierTransformCurveDock and the XYHilbertTransformCurveDock don't use this datasource
	// choosing therefore cbDataSourceCurve will not be initialized
	QList<AspectType> list{AspectType::Folder,
						   AspectType::Datapicker,
						   AspectType::Worksheet,
						   AspectType::CartesianPlot,
						   AspectType::XYCurve,
						   AspectType::XYAnalysisCurve,
						   AspectType::XYEquationCurve};
	cb->setTopLevelClasses(list);

	QList<const AbstractAspect*> hiddenAspects;
	for (auto* curve : m_curvesList)
		hiddenAspects << curve;
	cb->setHiddenAspects(hiddenAspects);

	cb->setModel(aspectModel());
}

void XYAnalysisCurveDock::setModel() {
	setModelCurve(cbDataSourceCurve);

	auto* model = aspectModel();
	const auto& topLevelClasses = TreeViewComboBox::plotColumnTopLevelClasses();
	if (cbY2DataColumn) {
		cbY2DataColumn->setTopLevelClasses(topLevelClasses);
		cbY2DataColumn->setModel(model);
	}

	cbXDataColumn->setTopLevelClasses(topLevelClasses);
	cbYDataColumn->setTopLevelClasses(topLevelClasses);

	cbXDataColumn->setModel(model);
	cbYDataColumn->setModel(model);

	XYCurveDock::setModel();
}

//*************************************************************
//******** SLOTs for changes triggered in the dock  ***********
//*************************************************************
void XYAnalysisCurveDock::dataSourceCurveChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* dataSourceCurve = static_cast<XYCurve*>(index.internalPointer());
	updateSettings(dataSourceCurve->xColumn());

	for (auto* curve : m_analysisCurves)
		curve->setDataSourceCurve(dataSourceCurve);

	enableRecalculate();
}

void XYAnalysisCurveDock::xDataColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* column = static_cast<AbstractColumn*>(index.internalPointer());
	updateSettings(column);

	for (auto* curve : m_analysisCurves)
		curve->setXDataColumn(column);

	enableRecalculate();
}

void XYAnalysisCurveDock::yDataColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* column = static_cast<AbstractColumn*>(index.internalPointer());
	for (auto* curve : m_analysisCurves)
		curve->setYDataColumn(column);

	enableRecalculate();
}

void XYAnalysisCurveDock::y2DataColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* column = static_cast<AbstractColumn*>(index.internalPointer());
	for (auto* curve : m_analysisCurves)
		curve->setY2DataColumn(column);

	enableRecalculate();
}

void XYAnalysisCurveDock::enableRecalculate() const {
	// enable the recalculate button if all required data source columns were provided, disable otherwise
	bool hasSourceData = false;
	if (m_analysisCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet) {
		AbstractAspect* aspectX = nullptr;
		AbstractAspect* aspectY = nullptr;
		AbstractAspect* aspectY2 = nullptr;
		switch (m_requiredDataSource) {
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

		if (aspectX)
			cbXDataColumn->setInvalid(false);

		if (aspectY)
			cbYDataColumn->setInvalid(false);

		if (aspectY2)
			cbY2DataColumn->setInvalid(false);
	} else
		hasSourceData = (m_analysisCurve->dataSourceCurve() != nullptr);

	m_recalculateButton->setEnabled(hasSourceData);
}

//**************************************************************
//***** SLOTs for changes triggered in the analysis curve ******
//**************************************************************
void XYAnalysisCurveDock::curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType type) {
	CONDITIONAL_LOCK_RETURN;
	cbDataSourceType->setCurrentIndex(static_cast<int>(type));
	enableRecalculate();
}

void XYAnalysisCurveDock::curveDataSourceCurveChanged(const XYCurve* curve) {
	CONDITIONAL_LOCK_RETURN;
	cbDataSourceCurve->setAspect(curve);
	enableRecalculate();
}

void XYAnalysisCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbXDataColumn->setAspect(column, m_analysisCurve->xDataColumnPath());
	enableRecalculate();
}

void XYAnalysisCurveDock::curveYDataColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbYDataColumn->setAspect(column, m_analysisCurve->yDataColumnPath());
	enableRecalculate();
}
