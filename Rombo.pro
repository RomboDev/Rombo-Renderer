TEMPLATE = app
TARGET = Rombo 

QT        += core gui opengl 

HEADERS   += rombo.h \
			glviewer.h
SOURCES   += main.cpp \
    		rombo.cpp \
    		glviewer.cpp
    		
FORMS     += rombo.ui    
RESOURCES += azpresources.qrc
