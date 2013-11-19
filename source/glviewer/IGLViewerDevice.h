/*
 * IGLViewerDevices.h
 *
 *  Created on: Feb 4, 2013
 *      Author: max
 */

#ifndef IGLVIEWERDEVICE_H_
#define IGLVIEWERDEVICE_H_

#include <QtGui>

class IGLViewerDevice:public QObject
{
	Q_OBJECT
public:
	struct IGLViewerSize
	{
		QSize widget_size;
		QSize fb_size;
		QSize offs_size;
	};

signals:
	void devicePainting (bool ispainting);

public slots:
	virtual void viewerInit () = 0;
	virtual void viewerResized () = 0;
	virtual void viewerPaint (QPainter* iPainter) = 0;

public:
	virtual void registerDevice () = 0;
};


#endif /* IGLVIEWERDEVICE_H_ */
