/*
	File                 : InfoElement.h
	Project              : LabPlot
	Description          : Marker which can highlight points of curves and
						   show their values
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2020-2025 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INFOELEMENT_H
#define INFOELEMENT_H

#include "WorksheetElement.h"
#include "backend/worksheet/TextLabel.h"

class CartesianPlot;
class CustomPoint;
class InfoElementPrivate;
class Line;
class QGraphicsItem;
class QMenu;
class XYCurve;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT InfoElement : public WorksheetElement {
#else
class InfoElement : public WorksheetElement {
#endif
	Q_OBJECT

public:
	InfoElement(const QString& name, CartesianPlot*);
	InfoElement(const QString& name, CartesianPlot*, const XYCurve*, double logicalPos);
	virtual void setParentGraphicsItem(QGraphicsItem*) override;
	~InfoElement();

	struct MarkerPoints_T {
		MarkerPoints_T() = default;
		MarkerPoints_T(CustomPoint*, const XYCurve*, const QString& curvePath);
		CustomPoint* customPoint{nullptr};
		const XYCurve* curve{nullptr};
		QString curvePath;
		bool visible{true};
	};

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig&) override;

	TextLabel* title();
	void addCurve(const XYCurve*, CustomPoint* = nullptr);
	void addCurvePath(const QString& curvePath, CustomPoint* = nullptr);
	bool assignCurve(const QVector<XYCurve*>&);
	void removeCurve(const XYCurve*);
	void curveDeleted(const AbstractAspect*);
	void setZValue(qreal) override;
	int markerPointsCount() const;
	MarkerPoints_T markerPointAt(int index) const;
	int gluePointsCount() const;
	TextLabel::GluePoint gluePoint(int index) const;
	TextLabel::TextWrapper createTextLabelText();
	QMenu* createContextMenu() override;

	bool isTextLabel() const;
	double setMarkerpointPosition(double x);
	int currentIndex(double new_x, double* found_x = nullptr) const;

	void retransform() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;

	BASIC_D_ACCESSOR_DECL(double, positionLogical, PositionLogical)
	BASIC_D_ACCESSOR_DECL(int, gluePointIndex, GluePointIndex)
	CLASS_D_ACCESSOR_DECL(QString, connectionLineCurveName, ConnectionLineCurveName)
	Line* verticalLine() const;
	Line* connectionLine() const;
	bool isValid() const;

	virtual void setVisible(bool on) override;

	typedef InfoElementPrivate Private;

public Q_SLOTS:
	void labelPositionChanged(TextLabel::PositionWrapper);
	void labelVisibleChanged(bool);
	void pointPositionChanged(const PositionWrapper&);
	void childRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child);
	void childAdded(const AbstractAspect*);
	void labelBorderShapeChanged();
	void labelTextWrapperChanged(TextLabel::TextWrapper);
	void curveVisibilityChanged();
	void curveDataChanged();
	void curveCoordinateSystemIndexChanged(int);
	void pointVisibleChanged(bool visible);

private:
	Q_DECLARE_PRIVATE(InfoElement)
	TextLabel* m_title{nullptr};
	bool m_suppressVisibleChange{false};
	QVector<struct MarkerPoints_T> markerpoints;
	bool m_menusInitialized{false};
	bool m_suppressChildRemoved{false};
	bool m_suppressChildPositionChanged{false};
	bool m_setTextLabelText{false};

	void init();
	void initActions();
	void initMenus();
	void initCurveConnections(const XYCurve*);
	void initCustomPointConnections(const CustomPoint*);
	void updateValid();
	void setConnectionLineNextValidCurve();
	virtual void handleAspectUpdated(const QString& path, const AbstractAspect*) override;
	void loadPoints(XmlStreamReader* reader, bool preview);

Q_SIGNALS:
	void gluePointIndexChanged(const int);
	void connectionLineCurveNameChanged(const QString&);
	void positionLogicalChanged(const double);
	void labelBorderShapeChangedSignal();
	void curveRemoved(const QString&);
};

#endif // INFOELEMENT_H
