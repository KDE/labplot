/***************************************************************************
    File                 : AbstractFit.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Knut Franke
    Email (use @ for *)  : knut.franke*gmx.de
    Description          : Base class for doing fits using the algorithms
                           provided by GSL.

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

#include "AbstractFit.h"
#include <QUndoCommand>
#include <gsl/gsl_multifit_nlin.h>
#include <gsl/gsl_multimin.h>

class FitSetAlgorithmCmd : public QUndoCommand
{
	public:
		FitSetAlgorithCmd(AbstractFit *target, AbstractFit::Algorithm algo)
			: m_target(target), m_other_algo(algo) {
				setText(QObject::tr("%1: change fit algorithm to %2.").arg(m_target->name()).arg(AbstractFit::nameOf(m_algo)));
			}

		virtual void undo() {
			AbstractFit::Algorithm tmp = m_target->m_algorithm;
			m_target->m_algorithm = m_other_algo;
			m_other_algo = tmp;
		}

		virtual void redo() { undo(); }

	private:
		AbstractFit *m_target;
		AbstractFit::Algorithm m_other_algo;
};

class FitSetAutoRefitCmd : public QUndoCommand
{
	public:
		FitSetAutoRefitCmd(AbstractFit *target, bool refit)
			: m_target(target), m_backup(refit) {
				setText((refit ?
							QObject::tr("%1: switch auto-refit on.", "label of AbstractFit's undo action") :
							QObject::tr("%1: switch auto-refit off.", "label of AbstractFit's undo action")).
						arg(m_target->name()));
			}

		virtual void undo() {
			bool tmp = m_target->m_auto_refit;
			m_target->m_auto_refit = m_backup;
			m_backup = tmp;
		}

		virtual void redo() { undo(); }

	private:
		AbstractFit *m_target;
		bool m_backup;
};
