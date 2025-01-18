#ifndef SETTINGS_H
#define SETTINGS_H

class KConfigGroup;

#include <KSharedConfig>

namespace Settings {

enum class Type {
		General,
		General_Number_Format,
		General_Units,
		Worksheet,
		Spreadsheet,
#ifdef HAVE_CANTOR_LIBS
		Notebook,
#endif
		Datasets,
#ifdef HAVE_KUSERFEEDBACK
		Feedback
#endif
	};

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
