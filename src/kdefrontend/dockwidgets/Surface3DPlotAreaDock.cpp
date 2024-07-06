#include "Surface3DPlotAreaDock.h"
#include "TreeViewComboBox.h"
#include <backend/core/AspectTreeModel.h>
#include <kdefrontend/TemplateHandler.h>
class TreeViewComboBox;
Surface3DPlotAreaDock::Surface3DPlotAreaDock(QWidget* parent)
    : BaseDock(parent)
	, aspectTreeModel(nullptr)
    , m_initializing(false) {
    ui.setupUi(this);

    this->retranslateUi();

    QList<AspectType> list;
    list << AspectType::Folder << AspectType::Workbook << AspectType::Spreadsheet << AspectType::Column << AspectType::Worksheet;

    const QVector<TreeViewComboBox*> treeViews(QVector<TreeViewComboBox*>()
                                               << ui.cbXCoordinate << ui.cbYCoordinate << ui.cbZCoordinate << ui.cbNode1 << ui.cbNode2 << ui.cbNode3);

    for (TreeViewComboBox* view : treeViews) {
        view->setTopLevelClasses(list);
    }

    list.clear();
    list << AspectType::Column;

    for (TreeViewComboBox* view : treeViews) {
        connect(view, SIGNAL(currentModelIndexChanged(const QModelIndex&)), SLOT(onTreeViewIndexChanged(const QModelIndex&)));
    }

    // // Matrix data source
    // list.clear();
    // list << AspectType::Folder << AspectType::Workbook << AspectType::Matrix;
    // ui.cbMatrix->setTopLevelClasses(list);

    // list.clear();
    // list << AspectType::Matrix;
    // 	ui.cbMatrix->setSelectableClasses(list);

    // SIGNALs/SLOTs
	// General
	connect(ui.leName, SIGNAL(returnPressed()), SLOT(onNameChanged()));
    connect(ui.leComment, SIGNAL(returnPressed()), SLOT(onCommentChanged()));
    connect(ui.cbDataSource, SIGNAL(currentIndexChanged(int)), SLOT(onDataSourceChanged(int)));
    // connect(ui.cbMatrix, SIGNAL(currentModelIndexChanged(const QModelIndex&)), SLOT(onTreeViewIndexChanged(const QModelIndex&)));

    connect(ui.chkVisible, SIGNAL(toggled(bool)), SLOT(onVisibilityChanged(bool)));

	// Mesh
	connect(ui.cbMeshType, SIGNAL(currentIndexChanged(int)), SLOT(onMeshTypeChanged(int)));
	connect(ui.cbDrawType, SIGNAL(currentIndexChanged(int)), SLOT(onDrawModeChanged(int)));
	connect(ui.chkFlatShading, SIGNAL(toggled(bool)), SLOT(onFlatShadingChanged(bool)));
	connect(ui.chkGridVisible, SIGNAL(toggled(bool)), SLOT(onGridVisibleChanged(bool)));
	connect(ui.cbShadowQuality, SIGNAL(currentIndexChanged(int)), SLOT(onShadowQualityChanged(int)));
}

void Surface3DPlotAreaDock::setSurfaces(const QList<Surface3DPlotArea*>& surfaces) {
    this->surfaces = surfaces;
	if (surfaces.size() == 1) {
		ui.leName->setText(surfaces.first()->name());
		ui.leComment->setText(surfaces.first()->comment());
		ui.leName->setEnabled(true);
		ui.leComment->setEnabled(true);
		ui.chkVisible->setEnabled(true);
		ui.chkVisible->setChecked(surfaces.first()->isVisible());
	} else {
		ui.leName->setEnabled(false);
        ui.leComment->setEnabled(false);
        ui.chkVisible->setEnabled(false);
    }

    aspectTreeModel = new AspectTreeModel(surfaces.first()->parentAspect());
    ui.cbXCoordinate->setModel(aspectTreeModel);
    ui.cbYCoordinate->setModel(aspectTreeModel);
    ui.cbZCoordinate->setModel(aspectTreeModel);
    ui.cbNode1->setModel(aspectTreeModel);
    ui.cbNode2->setModel(aspectTreeModel);
    ui.cbNode3->setModel(aspectTreeModel);
    ui.cbMatrix->setModel(aspectTreeModel);

	for (Surface3DPlotArea* surface : surfaces) {
		meshTypeChanged(surface->meshType());
		drawModeChanged(surface->drawMode());
		sourceTypeChanged(surface->dataSource());

		matrixChanged(surface->matrix());

		xColumnChanged(surface->xColumn());
        yColumnChanged(surface->yColumn());
        zColumnChanged(surface->zColumn());

        firstNodeChanged(surface->firstNode());
        secondNodeChanged(surface->secondNode());
        thirdNodeChanged(surface->thirdNode());
    }

    for (Surface3DPlotArea* surface : surfaces) {
        connect(surface, &Surface3DPlotArea::drawModeChanged, this, &Surface3DPlotAreaDock::drawModeChanged);
        connect(surface, &Surface3DPlotArea::sourceTypeChanged, this, &Surface3DPlotAreaDock::sourceTypeChanged);
        connect(surface, &Surface3DPlotArea::meshTypeChanged, this, &Surface3DPlotAreaDock::meshTypeChanged);
        connect(surface, &Surface3DPlotArea::flatShadingChanged, this, &Surface3DPlotAreaDock::flatShadingChanged);
        connect(surface, &Surface3DPlotArea::gridVisibilityChanged, this, &Surface3DPlotAreaDock::gridVisibleChanged);
        connect(surface, &Surface3DPlotArea::shadowQualityChanged, this, &Surface3DPlotAreaDock::shadowsQualityChanged);
        connect(surface, &Surface3DPlotArea::smoothChanged, this, &Surface3DPlotAreaDock::onSmoothChanged);

        connect(surface, &Surface3DPlotArea::matrixChanged, this, &Surface3DPlotAreaDock::matrixChanged);
        connect(surface, &Surface3DPlotArea::xColumnChanged, this, &Surface3DPlotAreaDock::xColumnChanged);
        connect(surface, &Surface3DPlotArea::yColumnChanged, this, &Surface3DPlotAreaDock::yColumnChanged);
        connect(surface, &Surface3DPlotArea::zColumnChanged, this, &Surface3DPlotAreaDock::zColumnChanged);
        connect(surface, &Surface3DPlotArea::firstNodeChanged, this, &Surface3DPlotAreaDock::firstNodeChanged);
        connect(surface, &Surface3DPlotArea::secondNodeChanged, this, &Surface3DPlotAreaDock::secondNodeChanged);
        connect(surface, &Surface3DPlotArea::thirdNodeChanged, this, &Surface3DPlotAreaDock::thirdNodeChanged);
    }
    updateUiVisibility();
}
void Surface3DPlotAreaDock::showItem(QWidget* label, QWidget* comboBox, bool pred) {
	label->setVisible(pred);
	comboBox->setVisible(pred);
}

void Surface3DPlotAreaDock::showTriangleInfo(bool pred) {
	if (m_initializing)
		return;

	showItem(ui.labelX, ui.cbXCoordinate, pred);
	showItem(ui.labelY, ui.cbYCoordinate, pred);
	showItem(ui.labelZ, ui.cbZCoordinate, pred);
	showItem(ui.labelNode1, ui.cbNode1, pred);
	showItem(ui.labelNode2, ui.cbNode2, pred);
	showItem(ui.labelNode3, ui.cbNode3, pred);
	ui.labelNodeHeader->setVisible(pred);
	Q_EMIT elementVisibilityChanged();
}

//*************************************************************
//****** SLOTs for changes triggered in Surface3DDock *********
//*************************************************************
void Surface3DPlotAreaDock::retranslateUi() {
    ui.cbDataSource->insertItem(Surface3DPlotArea::DataSource_Spreadsheet, i18n("Spreadsheet"));
    ui.cbDataSource->insertItem(Surface3DPlotArea::DataSource_Matrix, i18n("Matrix"));
    ui.cbDataSource->insertItem(Surface3DPlotArea::DataSource_Empty, i18n("Demo"));

    ui.cbDrawType->insertItem(Surface3DPlotArea::DrawSurface, i18n("Surface"));
	ui.cbDrawType->insertItem(Surface3DPlotArea::DrawWireframe, i18n("Wireframe"));
	// Mesh Type options
	ui.cbMeshType->insertItem(Surface3DPlotArea::UserDefined, i18n("User Defined"));
	ui.cbMeshType->insertItem(Surface3DPlotArea::Bar, i18n("Bar"));
	ui.cbMeshType->insertItem(Surface3DPlotArea::Cube, i18n("Cube"));
	ui.cbMeshType->insertItem(Surface3DPlotArea::Pyramid, i18n("Pyramid"));
	ui.cbMeshType->insertItem(Surface3DPlotArea::Cone, i18n("Cone"));
	ui.cbMeshType->insertItem(Surface3DPlotArea::Cylinder, i18n("Cylinder"));
	ui.cbMeshType->insertItem(Surface3DPlotArea::BevelBar, i18n("Bevel Bar"));
	ui.cbMeshType->insertItem(Surface3DPlotArea::BevelCube, i18n("Bevel Cube"));
	ui.cbMeshType->insertItem(Surface3DPlotArea::Sphere, i18n("Sphere"));
	ui.cbMeshType->insertItem(Surface3DPlotArea::Minimal, i18n("Minimal"));
	ui.cbMeshType->insertItem(Surface3DPlotArea::Arrow, i18n("Arrow"));
	ui.cbMeshType->insertItem(Surface3DPlotArea::Point, i18n("Point"));

	ui.cbShadowQuality->insertItem(Surface3DPlotArea::None, i18n("None"));
	ui.cbShadowQuality->insertItem(Surface3DPlotArea::Low, i18n("Low"));
	ui.cbShadowQuality->insertItem(Surface3DPlotArea::Medium, i18n("Medium"));
	ui.cbShadowQuality->insertItem(Surface3DPlotArea::High, i18n("High"));
	ui.cbShadowQuality->insertItem(Surface3DPlotArea::SoftLow, i18n("SoftLow"));
	ui.cbShadowQuality->insertItem(Surface3DPlotArea::SoftMedium, i18n("SoftMedium"));
	ui.cbShadowQuality->insertItem(Surface3DPlotArea::SoftHigh, i18n("SoftHigh"));
}
const AbstractColumn* Surface3DPlotAreaDock::getColumn(const QModelIndex& index) const {
	if (!index.isValid())
		return nullptr;

	// Assuming the model is an AspectTreeModel or compatible
	auto* aspect = static_cast<AbstractColumn*>(index.internalPointer());
	return dynamic_cast<const AbstractColumn*>(aspect);
}

const Matrix* Surface3DPlotAreaDock::getMatrix(const QModelIndex& index) const {
	if (!index.isValid())
		return nullptr;

	// Assuming the model is an AspectTreeModel or compatible
	auto* aspect = static_cast<Matrix*>(index.internalPointer());
	return dynamic_cast<const Matrix*>(aspect);
}

void Surface3DPlotAreaDock::onTreeViewIndexChanged(const QModelIndex& index) {
	const AbstractColumn* column = getColumn(index);

	QObject* senderW = sender();
	const Lock lock(m_initializing);
	for (Surface3DPlotArea* surface : surfaces) {
		if (senderW == ui.cbXCoordinate)
			surface->setXColumn(column);
		else if (senderW == ui.cbYCoordinate)
			surface->setYColumn(column);
		else if (senderW == ui.cbZCoordinate)
			surface->setZColumn(column);
		else if (senderW == ui.cbNode1)
			surface->setFirstNode(column);
		else if (senderW == ui.cbNode2)
			surface->setSecondNode(column);
		else if (senderW == ui.cbNode3)
			surface->setThirdNode(column);
		else if (senderW == ui.cbMatrix)
			surface->setMatrix(getMatrix(index));
	}
}

void Surface3DPlotAreaDock::onVisibilityChanged(bool visible) {
	const Lock lock(m_initializing);
	for (Surface3DPlotArea* surface : surfaces) {
		surface->show(visible);
	}
}

void Surface3DPlotAreaDock::onDataSourceChanged(int index) {
    qDebug() << Q_FUNC_INFO;
    if (m_initializing)
        return;
    const Surface3DPlotArea::DataSource type = static_cast<Surface3DPlotArea::DataSource>(index);
    qDebug() << index;
    const Lock lock(m_initializing);
    for (Surface3DPlotArea* surface : surfaces)
        surface->setDataSource(type);
    updateUiVisibility();
}

void Surface3DPlotAreaDock::updateUiVisibility() {
    const int type = ui.cbDrawType->currentIndex();
    const int dataType = ui.cbDataSource->currentIndex();
    if (type == Surface3DPlotArea::DrawMode::DrawSurface || type == Surface3DPlotArea::DrawMode::DrawWireframe) {
        showItem(ui.labelSource, ui.cbDataSource, true);
        showItem(ui.labelMatrix, ui.cbMatrix, false);
		showTriangleInfo(dataType == Surface3DPlotArea::DataSource_Spreadsheet);
    } else {
        showItem(ui.labelSource, ui.cbDataSource, false);
		showTriangleInfo(false);
	}

	Q_EMIT elementVisibilityChanged();
}

void Surface3DPlotAreaDock::onDrawModeChanged(int index) {
	{
		const Lock lock(m_initializing);
		for (Surface3DPlotArea* surface : surfaces)
			surface->setDrawMode(static_cast<Surface3DPlotArea::DrawMode>(index));
	}

    updateUiVisibility();
}

void Surface3DPlotAreaDock::onFlatShadingChanged(bool vis) {
    qDebug() << Q_FUNC_INFO;
	{
		const Lock lock(m_initializing);
		for (Surface3DPlotArea* surface : surfaces)
			surface->setFlatShading(vis);
	}
}

void Surface3DPlotAreaDock::onGridVisibleChanged(bool vis) {
	{
		const Lock lock(m_initializing);
		for (Surface3DPlotArea* surface : surfaces)
			surface->setGridVisibility(vis);
	}
}

void Surface3DPlotAreaDock::onShadowQualityChanged(int index) {
	{
		const Lock lock(m_initializing);
		for (Surface3DPlotArea* surface : surfaces)
			surface->setShadowQuality(static_cast<Surface3DPlotArea::ShadowQuality>(index));
	}
}

void Surface3DPlotAreaDock::onSmoothChanged(bool vis) {
	{
		const Lock lock(m_initializing);
		for (Surface3DPlotArea* surface : surfaces)
			surface->setSmooth(vis);
	}
}

void Surface3DPlotAreaDock::drawModeChanged(Surface3DPlotArea::DrawMode mode) {
	if (m_initializing)
		return;
	ui.cbDrawType->setCurrentIndex(mode);
    updateUiVisibility();
}

void Surface3DPlotAreaDock::meshTypeChanged(Surface3DPlotArea::MeshType type) {
	if (m_initializing)
		return;
	ui.cbMeshType->setCurrentIndex(type);
}

void Surface3DPlotAreaDock::flatShadingChanged(bool vis) {
	if (m_initializing)
		return;
    ui.chkFlatShading->setEnabled(vis);
}

void Surface3DPlotAreaDock::gridVisibleChanged(bool vis) {
	if (m_initializing)
		return;
	ui.chkGridVisible->setEnabled(vis);
}

void Surface3DPlotAreaDock::shadowsQualityChanged(Surface3DPlotArea::ShadowQuality quality) {
	if (m_initializing)
		return;
	ui.cbShadowQuality->setCurrentIndex(quality);
}

void Surface3DPlotAreaDock::sourceTypeChanged(Surface3DPlotArea::DataSource type) {
    qDebug() << Q_FUNC_INFO;
	if (m_initializing)
		return;
	ui.cbDataSource->setCurrentIndex(type);
}
QModelIndex Surface3DPlotAreaDock::modelIndexOfAspect(AspectTreeModel* model, const AbstractAspect* aspect) const {
	if (!model || !aspect)
		return QModelIndex();

	const QList<QModelIndex> indexes = model->match(model->index(0, 0), Qt::DisplayRole, QVariant::fromValue(aspect), 1, Qt::MatchExactly | Qt::MatchRecursive);

	return indexes.isEmpty() ? QModelIndex() : indexes.first();
}
void Surface3DPlotAreaDock::setModelFromAspect(TreeViewComboBox* cb, const AbstractAspect* aspect) {
	if (m_initializing)
		return;
	cb->setCurrentModelIndex(modelIndexOfAspect(aspectTreeModel, aspect));
}

void Surface3DPlotAreaDock::matrixChanged(const Matrix* matrix) {
	setModelFromAspect(ui.cbMatrix, matrix);
}

void Surface3DPlotAreaDock::xColumnChanged(const AbstractColumn* column) {
	setModelFromAspect(ui.cbXCoordinate, column);
}

void Surface3DPlotAreaDock::yColumnChanged(const AbstractColumn* column) {
	setModelFromAspect(ui.cbYCoordinate, column);
}

void Surface3DPlotAreaDock::zColumnChanged(const AbstractColumn* column) {
	setModelFromAspect(ui.cbZCoordinate, column);
}

void Surface3DPlotAreaDock::firstNodeChanged(const AbstractColumn* column) {
	setModelFromAspect(ui.cbNode1, column);
}

void Surface3DPlotAreaDock::secondNodeChanged(const AbstractColumn* column) {
	setModelFromAspect(ui.cbNode2, column);
}

void Surface3DPlotAreaDock::thirdNodeChanged(const AbstractColumn* column) {
	setModelFromAspect(ui.cbNode3, column);
}

void Surface3DPlotAreaDock::onMeshTypeChanged(int index) {
    {
        const Lock lock(m_initializing);
        for (Surface3DPlotArea* surface : surfaces)
            surface->setMeshType(static_cast<Surface3DPlotArea::MeshType>(index));
    }
}

void Surface3DPlotAreaDock::onNameChanged() {
	const Lock lock(m_initializing);
	surfaces.first()->setName(ui.leName->text());
}

void Surface3DPlotAreaDock::onCommentChanged() {
	const Lock lock(m_initializing);
	surfaces.first()->setComment(ui.leComment->text());
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void Surface3DPlotAreaDock::load() {
	// TODO
}

void Surface3DPlotAreaDock::loadConfig(KConfig& config) {
	// TODO
}
