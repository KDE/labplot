/*
	File                 : ErrorBar.h
	Project              : LabPlot
	Description          : ErrorBar
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ERRORBAR_H
#define ERRORBAR_H

#include "backend/core/AbstractAspect.h"
#include "backend/lib/macros.h"
#include "backend/worksheet/WorksheetElement.h"

class AbstractColumn;
class CartesianCoordinateSystem;
class ErrorBarPrivate;
class ErrorBarStyle;
class Line;
class KConfigGroup;

class ErrorBar : public AbstractAspect {
	Q_OBJECT

public:
	friend class ErrorBarSetXPlusColumnCmd;
	friend class ErrorBarSetXMinusColumnCmd;
	friend class ErrorBarSetYPlusColumnCmd;
	friend class ErrorBarSetYMinusColumnCmd;

	enum class ErrorType { NoError, Symmetric, Asymmetric, Poisson };
	enum class Type { Simple, WithEnds };
	enum class Dimension { Y, XY };

	explicit ErrorBar(const QString& name, Dimension);
	~ErrorBar() override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfigGroup&);
	void loadThemeConfig(const KConfigGroup&, const QColor&);
	void saveThemeConfig(KConfigGroup&) const;

	void init(const KConfigGroup&);
	void update();
	QPainterPath
	painterPath(const QVector<QPointF>&, const CartesianCoordinateSystem*, WorksheetElement::Orientation = WorksheetElement::Orientation::Vertical) const;
	void draw(QPainter*, const QPainterPath&);

	Dimension dimension() const;

	// x
	BASIC_D_ACCESSOR_DECL(ErrorType, xErrorType, XErrorType)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, xPlusColumn, XPlusColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, xMinusColumn, XMinusColumn)
	CLASS_D_ACCESSOR_DECL(QString, xPlusColumnPath, XPlusColumnPath)
	CLASS_D_ACCESSOR_DECL(QString, xMinusColumnPath, XMinusColumnPath)

	// y
	BASIC_D_ACCESSOR_DECL(ErrorType, yErrorType, YErrorType)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, yPlusColumn, YPlusColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, yMinusColumn, YMinusColumn)
	CLASS_D_ACCESSOR_DECL(QString, yPlusColumnPath, YPlusColumnPath)
	CLASS_D_ACCESSOR_DECL(QString, yMinusColumnPath, YMinusColumnPath)

	// styling
	BASIC_D_ACCESSOR_DECL(Type, type, Type)
	BASIC_D_ACCESSOR_DECL(double, capSize, CapSize)
	Line* line() const;

	typedef ErrorBarPrivate Private;

protected:
	ErrorBarPrivate* const d_ptr;

private:
	Q_DECLARE_PRIVATE(ErrorBar)
	void connectXPlusColumn(const AbstractColumn*);
	void connectXMinusColumn(const AbstractColumn*);
	void connectYPlusColumn(const AbstractColumn*);
	void connectYMinusColumn(const AbstractColumn*);

private Q_SLOTS:
	void xPlusColumnAboutToBeRemoved(const AbstractAspect*);
	void xMinusColumnAboutToBeRemoved(const AbstractAspect*);
	void yPlusColumnAboutToBeRemoved(const AbstractAspect*);
	void yMinusColumnAboutToBeRemoved(const AbstractAspect*);

Q_SIGNALS:
	void updateRequested();
	void updatePixmapRequested();

	void xErrorTypeChanged(ErrorBar::ErrorType);
	void xPlusDataChanged();
	void xPlusColumnChanged(const AbstractColumn*);
	void xMinusDataChanged();
	void xMinusColumnChanged(const AbstractColumn*);

	void yErrorTypeChanged(ErrorBar::ErrorType);
	void yPlusDataChanged();
	void yPlusColumnChanged(const AbstractColumn*);
	void yMinusDataChanged();
	void yMinusColumnChanged(const AbstractColumn*);

	void typeChanged(ErrorBar::Type);
	void capSizeChanged(double);
};

#endif
