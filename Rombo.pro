TEMPLATE = app
TARGET = Rombo 

QT        += core gui opengl 

HEADERS   += source/rombo.h \
			source/renderdevice.h \
			source/glmanager.h \
			source/glviewer.h \
			source/glviewer/IGLViewerDevice.h \
		    source/glviewer/glrendercamera.h \
		    source/glviewer/glrenderregion.h \
		    source/glviewer/gloverlaycontrols.h \
		    source/glviewer/glframebufferctrls.h \
		    source/glviewer/glrendererctrls.h
		    
SOURCES   += source/main.cpp \
    		source/rombo.cpp \
    		source/renderdevice.cpp \
    		source/glmanager.cpp \
    		source/glviewer.cpp \
		    source/glviewer/glrendercamera.cpp \
		    source/glviewer/glrenderregion.cpp \
		    source/glviewer/gloverlaycontrols.cpp \
		    source/glviewer/glframebufferctrls.cpp \
		    source/glviewer/glrendererctrls.cpp 
    		
FORMS     += rombo.ui    
RESOURCES += azpresources.qrc
