/*
	File                 : UndoStack.h
	Project              : LabPlot
	Description 	     : Reentrancy guard wrapper around QUndoStack to prevent stack corruption
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "macros.h"
#include <QUndoStack>

/**
 * \class UndoStack
 * \brief Reentrancy guard wrapper around QUndoStack to prevent stack corruption.
 *
 * This class wraps Qt's QUndoStack and prevents a critical issue: pushing commands
 * onto the undo stack while already executing an undo/redo operation.
 *
 * Qt's QUndoStack is not reentrant. Attempting to modify the stack during an active
 * undo/redo operation causes stack corruption and cryptic crashes. A typical chain 
 * of events leading to such problems is:
 * 1. User triggers undo/redo operation
 * 2. UndoStack begins executing the command (stack is being traversed)
 * 3. Command execution triggers signals (e.g., data changed, aspect modified)
 * 4. Signal handlers modify objects that are undo-aware
 * 5. These modifications attempt to push new commands onto the stack
 * 6. **CRASH**: Stack is modified while being traversed → undefined behavior
 *
 * \section solution The Solution
 * This class uses a state flag with RAII locking to detect reentrancy attempts:
 * - If reentrancy detected:
 *   - Debug builds: assert(false) → crashes early to help catch bugs
 *   - Release builds: Executes command directly without undo tracking,
 *     preventing the crash but losing undo capability for that operation
 */

class UndoStack : public QUndoStack {
public:
	/**
	 * \brief Push a command onto the undo stack with reentrancy protection
	 * \param cmd The command to push
	 *
	 * If called during an active undo/redo operation, asserts in debug builds
	 * and executes the command directly (without undo capability) in release builds.
	 */
	void push(QUndoCommand* cmd) {
		if (!m_executing) {
			Lock l(m_executing);
			QUndoStack::push(cmd);
		} else {
			// this case should never happen. So let it crash for the debug build and proceed in the release build
			assert(false);

			// ignore the undo capability here, it's not allowed to push on the undostack
			// while still processing another undo/redo operation
			cmd->redo();
		}
	}

	/**
	 * \brief Redo the next command with reentrancy protection
	 *
	 * If called during an active undo/redo operation, asserts in debug builds
	 * and does nothing in release builds.
	 */
	void redo() {
		if (!m_executing) {
			Lock l(m_executing);
			QUndoStack::redo();
		} else {
			// Should never happen!
			assert(false);
		}
	}

	/**
	 * \brief Undo the last command with reentrancy protection
	 *
	 * If called during an active undo/redo operation, asserts in debug builds
	 * and does nothing in release builds.
	 */
	void undo() {
		if (!m_executing) {
			Lock l(m_executing);
			QUndoStack::undo();
		} else {
			// Should never happen!
			assert(false);
		}
	}

private:
	bool m_executing{false}; ///< flag indicating if an undo/redo operation is currently executing
};
