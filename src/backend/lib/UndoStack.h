#include "macros.h"
#include <QUndoStack>

class UndoStack : public QUndoStack {
public:
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

	void redo() {
		if (!m_executing) {
			Lock l(m_executing);
			QUndoStack::redo();
		} else {
			// Should never happen!
			assert(false);
		}
	}

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
	bool m_executing{false};
};
