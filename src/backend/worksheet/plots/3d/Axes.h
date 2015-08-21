/***************************************************************************
    File                 : Axes.h
    Project              : LabPlot
    Description          : 3D plot axes
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Minh Ngo (minh@fedoraproject.org)

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

#ifndef PLOT3D_AXES_H
#define PLOT3D_AXES_H

#include "Base3D.h"
#include "backend/lib/macros.h"

class QColor;

class AxesPrivate;
class Axes : public Base3D {
		Q_OBJECT
		Q_DECLARE_PRIVATE(Axes)
		Q_DISABLE_COPY(Axes)
	public:
		enum Format {
			Format_Decimal,
			Format_Scientific,
			Format_PowerOf10,
			Format_PowerOf2,
			Format_PowerOfE,
			Format_MultiplierOfPi
		};

		Axes();
		~Axes();

		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);

		QIcon icon() const;

		BASIC_D_ACCESSOR_DECL(Format, formatX, FormatX)
		BASIC_D_ACCESSOR_DECL(Format, formatY, FormatY)
		BASIC_D_ACCESSOR_DECL(Format, formatZ, FormatZ)

		BASIC_D_ACCESSOR_DECL(int, fontSize, FontSize)
		CLASS_D_ACCESSOR_DECL(QColor, xLabelColor, XLabelColor)
		CLASS_D_ACCESSOR_DECL(QColor, yLabelColor, YLabelColor)
		CLASS_D_ACCESSOR_DECL(QColor, zLabelColor, ZLabelColor)
		CLASS_D_ACCESSOR_DECL(QString, xLabel, XLabel)
		CLASS_D_ACCESSOR_DECL(QString, yLabel, YLabel)
		CLASS_D_ACCESSOR_DECL(QString, zLabel, ZLabel)

		typedef Axes BaseClass;
		typedef AxesPrivate Private;

	public slots:
		void updateBounds();

	signals:
		friend class AxesSetFormatXCmd;
		friend class AxesSetFormatYCmd;
		friend class AxesSetFormatZCmd;
		friend class AxesSetFontSizeCmd;
		friend class AxesSetXLabelColorCmd;
		friend class AxesSetYLabelColorCmd;
		friend class AxesSetZLabelColorCmd;
		friend class AxesSetXLabelCmd;
		friend class AxesSetYLabelCmd;
		friend class AxesSetZLabelCmd;
		void formatXChanged(Axes::Format);
		void formatYChanged(Axes::Format);
		void formatZChanged(Axes::Format);
		void fontSizeChanged(int);
		void xLabelColorChanged(const QColor&);
		void yLabelColorChanged(const QColor&);
		void zLabelColorChanged(const QColor&);
		void xLabelChanged(const QString&);
		void yLabelChanged(const QString&);
		void zLabelChanged(const QString&);
};

#endif