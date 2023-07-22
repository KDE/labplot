#ifndef SETTINGS_H
#define SETTINGS_H

class KConfigGroup;

namespace Settings {
enum class DockPosBehaviour { OriginalPos, AboveLastActive };
DockPosBehaviour dockPosBehaviour();
void saveDockPosBehaviour(KConfigGroup& group, DockPosBehaviour posBehaviour);
}

#endif // SETTINGS_H
