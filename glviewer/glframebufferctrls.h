/*
 * glframebufferctrls.h
 *
 *  Created on: Nov 19, 2013
 *      Author: max
 */

#ifndef GLFRAMEBUFFERCTRLS_H_
#define GLFRAMEBUFFERCTRLS_H_


#include "gloverlaycontrols.h"


// main controller item builder
class OverlayFramebufferCtrlsBuilder: public OverlayItemsBuilder
{
public:
	bool buildItems (OverlayItemsController * const& iFactory, QVector<OverlayItem*> * const& iItems, int iPos);
};

// main bar controller
class OverlayFramebufferCtrls: public OverlayItemsController
{
	Q_OBJECT
public:
	OverlayFramebufferCtrls (GLViewer *iWidget);

private slots:
	void valueChanged_slot (int data, int decdigits, int id);
	void button_pressed_slot (bool pressed, int id);
    void slot_items_ready ();
    void slot_items_endanimready (int animstate, int last_item);
	void slot_resized ();
};

#endif /* GLFRAMEBUFFERCTRLS_H_ */
