/***************************************************************************
    File                 : Image.h
    Project              : LabPlot
    Description          : Worksheet element to draw images
    --------------------------------------------------------------------
    Copyright            : (C) 2019 Alexander Semke (alexander.semke@web.de)

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

#ifndef IMAGE_H
#define IMAGE_H

#include "backend/lib/macros.h"
#include "backend/worksheet/WorksheetElement.h"

#include <QPen>

class QBrush;
class QFont;
class ImagePrivate;

class Image : public WorksheetElement {
	Q_OBJECT

public:
	explicit Image(const QString& name);
	~Image() override;

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QGraphicsItem* graphicsItem() const override;
	void setParentGraphicsItem(QGraphicsItem*);

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig&) override;
	void saveThemeConfig(const KConfig&) override;

	CLASS_D_ACCESSOR_DECL(QString, fileName, FileName)
	BASIC_D_ACCESSOR_DECL(qreal, opacity, Opacity)
	CLASS_D_ACCESSOR_DECL(WorksheetElement::PositionWrapper, position, Position)
	void setPosition(QPointF);
	BASIC_D_ACCESSOR_DECL(WorksheetElement::HorizontalAlignment, horizontalAlignment, HorizontalAlignment)
	BASIC_D_ACCESSOR_DECL(WorksheetElement::VerticalAlignment, verticalAlignment, VerticalAlignment)
	BASIC_D_ACCESSOR_DECL(qreal, rotationAngle, RotationAngle)

	CLASS_D_ACCESSOR_DECL(QPen, borderPen, BorderPen)
	BASIC_D_ACCESSOR_DECL(qreal, borderOpacity, BorderOpacity)

	void setVisible(bool on) override;
	bool isVisible() const override;
	void setPrinting(bool) override;

	void retransform() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;

	typedef ImagePrivate Private;

private slots:
	//SLOTs for changes triggered via QActions in the context menu
	void visibilityChanged();

protected:
	ImagePrivate* const d_ptr;
	Image(const QString&, ImagePrivate*);

private:
	Q_DECLARE_PRIVATE(Image)
	void init();

	QAction* visibilityAction{nullptr};

signals:
	void fileNameChanged(const QString&);
	void opacityChanged(float);
	void positionChanged(const WorksheetElement::PositionWrapper&);
	void horizontalAlignmentChanged(WorksheetElement::HorizontalAlignment);
	void verticalAlignmentChanged(WorksheetElement::VerticalAlignment);
	void rotationAngleChanged(qreal);
	void borderPenChanged(QPen&);
	void borderOpacityChanged(float);
	void visibleChanged(bool);
	void changed();
};

#endif
