/*
 * glframebufferctrls.cpp
 *
 *  Created on: Nov 19, 2013
 *      Author: max
 */

#include "glframebufferctrls.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Framebuffer settings ///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Main render controller item builder ////////////////////////////////////////////////////////////////////////
bool OverlayFramebufferCtrlsBuilder::buildItems (	OverlayItemsController * const& iFactory,
													QVector<OverlayItem*> * const& iItems,
													int iPos )
{
	//int h = iFactory->getParentWidget()->getWidgetHeight()-78;
	int h = 8;

	//Width
	OverlaySliderItem * bmItem = new OverlaySliderItem (iFactory, 0);
	bmItem->setData ("Render Width", iFactory->getParentWidget()->getFbSceneWidth(), QPoint(128,8000));
	bmItem->setPosition (QRect (iPos+82, h, 170, 33));
	bmItem->setPositionEnd (QRect (iPos +82, h, 170, 33) );
	bmItem->setOpacity (0.0f);
	bmItem->setHasNoSlider();
	bmItem->setHasNoNumPad();
	iItems->push_back (bmItem);

	//Height
	OverlaySliderItem * cmItem = new OverlaySliderItem (iFactory, 1);
	cmItem->setData ("Render Height", iFactory->getParentWidget()->getFbSceneHeight(), QPoint(128,8000));
	cmItem->setPosition (QRect (iPos+82, h, 170, 33));
	cmItem->setPositionEnd (QRect (iPos +82, h+37, 170, 33) );
	cmItem->setOpacity (0.0f);
	cmItem->setHasNoSlider();
	cmItem->setHasNoNumPad();
	iItems->push_back (cmItem);


	// FB buttons
	OverlayButtonItem * hmItem = new OverlayButtonItem (iFactory, 2);
	hmItem->setBckPixmaps (	"./images/gloverlay/button_SQ_XL_base.png",
							"./images/gloverlay/button_SQ_XL_over.png",
							"./images/gloverlay/x_icon.png");
	hmItem->setClickedPixmap (	"./images/gloverlay/x_clicked.png");
	hmItem->setParamName ("");
	hmItem->setPosition (QRect (iPos+82, h, 33, 33));
	hmItem->setPositionEnd (QRect (iPos +82+172, h, 33, 33) );
	hmItem->setOpacity (0.0f);
	iItems->push_back (hmItem);

	OverlayButtonItem * h2mItem = new OverlayButtonItem (iFactory, 3);
	h2mItem->setBckPixmaps (	"./images/gloverlay/button_SQ_XL_base.png",
							"./images/gloverlay/button_SQ_XL_over.png",
							"./images/gloverlay/x_icon.png");
	h2mItem->setClickedPixmap (	"./images/gloverlay/x_clicked.png");
	h2mItem->setParamName ("");
	h2mItem->setPosition (QRect (iPos+82, h, 33, 33));
	h2mItem->setPositionEnd (QRect (iPos +82+172+35, h, 33, 33) );
	h2mItem->setOpacity (0.0f);
	iItems->push_back (h2mItem);

	OverlayBoolItem * h3mItem = new OverlayBoolItem (iFactory, 4);
	h3mItem->setBckPixmaps (	"./images/gloverlay/button_SQ_XL_base.png",
							"./images/gloverlay/button_SQ_XL_over.png",
							"./images/gloverlay/x_icon.png");
	h3mItem->setClickedPixmap (	"./images/gloverlay/x_clicked.png");
	h3mItem->setData (iFactory->getParentWidget()->getFramebufferMode()==GLViewer::STICHTOWIDGET);
	h3mItem->setPosition (QRect (iPos+82, h, 33, 33));
	h3mItem->setPositionEnd (QRect (iPos +82+172, h+37, 33, 33) );
	h3mItem->setOpacity (0.0f);
	iItems->push_back (h3mItem);

	OverlayBoolItem * h4mItem = new OverlayBoolItem (iFactory, 5);
	h4mItem->setBckPixmaps (	"./images/gloverlay/button_SQ_XL_base.png",
							"./images/gloverlay/button_SQ_XL_over.png",
							"./images/gloverlay/x_icon.png");
	h4mItem->setClickedPixmap (	"./images/gloverlay/x_clicked.png");
	h4mItem->setData (iFactory->getParentWidget()->getFbZoomMode());
	h4mItem->setPosition (QRect (iPos+82, h, 33, 33));
	h4mItem->setPositionEnd (QRect (iPos +82+172 +35, h+37, 33, 33) );
	h4mItem->setOpacity (0.0f);
	iItems->push_back (h4mItem);

	//Percent slider
	OverlaySliderItem * tmItem = new OverlaySliderItem (iFactory, 6);
	tmItem->setData ("Percent", iFactory->getParentWidget()->getFbScenePercent(), QPoint(10,200));
	tmItem->setPosition (QRect (iPos+82, h, 170, 70));
	tmItem->setPositionEnd (QRect (iPos +82+172+35 +35, h, 170, 70) );
	tmItem->setOpacity (0.0f);
	tmItem->setHasNoNumPad();
	iItems->push_back (tmItem);


	//Navigator box
	OverlayNavigatorItem * navItem = new OverlayNavigatorItem (iFactory, 7);
	navItem->setBckPixmaps ("./images/gloverlay/quad70x70.png");

	navItem->setPosition (QRect (iPos+82, h, 140, 140));
	navItem->setPositionEnd (QRect (iPos +82+172+35+172 +35, h, 140, 140) );
	navItem->setOpacity (0.0f);

	//connection to setup item once anim finished
	connect (navItem, 	SIGNAL (global_anim_ended(int,int)),
			 navItem, 	SLOT (global_anim_ended_slot(int,int)) );

	//set it hidden initially
	navItem->setIsHidden (2);
	iItems->push_back (navItem);


	return iItems->size() != 0;
}

OverlayFramebufferCtrls::OverlayFramebufferCtrls (GLViewer *iWidget)
: OverlayItemsController(iWidget, new OverlayFramebufferCtrlsBuilder())
{

	///////////////////////////////////////////////////////////
	connect (	this, SIGNAL 	(items_ready ()),
				this, SLOT 		(slot_items_ready ()) );
	connect (	this, SIGNAL 	(anim_ended	(int,int)),
				this, SLOT 		(slot_items_endanimready (int,int)) );

	connect (	this, SIGNAL 	(resized ()),
				this, SLOT 		(slot_resized ()) );
}

// bind camera actions
void OverlayFramebufferCtrls::button_pressed_slot (bool pressed, int id)
{
	switch (id)
	{
	case 2:
		break;
	case 3:
		break;
	case 4: //stich fb to widget
		{
		if(pressed)
			getParentWidget()->setFramebufferMode (GLViewer::STICHTOWIDGET);
		else
			getParentWidget()->setFramebufferMode (GLViewer::SCENESIZE);

		//reset width/height item num fields
		OverlaySliderItem* wItem = static_cast<OverlaySliderItem*>(this->getItem(0));
		wItem->setData ("Render Width", this->getParentWidget()->getFbSceneWidth(), QPoint(128,8000));

		OverlaySliderItem* hItem = static_cast<OverlaySliderItem*>(this->getItem(1));
		hItem->setData ("Render Height", this->getParentWidget()->getFbSceneHeight(), QPoint(128,8000));
		}
		break;
	case 5: //zoom mode
		getParentWidget()->setFbZoomMode (pressed);
		break;
	}
}

// bind camera settings
void OverlayFramebufferCtrls::valueChanged_slot (int data, int decdigits, int id)
{
	GLViewer * tViewer = getParentWidget();

	switch (id)
	{
	case 0:	//Width
	{
		int rdata = data / pow(10,decdigits);

		if(tViewer->getFbSceneWidth() == rdata) return;

		tViewer->setFbSceneWidth (rdata);

	} break;
	case 1:	//Height
	{
		float rdata = data / pow(10,decdigits);
		if(tViewer->getFbSceneHeight() == rdata) return;

		tViewer->setFbSceneHeight (rdata);

	} break;
	case 6:	//Percent
	{
		if(data>200) data = 200;
		if(data<10) data = 10;

		//if(tViewer->getFbScenePercent() == data) return;

		//if stichtowidget do not update the viewer but just set the value
		//for when we'll get back to scene size mode
		tViewer->setFbScenePercent (data, tViewer->getFramebufferMode()==GLViewer::STICHTOWIDGET);

		OverlaySliderItem* wItem = static_cast<OverlaySliderItem*>(this->getItem(0));
		wItem->setData ("Render Width", tViewer->getFbSceneWidth(), QPoint(128,8000));

		OverlaySliderItem* hItem = static_cast<OverlaySliderItem*>(this->getItem(1));
		hItem->setData ("Render Height", tViewer->getFbSceneHeight(), QPoint(128,8000));
	} break;
	}
}

void OverlayFramebufferCtrls::slot_items_ready ()
{
	for (int i=0; i<getItems()->size(); i++)
	{
		if(i<2)
		{
			connect (	getItem(i), 	SIGNAL 	( dataChanged (int,int, int)),
						this, 			SLOT 	( valueChanged_slot (int,int, int)) );

		}else
		if(i<6)
		{
			connect (	getItem(i), 	SIGNAL 	( button_pressed (bool,int)),
						this, 			SLOT 	( button_pressed_slot (bool,int)) );
		}else
		if(i==6)
		{
			connect (	getItem(i), 	SIGNAL 	( dataChanged (int,int, int)),
						this, 			SLOT 	( valueChanged_slot (int,int, int)) );
		}
	}
}

void OverlayFramebufferCtrls::slot_items_endanimready (int animstate, int last_item) {}

void OverlayFramebufferCtrls::slot_resized ()
{
	//update size field once the render fb is stiched to the widget being resized
	if(this->getParentWidget()->getFramebufferMode()==GLViewer::STICHTOWIDGET)
	{
		OverlaySliderItem* wItem = static_cast<OverlaySliderItem*>(this->getItem(0));
		wItem->setData ("Render Width", this->getParentWidget()->getFbSceneWidth(), QPoint(128,8000));

		OverlaySliderItem* hItem = static_cast<OverlaySliderItem*>(this->getItem(1));
		hItem->setData ("Render Height", this->getParentWidget()->getFbSceneHeight(), QPoint(128,8000));
	}
}



