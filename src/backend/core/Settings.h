#ifndef SETTINGS_H
#define SETTINGS_H

class KConfigGroup;

#include <KSharedConfig>

namespace Settings {

KSharedConfig::Ptr config();
KConfigGroup settingsGeneral();
enum class DockPosBehaviour { OriginalPos, AboveLastActive };

#define SETUP_SETTING(setting_name, datatype)                                                                                                                  \
	datatype read##setting_name();                                                                                                                             \
	void write##setting_name(const datatype& value);

SETUP_SETTING(DockPosBehaviour, DockPosBehaviour)
}

#endif // SETTINGS_H
