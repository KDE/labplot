#ifndef SETTINGS_H
#define SETTINGS_H

class KConfigGroup;

#include <KSharedConfig>
#include <QString>

namespace Settings {

KConfigGroup group(const QString& name);
KConfigGroup settingsGeneral();
bool sync();

#define SETUP_SETTING_DECLARATION(setting_name, datatype)                                                                                                      \
	datatype read##setting_name();                                                                                                                             \
	void write##setting_name(const datatype& value);

enum class DockPosBehavior { OriginalPos, AboveLastActive };
SETUP_SETTING_DECLARATION(DockPosBehavior, DockPosBehavior)
SETUP_SETTING_DECLARATION(XYCurveOptimizationLimit, int)
SETUP_SETTING_DECLARATION(XYCurveDrawPathLimit, int)

}

#endif // SETTINGS_H
