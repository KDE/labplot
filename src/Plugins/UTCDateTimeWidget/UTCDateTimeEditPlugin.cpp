#include "NumberSpinBoxPlugin.h"
#include "../../frontend/widgets/NumberSpinBox.h"

NumberSpinBoxPlugin::NumberSpinBoxPlugin(QObject *parent)
    : QObject(parent) {
}

QWidget *NumberSpinBoxPlugin::createWidget(QWidget *parent) {
    return new NumberSpinBox(parent);
}

void NumberSpinBoxPlugin::initialize(QDesignerFormEditorInterface * /* core */) {
    if (initialized)
        return;

    initialized = true;
}

bool NumberSpinBoxPlugin::isInitialized() const {
    return initialized;
}

QString NumberSpinBoxPlugin::name() const {
    return QStringLiteral("NumberSpinBox");
}

QString NumberSpinBoxPlugin::group() const {
    return QStringLiteral("Input Widgets");
}

QIcon NumberSpinBoxPlugin::icon() const {
    return QIcon();
}

QString NumberSpinBoxPlugin::toolTip() const {
    return {};
}

QString NumberSpinBoxPlugin::whatsThis() const {
    return {};
}

bool NumberSpinBoxPlugin::isContainer() const {
    return false;
}

QString NumberSpinBoxPlugin::domXml() const {
    return QLatin1String(R"(
<ui language="c++">
  <widget class="NumberSpinBox" name="numberSpinBox">
)"
R"(
    <property name="toolTip">
      <string></string>
    </property>
    <property name="feedback">
        <bool>false</bool>
    </property>
    <property name="whatsThis">
      <string>Spinbox for numbers.</string>
    </property>
  </widget>
</ui>
)");
}

QString NumberSpinBoxPlugin::includeFile() const
{
    return QStringLiteral("NumberSpinBox.h");
}
