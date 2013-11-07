TEMPLATE = app
TARGET = Rombo 

QT        += core gui opengl 

HEADERS   += rombo.h \
			renderdevice.h \
			IGLViewerDevice.h \
			glviewer.h \
		    glrendercamera.h \
		    glscreencontrols.h \
		    glrenderregion.h
SOURCES   += main.cpp \
    		rombo.cpp \
    		renderdevice.cpp \
    		glviewer.cpp \
		    glrendercamera.cpp \
		    glscreencontrols.cpp \
		    glrenderregion.cpp
    		
FORMS     += rombo.ui    
RESOURCES += azpresources.qrc
