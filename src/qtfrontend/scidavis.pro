### IMPORTANT: user-servicable part is now in config.pri

include(config.pri)
TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = $${MODULES} core

### update translations
#system(lupdate -verbose scidavis-translations.pro)
#system(lrelease -verbose scidavis-translations.pro)

### DOCUMENTATION

documentation.files += \
	README \
	CHANGES \
	gpl.txt \
	INSTALL.html \
	scidavis.css \
	scidavis-logo.png \
	manual/html \

