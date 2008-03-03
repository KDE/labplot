TARGET  	 = liborigin
TEMPLATE     = lib
CONFIG      += warn_on release thread
CONFIG      += dll 
MOC_DIR      = ./tmp
OBJECTS_DIR  = ./tmp

DESTDIR      = ./

HEADERS 	 = OPJFile.h  
SOURCES  	 = OPJFile.cpp
