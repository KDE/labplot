#include "Settings.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <QLatin1String>

namespace Settings {

namespace {
static constexpr QLatin1String settingsGeneralConfigName{"Settings_General"};
static constexpr QLatin1String dockReopenPositionAfterCloseConfigName{"DockReopenPositionAfterClose"};

}

DockPosBehaviour dockPosBehaviour() {
	return static_cast<DockPosBehaviour>(KSharedConfig::openConfig()
											 ->group(settingsGeneralConfigName)
											 .readEntry(dockReopenPositionAfterCloseConfigName, static_cast<int>(DockPosBehaviour::AboveLastActive)));
}
void saveDockPosBehaviour(KConfigGroup& group, DockPosBehaviour posBehaviour) {
	group.writeEntry(dockReopenPositionAfterCloseConfigName, static_cast<int>(posBehaviour));
}
}
