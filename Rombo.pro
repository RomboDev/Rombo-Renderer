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
		    source/glviewer/glrendererctrls.h \
		    source/glviewer/snapshot/pannableview_p.h \
		    source/glviewer/snapshot/pannableview.h \
		    source/glviewer/snapshot/snapshotsviewer.h
		    
SOURCES   += source/main.cpp \
    		source/rombo.cpp \
    		source/renderdevice.cpp \
    		source/glmanager.cpp \
    		source/glviewer.cpp \
		    source/glviewer/glrendercamera.cpp \
		    source/glviewer/glrenderregion.cpp \
		    source/glviewer/gloverlaycontrols.cpp \
		    source/glviewer/glframebufferctrls.cpp \
		    source/glviewer/glrendererctrls.cpp \
		    source/glviewer/snapshot/pannableview.cpp \
		    source/glviewer/snapshot/snapshotsviewer.cpp
    		
FORMS     += rombo.ui    
RESOURCES += azpresources.qrc
