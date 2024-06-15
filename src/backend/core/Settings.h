#ifndef SETTINGS_H
#define SETTINGS_H

class KConfigGroup;

#include <KSharedConfig>

namespace Settings {

KConfigGroup group(const QString& name);
KConfigGroup settingsGeneral();
bool sync();

#define SETUP_SETTING2(setting_name, datatype)                                                                                                                 \
	datatype read##setting_name();                                                                                                                             \
	void write##setting_name(const datatype& value);

enum class DockPosBehavior { OriginalPos, AboveLastActive };
SETUP_SETTING2(DockPosBehavior, DockPosBehavior)
}

#endif // SETTINGS_H
