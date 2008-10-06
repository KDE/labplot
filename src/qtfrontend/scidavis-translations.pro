# This file is only meant to be used by lupdate and lrelease.

TRANSLATIONS = \
	translations/scidavis_de.ts \
	translations/scidavis_es.ts \
	translations/scidavis_fr.ts \
	translations/scidavis_ru.ts \
	translations/scidavis_ja.ts \
	translations/scidavis_sv.ts \

### modules to be included in the translations

DEPENDPATH += core
include(core/core.pro)

DEPENDPATH += table
include(table/table.pro)

DEPENDPATH += python
include(python/python.pro)

DEPENDPATH += muparser
include(muparser/muparser.pro)

DEPENDPATH += graph
include(graph/graph.pro)

DEPENDPATH += graph-3D
include(graph-3D/graph-3D.pro)

DEPENDPATH += notes
include(notes/notes.pro)

