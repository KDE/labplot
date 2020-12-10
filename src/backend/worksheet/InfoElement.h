/***************************************************************************
	File                 : InfoElement.h
	Project              : LabPlot
	Description          : Marker which can highlight points of curves and
						   show their values
	--------------------------------------------------------------------
    Copyright            : (C) 2020 Martin Marmsoler (martin.marmsoler@gmail.com)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/


#ifndef INFOELEMENT_H
#define INFOELEMENT_H

#include "WorksheetElement.h"
#include "backend/worksheet/TextLabel.h"

class TextLabel;
class CustomPoint;
class CartesianPlot;
class InfoElementPrivate;
class QGraphicsItem;
class XYCurve;
class QAction;
class QMenu;

class InfoElement : public WorksheetElement {
	Q_OBJECT

public:
	InfoElement(const QString& name, CartesianPlot*);
	InfoElement(const QString& name, CartesianPlot*, const XYCurve*, double pos);
	void setParentGraphicsItem(QGraphicsItem* item);
	~InfoElement();

	struct MarkerPoints_T {
		MarkerPoints_T() = default;
		MarkerPoints_T(CustomPoint* custompoint, QString customPointPath, const XYCurve* curve, QString curvePath):
			customPoint(custompoint), customPointPath(customPointPath), curve(curve), curvePath(curvePath) {}
		CustomPoint* customPoint{nullptr};
		QString customPointPath{""};
		const XYCurve* curve{nullptr};
		QString curvePath{""};
        // x and y values are needed, because the columns of the curves of different Markerpoints might do not have the same
		// length
		double x{0}; // x Value
		double y{0}; // y Value
	};

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig&) override;

	void init();
	void initActions();
	void initMenus();
	void addCurve(const XYCurve*, CustomPoint* = nullptr);
	void addCurvePath(QString& curvePath, CustomPoint* = nullptr);
	bool assignCurve(const QVector<XYCurve*>&);
	void removeCurve(const XYCurve*);
	void setZValue(qreal) override;
	int markerPointsCount();
	MarkerPoints_T markerPointAt(int index);
	int gluePointsCount();
	TextLabel::GluePoint gluePoint(int index);
	TextLabel::TextWrapper createTextLabelText();
	QMenu* createContextMenu() override;
	CartesianPlot* plot();
	bool isTextLabel() const;
	double setMarkerpointPosition(double x);
	int currentIndex(double new_x, double* found_x=nullptr);

	QGraphicsItem* graphicsItem() const override;
	void setPrinting(bool on) override;

	void retransform() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;

    BASIC_D_ACCESSOR_DECL(double, position, Position);
	BASIC_D_ACCESSOR_DECL(int, gluePointIndex, GluePointIndex);
	BASIC_D_ACCESSOR_DECL(QString, connectionLineCurveName, ConnectionLineCurveName);
	CLASS_D_ACCESSOR_DECL(QPen, verticalLinePen, VerticalLinePen)
	BASIC_D_ACCESSOR_DECL(qreal, verticalLineOpacity, VerticalLineOpacity)
	CLASS_D_ACCESSOR_DECL(QPen, connectionLinePen, ConnectionLinePen)
	BASIC_D_ACCESSOR_DECL(qreal, connectionLineOpacity, ConnectionLineOpacity)

	void setVisible(bool on) override;
	bool isVisible() const override;

	typedef  InfoElementPrivate Private;

public slots:
	void labelPositionChanged(TextLabel::PositionWrapper);
	void labelVisibleChanged(bool);
	void pointPositionChanged(QPointF);
	void childRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child);
	void childAdded(const AbstractAspect*);
	void labelBorderShapeChanged();
	void labelTextWrapperChanged(TextLabel::TextWrapper);
	void moveElementBegin();
	void moveElementEnd();
	void curveVisibilityChanged();

protected:
	InfoElementPrivate* const d_ptr;

private:
	Q_DECLARE_PRIVATE(InfoElement)
	TextLabel* label{nullptr};
	QVector<struct MarkerPoints_T> markerpoints;
	bool m_menusInitialized {false};
	bool m_suppressChildRemoved {false};
	bool m_suppressChildPositionChanged {false};
	bool m_setTextLabelText{false};
	/*!
		* This variable is set when a curve is moved in the order, because there
		* the curve is removed and readded and we would like to ignore this remove and
		* add. Because of single thread it makes no problems.
		*/
	bool m_curveGetsMoved{false};

	// Actions
	QAction* visibilityAction;

signals:
	void visibleChanged(const bool);
	void gluePointIndexChanged(const int);
	void connectionLineCurveNameChanged(const QString&);
	void positionChanged(const double);
	void labelBorderShapeChangedSignal();
	void curveRemoved(const QString&);
	void verticalLinePenChanged(const QPen&);
	void verticalLineOpacityChanged(qreal);
	void connectionLinePenChanged(const QPen&);
	void connectionLineOpacityChanged(qreal);
};

#endif // INFOELEMENT_H
