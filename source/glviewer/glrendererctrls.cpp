/*
 * glrendererctrl.cpp
 *
 *  Created on: Nov 19, 2013
 *      Author: max
 */


#include "glrendererctrls.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Renderer settings //////////////////////////////////////////////////////////////////////////////////////////

// subitems builder
bool OverlayRenderSettingsBuilder::buildItems ( OverlayItemsController * const& iFactory,
												QVector<OverlayItem*> * const& iSubitems,
												int iPos )
{
	iFactory->setActiveDevice(1);

	int h = iFactory->getParentWidget()->getWidgetHeight()-78;

	OverlaySliderItem * tmItem = new OverlaySliderItem (iFactory, 0);
	tmItem->setData ("Max Depth", iFactory->getParentWidget()->getRendererMaxDepth(), QPoint(1,32));
	tmItem->setPosition (QRect (iPos+82, h, 170, 70));
	tmItem->setPositionEnd (QRect (iPos +82, h, 170, 70) );
	tmItem->setOpacity (0.0f);
	iSubitems->push_back (tmItem);

	OverlaySliderItem * xmItem = new OverlaySliderItem (iFactory, 1);
	xmItem->setData ("Samples Per Pixel", iFactory->getParentWidget()->getRendererSPP(), QPoint(1,8));
	xmItem->setPosition (QRect (iPos+82, h, 170, 70));
	xmItem->setPositionEnd (QRect (iPos +82+172, h, 170, 70) );
	xmItem->setOpacity (0.0f);
	iSubitems->push_back (xmItem);

	OverlaySliderItem * amItem = new OverlaySliderItem (iFactory, 2);
	amItem->setData ("Min Contribution", iFactory->getParentWidget()->getRendererMinContribution()*1000, QPoint(1,100), 3);
	amItem->setPosition (QRect (iPos+82, h, 170, 70));
	amItem->setPositionEnd (QRect (iPos +82+172+172, h, 170, 70) );
	amItem->setOpacity (0.0f);
	iSubitems->push_back (amItem);

	return iSubitems->size() != 0;
}

// bind signaling
void OverlayRendererSettingsItem::bind_subitems (int iID)
{
	for (int i=0; i<getSubitems()->size(); i++)
	{
		connect (	getSubitem(i), 	SIGNAL 	( dataChanged (int,int, int)),
					this, 			SLOT 	( valueChanged_slot (int,int, int)) );
		connect(	this, 		 	SIGNAL 	( undoredo_slot (int,int)),
					getSubitem(i), 	SLOT 	( setData (int,int)));
	}
}

// bind renderer settings
void OverlayRendererSettingsItem::valueChanged_slot (int data, int decdigits, int id)
{
	GLViewer * tViewer = getFactory()->getParentWidget();

	switch (id)
	{
	case 0:	//MaxDepth GLViewer
		if(tViewer->getRendererMaxDepth() != data ){
			//tViewer->setRendererMaxDepth (data);

			std::cout << "PUSHdepth!" << std::endl;
			QUndoCommand *rCommand = new GLViewer::glRendererCommand (0, data, tViewer);
			tViewer->getStack()->push (rCommand);
		}
		break;
	case 1:	//SPP
		if(tViewer->getRendererSPP() != data ){
			//tViewer->setRendererSPP (data);
			std::cout << "PUSHspp!" << std::endl;
			QUndoCommand *rCommand = new GLViewer::glRendererCommand (1, data, tViewer);
			tViewer->getStack()->push (rCommand);
		}
		break;
	case 2:	//MinContribution
		float rdata = data / pow(10,decdigits);
		if(tViewer->getRendererMinContribution() != rdata ){
			//tViewer->setRendererMinContribution (rdata);
			std::cout << "PUSHminc!" << std::endl;
			QUndoCommand *rCommand = new GLViewer::glRendererCommand (2, rdata, tViewer);
			tViewer->getStack()->push (rCommand);
		}
		break;
	}
}


// Camera settings //////////////////////////////////////////////////////////////////////////////////////////

// subitems builder
bool OverlayCameraSettingsBuilder::buildItems ( OverlayItemsController * const& iFactory,
												QVector<OverlayItem*> * const& iSubitems,
												int iPos )
{
	iFactory->setActiveDevice(2);

	int h = iFactory->getParentWidget()->getWidgetHeight()-78;

	// center focus buttons
	OverlayButtonItem * bmItem = new OverlayButtonItem (iFactory, 0);
	bmItem->setBckPixmaps (	"./images/gloverlay/button_HL_base.png",
							"./images/gloverlay/button_HL_over.png",
							"./images/gloverlay/camera_center_icon.png");
	bmItem->setClickedPixmap (	"./images/gloverlay/camera_center_clicked.png");
	bmItem->setParamName ("Pick Center");
	bmItem->setPosition (QRect (iPos+82, h, 120, 33));
	bmItem->setPositionEnd (QRect (iPos +82, h, 120, 33) );
	bmItem->setOpacity (0.0f);
	iSubitems->push_back (bmItem);

	OverlayButtonItem * cmItem = new OverlayButtonItem (iFactory, 1);
	cmItem->setBckPixmaps (	"./images/gloverlay/button_HL_base.png",
							"./images/gloverlay/button_HL_over.png",
							"./images/gloverlay/camera_center_icon.png");
	cmItem->setClickedPixmap (	"./images/gloverlay/camera_center_clicked.png");
	cmItem->setParamName ("Pick Focus");
	cmItem->setPosition (QRect (iPos+82, h, 120, 33));
	cmItem->setPositionEnd (QRect (iPos +82, h+37, 120, 33) );
	cmItem->setOpacity (0.0f);
	iSubitems->push_back (cmItem);

	// nav buttons
	OverlayButtonItem * hmItem = new OverlayButtonItem (iFactory, 2);
	hmItem->setBckPixmaps (	"./images/gloverlay/button_SQ_XL_base.png",
							"./images/gloverlay/button_SQ_XL_over.png",
							"./images/gloverlay/camera_orbit_icon.png");
	hmItem->setClickedPixmap (	"./images/gloverlay/camera_orbit_clicked.png");
	hmItem->setParamName ("");
	hmItem->setPosition (QRect (iPos+82, h, 33, 33));
	hmItem->setPositionEnd (QRect (iPos +82+122, h, 33, 33) );
	hmItem->setOpacity (0.0f);
	iSubitems->push_back (hmItem);

	OverlayButtonItem * h2mItem = new OverlayButtonItem (iFactory, 3);
	h2mItem->setBckPixmaps (	"./images/gloverlay/button_SQ_XL_base.png",
							"./images/gloverlay/button_SQ_XL_over.png",
							"./images/gloverlay/camera_pan_icon.png");
	h2mItem->setClickedPixmap (	"./images/gloverlay/camera_pan_clicked.png");
	h2mItem->setParamName ("");
	h2mItem->setPosition (QRect (iPos+82, h, 33, 33));
	h2mItem->setPositionEnd (QRect (iPos +82+122+35, h, 33, 33) );
	h2mItem->setOpacity (0.0f);
	iSubitems->push_back (h2mItem);

	OverlayButtonItem * h3mItem = new OverlayButtonItem (iFactory, 4);
	h3mItem->setBckPixmaps (	"./images/gloverlay/button_SQ_XL_base.png",
							"./images/gloverlay/button_SQ_XL_over.png",
							"./images/gloverlay/camera_zoom_icon.png");
	h3mItem->setClickedPixmap (	"./images/gloverlay/camera_zoom_clicked.png");
	h3mItem->setParamName ("");
	h3mItem->setPosition (QRect (iPos+82, h, 33, 33));
	h3mItem->setPositionEnd (QRect (iPos +82+122, h+37, 33, 33) );
	h3mItem->setOpacity (0.0f);
	iSubitems->push_back (h3mItem);

	OverlayButtonItem * h4mItem = new OverlayButtonItem (iFactory, 5);
	h4mItem->setBckPixmaps (	"./images/gloverlay/button_SQ_XL_base.png",
							"./images/gloverlay/button_SQ_XL_over.png",
							"./images/gloverlay/camera_roll_icon.png");
	h4mItem->setClickedPixmap (	"./images/gloverlay/camera_roll_clicked.png");
	h4mItem->setParamName ("");
	h4mItem->setPosition (QRect (iPos+82, h, 33, 33));
	h4mItem->setPositionEnd (QRect (iPos +82+122+35, h+37, 33, 33) );
	h4mItem->setOpacity (0.0f);
	iSubitems->push_back (h4mItem);

	//camera settings sliders
	OverlaySliderItem * tmItem = new OverlaySliderItem (iFactory, 6);
	tmItem->setData ("Field Of View", iFactory->getParentWidget()->getRenderCameraFOV()*100, QPoint(100,6000), 2);
	tmItem->setPosition (QRect (iPos+82, h, 170, 70));
	tmItem->setPositionEnd (QRect (iPos +82+122+35+35, h, 170, 70) );
	tmItem->setOpacity (0.0f);
	iSubitems->push_back (tmItem);

	OverlaySliderItem * xmItem = new OverlaySliderItem (iFactory, 7);
	xmItem->setData ("Focal Radius", iFactory->getParentWidget()->getRenderCameraRadius()*100, QPoint(0,10000), 2);
	xmItem->setPosition (QRect (iPos+82, h, 170, 70));
	xmItem->setPositionEnd (QRect (iPos +82+122+35+35+172, h, 170, 70) );
	xmItem->setOpacity (0.0f);
	iSubitems->push_back (xmItem);

	return iSubitems->size() != 0;
}

void OverlayCameraSettingsItem::drawInstructions(QPainter *painter, int context, int width, int height)
{
	QString text = tr("");
	QColor defCol(0, 0, 0, 127);
	if(context==0){
		text = tr("Click and drag with the left mouse button "
					   "to rotate the camera, right to zoom, middle to pan.");
	}else
	if(context==1){ //rregion
		text = tr("Click and drag with the left mouse button "
					   "to draw the rregion box, use shift + left button for painting.");
	}
	defCol = QColor (0, 0, 0, 127);

	QFont nfFont("Arial");
	nfFont.setPixelSize( 12 );

	QFontMetrics metrics = QFontMetrics (nfFont);
	int border = qMax(4, metrics.leading());
	QRect rect = metrics.boundingRect(0, 0, width - 2*border, int(height*0.125),
								   Qt::AlignCenter | Qt::TextWordWrap, text);

	painter->setRenderHint(QPainter::TextAntialiasing);
	painter->fillRect(QRect(0, height, width, rect.height() + 2*border), defCol);
	painter->setPen(Qt::white);
	painter->fillRect(QRect(0, 0, width, rect.height() + 2*border), defCol);

	painter->setFont (nfFont);
	painter->drawText((width - rect.width())/2, border,
				   rect.width(), rect.height(),
				   Qt::AlignCenter | Qt::TextWordWrap, text);
}

void OverlayCameraSettingsItem::resetActionButton ()
{
	if(m_active_id == -1) return;

	getSubitem(m_active_id)->setIsActive(false);

	for (int i=0; i<getSubitems()->size(); i++)
		if(i!=m_active_id)
			getSubitem(i)->setIsDimmed (false);

	m_in_action = false;
	m_active_id = -1;


	if(m_rr_active)
	{
		getFactory()->getParentWidget()->setIsRenderRegion (true);
		getFactory()->getParentWidget()->initRenderRegion();
		m_rr_active = false;
	}
}

// paint
void OverlayCameraSettingsItem::paint(QPainter* iPainter)
{
	OverlayAnimSubItem::paint (iPainter);

	if(m_in_action)
	{
		int width = getFactory()->getParentWidget()->getWidgetWidth()-1;
		int height = getFactory()->getParentWidget()->getWidgetHeight()-1;

		//draw intructions
		drawInstructions (iPainter, m_active_id, width, height);

		//draw screen rectangle
		QRect screenRect;
		screenRect.setTopLeft(QPoint(1, 1));
		screenRect.setBottomRight(QPoint(width,height));

		QPen pen;
		pen.setColor (QColor(61,166,244));
		pen.setWidth (1);
		iPainter->setPen (pen);
		iPainter->setBrush(Qt::NoBrush);

		iPainter->drawRect (screenRect);
	}
}

//event handler
bool OverlayCameraSettingsItem::eventFilter (QObject *object, QEvent *event)
{
	bool clickedparent = OverlayAnimSubItem::eventFilter (object, event);

	if(clickedparent) 	// we clicked to get back on parent menu ...
	{					// reset stuff
		resetActionButton ();
		return clickedparent;
	}


	if(m_in_action)
	{
		if (object == getFactory()->getParentWidget() && getFactory()->isEnabled() && !getFactory()->isPainting()
			&&	!this->isHidden())
		{
			switch (event->type())
			{

	        case QEvent::KeyPress:
	        {
				QKeyEvent *ek = (QKeyEvent *) event;
				if(ek->key()==Qt::Key_Escape)
				{
					resetActionButton ();
					return true;
				}
	        } break;

			case QEvent::MouseButtonPress:
			{
				GLViewer * wWidget = getFactory()->getParentWidget();

				QMouseEvent *e = (QMouseEvent *) event;
				int x = e->pos().x() - wWidget->getOffsetWidth();
				int y = e->pos().y() - wWidget->getOffsetHeight();


				if(m_active_id<2)
				{
					switch (m_active_id)
					{
					case 0:
						//!< center camera
						wWidget->centerRenderCamera(x,y);
						break;
					case 1:
						//!< focus camera
						wWidget->focusRenderCamera(x,y);
						break;
					}

					wWidget->resetAccumulation();

					resetActionButton ();

					if(wWidget->isRenderRegion())
						wWidget->setRenderRegionHasInvalidatedContext (true);
				}else
				{
					switch (m_active_id)
					{
					case 2:
						//!< orbit camera
						m_cammouse_mode = 1;
						break;
					case 3:
						//!< pan camera
						m_cammouse_mode = 2;
						break;
					case 4:
						//!< zoom camera
						m_cammouse_mode = 3;
						break;
					case 5:
						//!< roll camera
						m_cammouse_mode = 4;
						break;
					}

	        		if(m_cammouse_mode>0)
	        		{
	        			m_cammouse_tracking = true;

	        			//track last mouse postion ...............
	        			//to update camera move at mousemove event
	        			#ifdef RIGHTHANDLEDCOORDSYS
	        			m_cammouse_clickX = wWidget->getWidgetWidth() - x;
						#else
	        			m_cammouse_clickX = x;
	        			#endif
	        			m_cammouse_clickY = y;
	        		}
				}
				return true;
			} break;


			case QEvent::MouseMove:
			{
				if(!m_cammouse_tracking)return false;

				GLViewer * pWidget = getFactory()->getParentWidget();

				QMouseEvent *e = (QMouseEvent *) event;
				int x = e->pos().x() - pWidget->getOffsetWidth();
				int y = e->pos().y() - pWidget->getOffsetHeight();

				int wWidth = pWidget->getWidgetWidth();

				pWidget->updateRenderCameraMove( m_cammouse_mode, x, y, m_cammouse_clickX, m_cammouse_clickY, wWidth );
				pWidget->resetAccumulation();

				//make lastpos the current one
				//removing this will make mouse moves kinda 'countinous'
				#ifdef RIGHTHANDLEDCOORDSYS
				m_cammouse_clickX = wWidth - x;
				#else
				m_cammouse_clickX = x;
				#endif
				m_cammouse_clickY = y;
			} break;

			case QEvent::MouseButtonRelease:
			{
				if(m_cammouse_tracking)
				{
					if(getFactory()->getParentWidget()->isRenderRegion())
					getFactory()->getParentWidget()->setRenderRegionHasInvalidatedContext (true);
				}

				m_cammouse_tracking = false;
			} break;

			}
		}
	}
    return false;
}

// bind signaling
void OverlayCameraSettingsItem::bind_subitems (int iID)
{
	for (int i=0; i<getSubitems()->size(); i++)
	{
		if(i<6)
		{
			connect (	getSubitem(i), 	SIGNAL 	( button_pressed (bool,int)),
						this, 			SLOT 	( button_pressed_slot (bool,int)) );

		}else
		{
			connect (	getSubitem(i), 	SIGNAL 	( dataChanged (int,int, int)),
						this, 			SLOT 	( valueChanged_slot (int,int, int)) );
		}
	}
}

// bind camera actions
void OverlayCameraSettingsItem::button_pressed_slot (bool pressed, int id)
{
	if(pressed)
	{
		m_in_action = true;
		m_active_id = id;

		for (int i=0; i<getSubitems()->size(); i++)
			if(i!=m_active_id)
				getSubitem(i)->setIsDimmed(true);

		/*GLViewer * gl_widget = getFactory()->getParentWidget();
		if( gl_widget->isRenderRegion() )
		{
			m_rr_active = true;
			gl_widget->setIsRenderRegion (false);
			gl_widget->initRenderRegion();
		}*/
	}else
	{
		resetActionButton ();
	}
}

// bind camera settings
void OverlayCameraSettingsItem::valueChanged_slot (int data, int decdigits, int id)
{
	GLViewer * tViewer = getFactory()->getParentWidget();

	//if(tViewer->getRendererMaxDepth() != data )
	switch (id)
	{
	case 6:	//Field Of View
	{
		float rdata = data / pow(10,decdigits);
		if(tViewer->getRenderCameraFOV() == rdata) return;

		tViewer->setRenderCameraFOV (rdata);
		tViewer->updateRenderCamera();
		tViewer->resetAccumulation ();

		if(tViewer->isRenderRegion())
			tViewer->setRenderRegionHasInvalidatedContext (true);
	} break;
	case 7:	//Focal Radius
	{
		float rdata = data / pow(10,decdigits);
		if(tViewer->getRenderCameraRadius() == rdata) return;

		tViewer->setRenderCameraRadius (rdata);
		tViewer->updateRenderCamera();
		tViewer->resetAccumulation ();

		if(tViewer->isRenderRegion())
			tViewer->setRenderRegionHasInvalidatedContext (true);
	} break;
	}
}


// Tonemapper settings ////////////////////////////////////////////////////////////////////////////////////////

// subitems builder
bool OverlayTonemapperSettingsBuilder::buildItems ( OverlayItemsController * const& iFactory,
													QVector<OverlayItem*> * const& iSubitems,
													int iPos )
{
	iFactory->setActiveDevice(3);

	int h = iFactory->getParentWidget()->getWidgetHeight()-78;

	OverlaySliderItem * amItem = new OverlaySliderItem (iFactory, 0);
	amItem->setData ("Device Gamma", iFactory->getParentWidget()->getTonemapperGamma()*100, QPoint(100,220), 2);
	amItem->setPosition (QRect (iPos+82, h, 170, 70));
	amItem->setPositionEnd (QRect (iPos +82, h, 170, 70) );
	amItem->setOpacity (0.0f);
	iSubitems->push_back (amItem);


	OverlayBoolItem * bmItem = new OverlayBoolItem (iFactory, 1);
	bmItem->setBckPixmaps (		"./images/gloverlay/button_SQ_base.png",
								"./images/gloverlay/button_SQ_over.png",
								"./images/gloverlay/tp_vignette_icon.png");
	bmItem->setClickedPixmap (	"./images/gloverlay/tp_vignette_clicked.png");
	bmItem->setData (iFactory->getParentWidget()->getTonemapperVignetting());
	bmItem->setPosition (QRect (iPos+82, h, 70, 70));
	bmItem->setPositionEnd (QRect (iPos +82+172, h, 70, 70) );
	bmItem->setOpacity (0.0f);
	iSubitems->push_back (bmItem);

	OverlaySliderItem * tmItem = new OverlaySliderItem (iFactory, 2);
	tmItem->setData ("Film-ISO", iFactory->getParentWidget()->getTonemapperSensitivity()*10, QPoint(200,16000), 1);
	tmItem->setPosition (QRect (iPos+82, h, 170, 70));
	tmItem->setPositionEnd (QRect (iPos +82+172+72, h, 170, 70) );
	tmItem->setOpacity (0.0f);
	iSubitems->push_back (tmItem);

	OverlaySliderItem * jmItem = new OverlaySliderItem (iFactory, 3);
	jmItem->setData ("Shutter Time", iFactory->getParentWidget()->getTonemapperExposure()*1000, QPoint(1,30000), 3);
	jmItem->setPosition (QRect (iPos+82, h, 170, 70));
	jmItem->setPositionEnd (QRect (iPos +82+172+72+172, h, 170, 70) );
	jmItem->setOpacity (0.0f);
	iSubitems->push_back (jmItem);

	OverlaySliderItem * vmItem = new OverlaySliderItem (iFactory, 4);
	vmItem->setData ("F-Stop", iFactory->getParentWidget()->getTonemapperFStop()*1000, QPoint(1,12800), 3);
	vmItem->setPosition (QRect (iPos+82, h, 170, 70));
	vmItem->setPositionEnd (QRect (iPos +82+172+72+172+172, h, 170, 70) );
	vmItem->setOpacity (0.0f);
	iSubitems->push_back (vmItem);

	return iSubitems->size() != 0;
}

// bind signaling
void OverlayTonemapperSettingsItem::bind_subitems (int iID)
{
	for (int i=0; i<getSubitems()->size(); i++)
	{
		if(i==1)
		{
			connect (	getSubitem(i), 	SIGNAL 	( button_pressed (bool,int)),
						this, 			SLOT 	( button_pressed_slot (bool,int)) );
		}else
		{
			connect (	getSubitem(i), 	SIGNAL 	( dataChanged (int,int, int)),
						this, 			SLOT 	( valueChanged_slot (int,int, int)) );
		}
	}
}

// bind camera actions
void OverlayTonemapperSettingsItem::button_pressed_slot (bool pressed, int id)
{
	if(id==1) //Vignetting
	getFactory()->getParentWidget()->setTonemapperVignetting (pressed);
}

// bind renderer settings
void OverlayTonemapperSettingsItem::valueChanged_slot (int data, int decdigits, int id)
{
	switch (id)
	{
	case 0:	//Gamma
		getFactory()->getParentWidget()->setTonemapperGamma (data / pow(10,decdigits));
		break;
	case 2:	//Sensitivity
		getFactory()->getParentWidget()->setTonemapperSensitivity (data / pow(10,decdigits));
		break;
	case 3:	//Exposure
		getFactory()->getParentWidget()->setTonemapperExposure (data / pow(10,decdigits));
		break;
	case 4:	//F-Stop
		getFactory()->getParentWidget()->setTonemapperFStop (data / pow(10,decdigits));
		break;
	}
}







///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Main render controller item builder ////////////////////////////////////////////////////////////////////////
bool OverlayRendererCtrlsBuilder::buildItems (	OverlayItemsController * const& iFactory,
												QVector<OverlayItem*> * const& iItems,
												int iPos )
{
	iFactory->setActiveDevice(0);

	int h = iFactory->getParentWidget()->getWidgetHeight()-78;


    //!< Renderer settings
    OverlayRendererSettingsItem * tmItem = new OverlayRendererSettingsItem (iFactory, 0,
												new OverlayRenderSettingsBuilder(),
												QRect (iPos, h, 80, 70),
												QRect (112, h, 80, 70),
												QRect (114, h, 80, 70) );
    tmItem->setBckPixmaps (	"./images/gloverlay/bck_base.png",
							"./images/gloverlay/bck_over.png",
							"./images/gloverlay/renderer_icon.png");
    tmItem->setOpacity (0.0f);
    iItems->push_back (tmItem);


    //!< Camera settings
    OverlayCameraSettingsItem * bmItem = new OverlayCameraSettingsItem (iFactory, 1,
												new OverlayCameraSettingsBuilder(),
												QRect (iPos, h, 80, 70),
												QRect (112+82, h, 80, 70),
												QRect (114, h, 80, 70) );
    bmItem->setBckPixmaps (	"./images/gloverlay/bck_base.png",
							"./images/gloverlay/bck_over.png",
							"./images/gloverlay/camera_icon.png");
    bmItem->setOpacity (0.0f);
    iItems->push_back (bmItem);


    //!< Tonemapper settings
    OverlayTonemapperSettingsItem * xmItem = new OverlayTonemapperSettingsItem (iFactory, 2,
												new OverlayTonemapperSettingsBuilder(),
												QRect (iPos, h, 80, 70),
												QRect (112+82+82, h, 80, 70),
												QRect (94, h, 80, 70) );
    xmItem->setBckPixmaps (	"./images/gloverlay/bck_base.png",
							"./images/gloverlay/bck_over.png",
							"./images/gloverlay/tonemapper_icon.png");
    xmItem->setOpacity (0.0f);
    iItems->push_back (xmItem);


    //!< Environment settings
    OverlayAnimSubItem * wmItem = new OverlayAnimSubItem (iFactory, 3, NULL);
    wmItem->setBckPixmaps (	"./images/gloverlay/bck_base.png",
							"./images/gloverlay/bck_over.png",
							"./images/gloverlay/environment_icon.png");
    wmItem->setPosition (QRect (iPos, h, 80, 70));
    wmItem->setPositionEnd (QRect (112+82+82+82, h, 80, 70) );
    wmItem->setPositionAlone(	QRect (174, h, 80, 70));
    wmItem->setOpacity (0.0f);
    iItems->push_back (wmItem);

	return iItems->size() != 0;
}


