/*
 * glviewer.h
 *
 *  Created on: Nov 6, 2013
 *      Author: max
 */

#ifndef GLVIEWER_H_
#define GLVIEWER_H_


#include <QGLWidget>
#include <QGLFramebufferObject>
#include <QGLBuffer>
#include <QMouseEvent>
#include <QKeyEvent>

/*
#include "IGLViewerDevice.h"
#include"renderdevice.h"
#include"glrendercamera.h"
#include"glrenderregion.h"
#include"glscreencontrols.h"
*/

class RenderCamera;
class RenderRegion;
class OverlayItemsController;

class GLViewer : public QGLWidget
{
	Q_OBJECT

public:
	GLViewer ( int argc, char *argv[] );
	~GLViewer ();
};


#endif /* GLVIEWER_H_ */
