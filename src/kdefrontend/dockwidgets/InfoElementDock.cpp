#include "InfoElementDock.h"

#include "backend/worksheet/InfoElement.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "ui_infoelementdock.h"

InfoElementDock::InfoElementDock(QWidget *parent) :
	BaseDock(parent),
	ui(new Ui::InfoElementDock) {
	ui->setupUi(this);

	connect(ui->chbVisible, &QCheckBox::toggled, this, &InfoElementDock::visibilityChanged);
	connect(ui->btnAddCurve, &QPushButton::clicked, this, &InfoElementDock::addCurve);
	connect(ui->btnRemoveCurve, &QPushButton::clicked, this, &InfoElementDock::removeCurve);
	connect(ui->sbXPosLineWidth, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &InfoElementDock::xposLineWidthChanged);
	connect(ui->sbConnectionLineWidth, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &InfoElementDock::connectionLineWidthChanged);
	// From Qt 5.7 qOverload can be used:
	//connect(ui->sbXPosLineWidth, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &InfoElementDock::xposLineWidthChanged);
	//connect(ui->sbConnectionLineWidth, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &InfoElementDock::connectionLineWidthChanged);
	connect(ui->kcbXPosLineColor, &KColorButton::changed, this, &InfoElementDock::xposLineColorChanged);
	connect(ui->kcbConnectionLineColor, &KColorButton::changed, this, &InfoElementDock::connectionLineColorChanged);
	connect(ui->chbXPosLineVisible, &QCheckBox::toggled, this, &InfoElementDock::xposLineVisibilityChanged);
	connect(ui->cbConnnectionLineVisible, &QCheckBox::toggled, this, &InfoElementDock::connectionLineVisibilityChanged);

	connect(ui->cb_gluePoint, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InfoElementDock::gluePointChanged);
	connect(ui->cb_curve, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InfoElementDock::curveChanged);
}

void InfoElementDock::setInfoElements(QList<InfoElement *> &list, bool sameParent) {
	const Lock lock(m_initializing);

	m_elements = list;
	m_element = list.first();
	m_sameParent = sameParent;

	ui->lstAvailableCurves->clear();
	ui->lstSelectedCurves->clear();
	ui->cb_curve->clear();

	ui->chbVisible->setChecked(m_element->isVisible());

	// disable if not all worksheetelements do not have the same parent (different CartesianPlots),
	// because then the available curves are different
	if (sameParent) {
		QVector<XYCurve*> curves = m_element->plot()->children<XYCurve>();
		for (int i=0; i< curves.length(); i++)
			ui->lstAvailableCurves->addItem(curves[i]->name());

		for (int i=0; i<m_element->markerPointsCount(); i++)
			if (m_element->markerPointAt(i).curve != nullptr) {
				ui->lstSelectedCurves->addItem(m_element->markerPointAt(i).curve->name());
				ui->cb_curve->addItem(m_element->markerPointAt(i).curve->name());
			}

	} else {
		ui->lstAvailableCurves->setEnabled(false);
		ui->lstSelectedCurves->setEnabled(false);
	}
	QString curveName = m_element->connectionLineCurveName();
	for (int i=0; i< ui->cb_curve->count(); i++) {
		if (ui->cb_curve->itemData(i, Qt::DisplayRole).toString().compare(curveName) == 0) {
			ui->cb_curve->setCurrentIndex(i);
			break;
		}
	}

	if (m_element->isTextLabel()) {
        elementLabelBorderShapeChanged();
		ui->cb_gluePoint->setCurrentIndex(m_element->gluePointIndex()+1);
	}

	ui->chbXPosLineVisible->setChecked(m_element->xposLineVisible());
	ui->cbConnnectionLineVisible->setChecked(m_element->connectionLineVisible());
	ui->kcbXPosLineColor->setColor(m_element->xposLineColor());
	ui->kcbConnectionLineColor->setColor(m_element->connectionLineColor());
	ui->sbXPosLineWidth->setValue(m_element->xposLineWidth());
	ui->sbConnectionLineWidth->setValue(m_element->connectionLineWidth());

	initConnections();
}

void InfoElementDock::initConnections() {
	connect( m_element, &InfoElement::visibleChanged,
			 this, &InfoElementDock::elementVisibilityChanged );
	connect( m_element, &InfoElement::connectionLineWidthChanged,
			 this, &InfoElementDock::elementConnectionLineWidthChanged );
	connect( m_element, &InfoElement::connectionLineColorChanged,
			 this, &InfoElementDock::elementConnectionLineColorChanged );
	connect( m_element, &InfoElement::xposLineWidthChanged,
			 this, &InfoElementDock::elementXPosLineWidthChanged );
	connect( m_element, &InfoElement::xposLineColorChanged,
			 this, &InfoElementDock::elementXposLineColorChanged );
	connect( m_element, &InfoElement::xposLineVisibleChanged,
			 this, &InfoElementDock::elementXPosLineVisibleChanged );
	connect (m_element, &InfoElement::gluePointIndexChanged,
			 this, &InfoElementDock::elementGluePointIndexChanged );
	connect (m_element, &InfoElement::connectionLineCurveNameChanged,
			 this, &InfoElementDock::elementConnectionLineCurveChanged );
	connect (m_element, &InfoElement::labelBorderShapeChangedSignal,
			 this, &InfoElementDock::elementLabelBorderShapeChanged );
	connect (m_element, &InfoElement::curveRemoved,
			 this, &InfoElementDock::elementCurveRemoved);

}

InfoElementDock::~InfoElementDock() {
	delete ui;
}

void InfoElementDock::visibilityChanged(bool state) {
	if (m_initializing)
		return;

	for (auto* infoElement : m_elements)
		infoElement->setVisible(state);
}

void InfoElementDock::connectionLineWidthChanged(double width) {
	if (m_initializing)
		return;

	for (auto* infoElement: m_elements)
		infoElement->setConnectionLineWidth(width);
}

void InfoElementDock::connectionLineColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	for (auto* infoElement: m_elements)
		infoElement->setConnectionLineColor(color);
}

void InfoElementDock::xposLineWidthChanged(double value) {
	if (m_initializing)
		return;

	for (auto* infoElement: m_elements)
		infoElement->setXPosLineWidth(value);
}

void InfoElementDock::xposLineColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	for (auto* infoElement: m_elements)
		infoElement->setXPosLineColor(color);
}

void InfoElementDock::xposLineVisibilityChanged(bool visible) {
	if (m_initializing)
		return;

	for (auto* infoElement: m_elements)
		infoElement->setXPosLineVisible(visible);
}

void InfoElementDock::connectionLineVisibilityChanged(bool visible) {
	if (m_initializing)
		return;

	for (auto* infoElement: m_elements)
		infoElement->setConnectionLineVisible(visible);
}

void InfoElementDock::gluePointChanged(int index) {
	if (m_initializing)
		return;

	for (auto* infoElement: m_elements)
		infoElement->setGluePointIndex(index - 1); // index 0 means automatic, which is defined as -1
}

void InfoElementDock::curveChanged() {
	if (m_initializing)
		return;

	QString name = ui->cb_curve->currentText();
	for (auto* infoElement: m_elements)
		infoElement->setConnectionLineCurveName(name);
}

void InfoElementDock::addCurve() {

	if (!m_sameParent)
		return;

	QList<QListWidgetItem*> list = ui->lstAvailableCurves->selectedItems();

	bool curveAlreadyExist;
	for (QListWidgetItem* selectedItem: list) {
		QString curveName = selectedItem->data(Qt::DisplayRole).toString();

		curveAlreadyExist = false;
		for (int i=0; i<ui->lstSelectedCurves->count(); i++) {
			QListWidgetItem* item = ui->lstSelectedCurves->item(i);

			if (item->data(Qt::DisplayRole) == selectedItem->data(Qt::DisplayRole)) {
				curveAlreadyExist = true;
				break;
			}
		}
		if (curveAlreadyExist)
			continue;

		XYCurve* curve;
		for (int i=0; i < m_elements[0]->plot()->children<XYCurve>().count(); i++) {
			if (m_elements[0]->plot()->children<XYCurve>()[i]->name() == curveName)
				curve = m_elements[0]->plot()->children<XYCurve>()[i];
		}

		ui->lstSelectedCurves->addItem(selectedItem->data(Qt::DisplayRole).toString());
		for (int i=0; i< list.count(); i++) {
			for (int j=0; j < m_elements.count(); j++) {
				for (int k=0; k < m_elements[j]->markerPointsCount(); k++)
					m_elements[j]->addCurve(curve);
			}
		}
	}

}

void InfoElementDock::removeCurve() {
	if (!m_sameParent)
		return;

	QList<QListWidgetItem*> list = ui->lstSelectedCurves->selectedItems();

	for (auto item: list)
		ui->lstSelectedCurves->takeItem(ui->lstSelectedCurves->row(item));

	for (int i=0; i<list.count(); i++) {
		for (int j=0; j< m_elements.count(); j++) {
			for (int k=0; k < m_elements[j]->markerPointsCount(); k++) {
				if (m_elements[j]->markerPointAt(k).curve->name() == list.at(i)->data(Qt::DisplayRole)) {
					m_elements[j]->removeCurve(m_elements[j]->markerPointAt(k).curve);
					continue;
				}
			}
		}
	}

}

//***********************************************************
//****** SLOTs for changes triggered in InfoElement
//***********************************************************

void InfoElementDock::elementConnectionLineWidthChanged(const double width) {
	const Lock lock(m_initializing);
	ui->sbConnectionLineWidth->setValue(width);
}

void InfoElementDock::elementConnectionLineColorChanged(const QColor& color) {
	const Lock lock(m_initializing);
	ui->kcbConnectionLineColor->setColor(color);
}

void InfoElementDock::elementXPosLineWidthChanged(const double width) {
	const Lock lock(m_initializing);
	ui->sbXPosLineWidth->setValue(width);
}

void InfoElementDock::elementXposLineColorChanged(const QColor& color) {
	const Lock lock(m_initializing);
	ui->kcbXPosLineColor->setColor(color);
}

void InfoElementDock::elementXPosLineVisibleChanged(const bool visible) {
	const Lock lock(m_initializing);
	ui->chbXPosLineVisible->setChecked(visible);
}

void InfoElementDock::elementConnectionLineVisibleChanged(const bool visible) {
	const Lock lock(m_initializing);
	ui->cbConnnectionLineVisible->setChecked(visible);
}

void InfoElementDock::elementVisibilityChanged(const bool visible) {
	const Lock lock(m_initializing);
	ui->chbVisible->setChecked(visible);
}

void InfoElementDock::elementGluePointIndexChanged(const int index) {
	const Lock lock(m_initializing);
	if (index < 0)
		ui->cb_gluePoint->setCurrentIndex(0);
	else
		ui->cb_gluePoint->setCurrentIndex(index + 1); // automatic label is in same combo box
}

void InfoElementDock::elementConnectionLineCurveChanged(const QString name) {
	const Lock lock(m_initializing);
	for (int i=0; i< ui->cb_curve->count(); i++) {
		if (ui->cb_curve->itemData(i).toString().compare(name) == 0) {
			ui->cb_curve->setCurrentIndex(i);
			break;
		}
	}
}

void InfoElementDock::elementLabelBorderShapeChanged() {
	const Lock lock(m_initializing);
	ui->cb_gluePoint->clear();
	ui->cb_gluePoint->addItem("Automatic");
	for (int i=0; i < m_element->gluePointsCount(); i++)
		ui->cb_gluePoint->addItem(m_element->gluePoint(i).name);
}

void InfoElementDock::elementCurveRemoved(QString name) {
	const Lock lock(m_initializing);
	for (int i=0; i < ui->lstSelectedCurves->count(); i++) {
		QListWidgetItem *item = ui->lstSelectedCurves->item(i);
		if (item->data(Qt::DisplayRole).toString().compare(name) == 0) {
			ui->lstSelectedCurves->takeItem(i);
			break;
		}
	}
}
