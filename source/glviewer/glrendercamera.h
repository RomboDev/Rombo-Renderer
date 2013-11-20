/*
 * glrendercamera.h
 *
 *  Created on: Jan 18, 2013
 *      Author: max
 */

#ifndef GLRENDERCAMERA_H_
#define GLRENDERCAMERA_H_

#include <QtGui>
#include "../glviewer.h"

class GLViewer;

class RenderCamera : public IGLViewerDevice
{
	Q_OBJECT
public:
	RenderCamera (GLViewer * widget);
    ~RenderCamera() { m_widget=NULL; }

    void setWidget(GLViewer * widget);

protected:
    bool eventFilter (QObject *object, QEvent *event);

private slots:
	virtual void viewerInit (){};
	virtual void viewerResized (){}
	virtual void viewerPaint (QPainter* iPainter) { /*do nothing*/; }

private:
    void registerDevice ();

private:
	// mouse controls
	int m_mouseMode;
	int m_clickX;
	int m_clickY;

	bool isMoving;

	//host widget
	GLViewer * m_widget;
};

#endif /* GLRENDERCAMERA_H_ */
