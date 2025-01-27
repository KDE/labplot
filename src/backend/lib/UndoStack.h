#include <QUndoStack>
#include "macros.h"

class UndoStack: public QUndoStack {
public:
    void push(QUndoCommand *cmd) {
        if (!m_executing) {
			Lock l(m_executing);
            QUndoStack::push(cmd);
        } else {
   assert(false); // This case should never happen. So let it crash for the debug build and proceed in the release build
			// Ignore the undo capability here,
			// because it is not allowed to push on the undostack
			// while the undostack is in a undo/redo operation
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
