QT          += widgets uiplugin
CONFIG      += plugin
TEMPLATE    = lib

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target

INCLUDEPATH += $$PWD \
                $$PWD/../.. \
                $$PWD/../../commonfrontend/widgets/NumberSpinBox.h
HEADERS += $$PWD/../../commonfrontend/widgets/NumberSpinBox.h \
            NumberSpinBoxPlugin.h
SOURCES += $$PWD/../../commonfrontend/widgets/NumberSpinBox.cpp \
            NumberSpinBoxPlugin.cpp
OTHER_FILES += NumberSpinBox.json
