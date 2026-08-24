/*
	File                 : Surface3DScene.h
	Project              : LabPlot
	Description          : 3D Surface Plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2024 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SURFACE3DSCENE_H
#define SURFACE3DSCENE_H

#include "Base3DPlot.h"

#include <QtGraphsWidgets/Q3DSurfaceWidgetItem>

class AbstractColumn;
class Matrix;
class Surface3DScenePrivate;

class Surface3DScene : public Base3DPlot {
	Q_OBJECT
public:
	Surface3DScene(const QString& name);
	~Surface3DScene() override;
	enum DataSource { DataSource_Spreadsheet = 0, DataSource_Matrix = 1, DataSource_Empty = 2 };
	enum DrawMode { DrawWireframe = 1, DrawSurface = 2, DrawWireframeSurface = 3 };

	BASIC_D_ACCESSOR_DECL(DataSource, dataSource, DataSource)
	BASIC_D_ACCESSOR_DECL(DrawMode, drawMode, DrawMode)
	BASIC_D_ACCESSOR_DECL(bool, flatShading, FlatShading)
	BASIC_D_ACCESSOR_DECL(bool, smooth, Smooth)
	BASIC_D_ACCESSOR_DECL(QColor, color, Color)

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void init(bool transform = true);
	void finalizeAdd() override;
	void finalizeLoad();
	void retransform() override;

	// Matrix parameters
	POINTER_D_ACCESSOR_DECL(const Matrix, matrix, Matrix)
	const QString& matrixPath() const;

	// Spreadsheet parameters
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, xColumn, XColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, yColumn, YColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, zColumn, ZColumn)

	const QString& xColumnPath() const;
	const QString& yColumnPath() const;
	const QString& zColumnPath() const;

	void show(bool visible);
	void recalc();

	QMenu* createContextMenu() override;

	typedef Surface3DScenePrivate Private;

public Q_SLOTS:
	void addPlot();

private:
	Q_DECLARE_PRIVATE(Surface3DScene)
	void initMenus();
	void handleChildAdded(const AbstractAspect*);
	QMenu* m_addNewMenu{nullptr};
	bool m_menusInitialized{false};

private Q_SLOTS:
	// Spreadsheet slots
	void xColumnAboutToBeRemoved(const AbstractAspect*);
	void yColumnAboutToBeRemoved(const AbstractAspect*);
	void zColumnAboutToBeRemoved(const AbstractAspect*);

	// Matrix slots
	void matrixAboutToBeRemoved(const AbstractAspect*);

Q_SIGNALS:
	void sourceTypeChanged(Surface3DScene::DataSource);
	void drawModeChanged(Surface3DScene::DrawMode);
	void flatShadingChanged(bool);
	void smoothChanged(bool);
	void colorChanged(QColor);
	void matrixChanged(const Matrix*);
	void xColumnChanged(const AbstractColumn*);
	void yColumnChanged(const AbstractColumn*);
	void zColumnChanged(const AbstractColumn*);
	void dataChanged();
};
#endif // SURFACE3DSCENE_H
