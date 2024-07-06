#ifndef SURFACE3DPLOTAREADOCK_H
#define SURFACE3DPLOTAREADOCK_H
#include "backend/worksheet/plots/3d/Surface3DPlotArea.h"
#include <QWidget>
#include <TreeViewComboBox.h>

#include "BaseDock.h"
#include "ui_surface3dplotareadock.h"

class Surface3DPlotArea;
class Matrix;
class AbstractColumn;
class AspectTreeModel;
class ColorMapSelector;
class BaseDock;

class Surface3DPlotAreaDock : public BaseDock {
	Q_OBJECT

public:
    explicit Surface3DPlotAreaDock(QWidget* parent);
    void setSurfaces(const QList<Surface3DPlotArea*>& surfaces);

private:
	void showTriangleInfo(bool pred);
	void showItem(QWidget* label, QWidget* comboBox, bool pred);

	void setModelFromAspect(TreeViewComboBox* cb, const AbstractAspect* aspect);
	void updateUiVisibility();
	const AbstractColumn* getColumn(const QModelIndex& index) const;
	const Matrix* getMatrix(const QModelIndex& index) const;

private Q_SLOTS:
	void retranslateUi();

	// SLOTs for changes triggered in Surface3DDock
	void onNameChanged();
	void onCommentChanged();

	void onTreeViewIndexChanged(const QModelIndex&);
	void onDataSourceChanged(int);
	void onVisibilityChanged(bool);

	// Surface 3D
	void sourceTypeChanged(Surface3DPlotArea::DataSource);

	// Matrix handling
	void matrixChanged(const Matrix*);

	// Spreadsheet handling
	void xColumnChanged(const AbstractColumn*);
	void yColumnChanged(const AbstractColumn*);
	void zColumnChanged(const AbstractColumn*);
	void firstNodeChanged(const AbstractColumn*);
	void secondNodeChanged(const AbstractColumn*);
	void thirdNodeChanged(const AbstractColumn*);

	// Appearance properties
	void onMeshTypeChanged(int);
	void onDrawModeChanged(int);
	void onShadowQualityChanged(int);
	void onFlatShadingChanged(bool);
	void onGridVisibleChanged(bool);

	// Rendering properties
	void onSmoothChanged(bool);

	void drawModeChanged(Surface3DPlotArea::DrawMode mode);
	void meshTypeChanged(Surface3DPlotArea::MeshType type);
	void flatShadingChanged(bool);
	void gridVisibleChanged(bool);
	void shadowsQualityChanged(Surface3DPlotArea::ShadowQuality quality);

private:
	Ui::Surface3DPlotAreaDock ui;
    QList<Surface3DPlotArea*> surfaces;
	AspectTreeModel* aspectTreeModel;
    bool m_initializing;

	void load();
    void loadConfig(KConfig&);
    QModelIndex modelIndexOfAspect(AspectTreeModel* model, const AbstractAspect* aspect) const;

Q_SIGNALS:
    void info(const QString&);
    void elementVisibilityChanged();
};

#endif // SURFACE3DPLOTAREADOCK_H
