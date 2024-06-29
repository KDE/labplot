#include "Settings.h"

#include <KConfigGroup>

#include <QLatin1String>

/*!
 * Setup new setting.
 * setting_name: the name of the function used after the read/write prefix
 * datatype: the datatype of the setting
 * settings_datatype: the datatype used in the settings, for example an enum class cannot be directly stored in the settings, it must be converted to an integer
 * setting_group: the setting group in which the setting shall be located
 * config_name: the actual name used in the settings for reading/writing. This value must be unique within one setting_group!
 * default_value: the default value used, if no value for the setting is stored in the settings file
 */
#define SETUP_SETTING(setting_name, datatype, settings_datatype, setting_group, config_name, default_value)                                                    \
	namespace {                                                                                                                                                \
	const QLatin1String config_name##ConfigName(#config_name);                                                                                                 \
	}                                                                                                                                                          \
	/* read config */                                                                                                                                          \
	datatype read##setting_name() {                                                                                                                            \
		return static_cast<datatype>(setting_group.readEntry(config_name##ConfigName, static_cast<settings_datatype>(default_value)));                         \
	}                                                                                                                                                          \
	/* write config */                                                                                                                                         \
	void write##setting_name(const datatype& value) { setting_group.writeEntry(config_name##ConfigName, static_cast<settings_datatype>(value)); }

namespace Settings {

namespace {
KSharedConfig::Ptr confPtr;
} // anonymous namespace

KSharedConfig::Ptr config() {
	if (!confPtr)
		confPtr = KSharedConfig::openConfig();
	return confPtr;
}

KConfigGroup group(const QString& name) {
	return config()->group(name);
}

bool sync() {
	return config()->sync();
}

KConfigGroup settingsGeneral() {
	const QLatin1String settingsGeneralConfigName("Settings_General");
	return group(settingsGeneralConfigName);
}

SETUP_SETTING(DockPosBehavior, DockPosBehavior, int, settingsGeneral(), DockReopenPositionAfterClose, DockPosBehavior::AboveLastActive)

} // namespace Settings
