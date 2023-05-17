QT          += widgets uiplugin
CONFIG      += plugin
TEMPLATE    = lib

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target

INCLUDEPATH += $$PWD \
                $$PWD/../.. \
                $$PWD/../../commonfrontend/widgets/UTCDateTimeEdit.h
HEADERS += $$PWD/../../commonfrontend/widgets/UTCDateTimeEdit.h \
            UTCDateTimeEditPlugin.h
SOURCES += $$PWD/../../commonfrontend/widgets/UTCDateTimeEdit.cpp \
            UTCDateTimeEditPlugin.cpp
OTHER_FILES += UTCDateTimeEdit.json
