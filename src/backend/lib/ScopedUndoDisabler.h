/*
	File                 : ScopedUndoDisabler.h
	Project              : LabPlot
	Description          : RAII helper to temporarily disable undo awareness globally.
	--------------------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCOPEDUNDODISABLER_H
#define SCOPEDUNDODISABLER_H

#include "backend/core/Project.h"

/**
 * \class ScopedUndoDisabler
 * \brief RAII helper class to temporarily disable undo awareness for a project globally.
 *
 * This class uses the RAII (Resource Acquisition Is Initialization) pattern
 * to automatically restore the undo-aware state when the object goes out of scope.
 *
 * Usage example:
 * \code
 * {
 *     ScopedUndoDisabler undoDisabler(project());
 *     // ... perform operations without undo tracking ...
 *     // undo awareness is automatically restored when undoDisabler goes out of scope
 * }
 * \endcode
 */
class ScopedUndoDisabler {
public:
	/**
	 * \brief Constructor - disables undo awareness if it was enabled
	 * \param project The project to disable undo awareness for
	 */
	explicit ScopedUndoDisabler(Project* project)
		: m_project(project) {
		if (m_project && m_project->isUndoAware()) {
			m_project->setUndoAware(false);
			m_wasUndoAware = true;
		}
	}

	/**
	 * \brief Destructor - restores the original undo awareness state
	 */
	~ScopedUndoDisabler() {
		if (m_wasUndoAware)
			m_project->setUndoAware(true);
	}

	// Delete copy constructor and assignment operator to prevent misuse
	ScopedUndoDisabler(const ScopedUndoDisabler&) = delete;
	ScopedUndoDisabler& operator=(const ScopedUndoDisabler&) = delete;

private:
	Project* m_project{nullptr};
	bool m_wasUndoAware{false};
};

#endif // SCOPEDUNDODISABLER_H
