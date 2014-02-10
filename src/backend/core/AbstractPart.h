/***************************************************************************
    File                 : AbstractPart.h
    Project              : SciDAVis
    Description          : Base class of Aspects with MDI windows as views.
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Knut Franke (knut.franke*gmx.de)
    Copyright            : (C) 2012-2013 Alexander Semke (alexander.semke*web.de)
                           (replace * with @ in the email address)

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
#ifndef ABSTRACT_PART_H
#define ABSTRACT_PART_H

#include "AbstractAspect.h"

class PartMdiView;
class QMenu;

class AbstractPart : public AbstractAspect {
	Q_OBJECT

	public:
		explicit AbstractPart(const QString &name);
		virtual ~AbstractPart();
		
		virtual QWidget* view() const = 0;
		void deleteView() const;

		PartMdiView* mdiSubWindow() const;
		bool hasMdiSubWindow() const;
		void deleteMdiSubWindow();
		
		virtual QMenu* createContextMenu();
		virtual bool fillProjectMenu(QMenu* menu);

	public slots:
		virtual void editCopy();
		virtual void editCut();
		virtual void editPaste();

	private:
		mutable PartMdiView* m_mdiWindow;

	protected:
		mutable QWidget* m_view;

	signals:
		void showRequested();
		void exportRequested();
		void printRequested();
		void printPreviewRequested();
};

#endif // ifndef ABSTRACT_PART_H
