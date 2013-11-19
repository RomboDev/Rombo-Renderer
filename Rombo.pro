TEMPLATE = app
TARGET = Rombo 

QT        += core gui opengl 

HEADERS   += rombo.h \
			renderdevice.h \
			IGLViewerDevice.h \
			glviewer.h \
		    glviewer/glrendercamera.h \
		    glviewer/glrenderregion.h \
		    glviewer/gloverlaycontrols.h \
		    glviewer/glframebufferctrls.h \
		    glviewer/glrendererctrls.h
		    
SOURCES   += main.cpp \
    		rombo.cpp \
    		renderdevice.cpp \
    		glviewer.cpp \
		    glviewer/glrendercamera.cpp \
		    glviewer/glrenderregion.cpp \
		    glviewer/gloverlaycontrols.cpp \
		    glviewer/glframebufferctrls.cpp \
		    glviewer/glrendererctrls.cpp 
    		
FORMS     += rombo.ui    
RESOURCES += azpresources.qrc
