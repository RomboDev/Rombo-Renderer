/*
 * glscreencontrols.cpp
 *
 *  Created on: Jan 18, 2013
 *      Author: max
 */

#include"glscreencontrols.h"


//***********************************************************************************************************//
// Overlay item *********************************************************************************************//
//***********************************************************************************************************//
OverlayItem::OverlayItem (OverlayItemsController * iFactory, int iID) //(QRect iStart, QRect iStop)
: m_factory (iFactory)

, m_qpix_bck (NULL)
, m_qpix_over (NULL)
, m_qpix_icon (NULL)
, m_single_pixmap (true)

, m_id (iID)
, m_is_active (false)
, m_is_dimmed (false)
, m_is_hidden (0)
, m_is_over (false)
, m_opacity (1.0)
, m_end_opacity (1.0)
{}
OverlayItem::~OverlayItem()
{
	m_factory=NULL;
	if(m_qpix_bck!=NULL) 	delete m_qpix_bck;
	if(m_qpix_over!=NULL) 	delete m_qpix_over;
	if(m_qpix_icon!=NULL) 	delete m_qpix_icon;
}

void OverlayItem::setBckPixmaps (const QString & iBck, const QString & iHover, const QString & iIcon)
{
	m_qpix_bck = new QPixmap (iBck);

	if(iHover!="") m_qpix_over = new QPixmap (iHover);
	if(iIcon!="") m_qpix_icon = new QPixmap (iIcon);

	if(iHover=="") 	m_single_pixmap = true;
	else 			m_single_pixmap = false;
}

void OverlayItem::setIconPixmap (const QString & iIcon)
{
	if(iIcon!="")
	{
		m_qpix_icon = new QPixmap (iIcon);

		//set mode to full
		m_single_pixmap = false;

		//ensure we've all the necessary pxmp
		if(m_qpix_over==NULL) m_qpix_over = m_qpix_bck;
	}
}

void OverlayItem::paint (QPainter* iPainter)
{
	if(m_is_hidden) return;

	iPainter->setRenderHint(QPainter::Antialiasing);

	iPainter->setOpacity (m_opacity);

	if(m_single_pixmap)
	{
		iPainter->drawPixmap (m_qpix_rect_cur, *m_qpix_bck);
	}else
	{
		if(!m_is_over)
			iPainter->drawPixmap (m_qpix_rect_cur, *m_qpix_bck);
		else
			iPainter->drawPixmap (m_qpix_rect_cur, *m_qpix_over);

		if(m_qpix_icon)
		iPainter->drawPixmap (m_qpix_rect_cur, *m_qpix_icon);
	}

	iPainter->setOpacity (1.0); //reset opacity
}

bool OverlayItem::eventFilter (QObject *object, QEvent *event)
{
    if (object == m_factory->getParentWidget() && m_factory->isEnabled() && !m_is_hidden  && !isDimmed())
    {
        switch (event->type())
        {
		#ifndef GFXVIEW
        case QEvent::MouseMove:
		#else
        case QEvent::GraphicsSceneMouseMove:
		#endif
        {
        	QMouseEvent *e = (QMouseEvent *) event;
        	int x = e->pos().x();
        	int y = e->pos().y();

			if (m_qpix_rect_cur.intersects (QRect(x, y, 1, 1)))
			{
				m_is_over = true;
			}else
			{
				m_is_over = false;
			}
        } break;
        }
    }
	return false;
}


//***********************************************************************************************************//
// Animated item ********************************************************************************************//
//***********************************************************************************************************//
void OverlayAnimItem::setPositionEnd(const QRect & iPos)
{
	m_pix_rect_last = getPosition();
	m_qpix_rect_end = iPos;
}

void OverlayAnimItem::animateForward (float iTimeElapsed)
{
	m_utime = iTimeElapsed;
	if(m_utime>=0.8f) m_utime=1.0f;
	//if(m_utime>=getEndOpacity()-0.2) m_utime = getEndOpacity();

	//animate
	setCurrentPos (animate (m_utime, m_pix_rect_last, m_qpix_rect_end));

	if(m_utime==1.0){
		m_pix_rect_last = getCurrentPos ();
		setOpacity (getEndOpacity());
	}else{
	setOpacity (m_utime);
	}
}

void OverlayAnimItem::animateBackward (float iTimeElapsed)
{
	m_utime = 1.f - iTimeElapsed;
	if(m_utime<=0.2f) m_utime=0.0f;

	//animate
	setCurrentPos (animate (iTimeElapsed, m_pix_rect_last, getPosition()));

	if(m_utime<=0.2f)
		m_pix_rect_last = getCurrentPos ();

	setOpacity (m_utime);
}

QRect OverlayAnimItem::animate (float iTimeElapsed, const QRect& iStart, const QRect& iEnd)
{
	//linear interpolation (position only atm)
	int x = ( iStart.topLeft().x()+ ((iEnd.topLeft().x() - iStart.topLeft().x()) *iTimeElapsed) );
	int y = ( iStart.topLeft().y()+ ((iEnd.topLeft().y() - iStart.topLeft().y()) *iTimeElapsed) );

	return QRect (x, y, iEnd.width(), iEnd.height());
}


//***********************************************************************************************************//
// Group item ***********************************************************************************************//
//***********************************************************************************************************//
OverlayGroupItem::~OverlayGroupItem()
{
	if(m_subitems.size()>0)
	{
		for(int i=0; i<m_subitems.size(); i++)
		{
			if(m_subitems[i])
			{
	    		getFactory()->removeEventFilter (m_subitems [i]);
				delete m_subitems[i];
			}
		}
		m_subitems.clear();
	}
}

void OverlayGroupItem::addItem (OverlayItem*item)
{
	m_subitems.push_back (item);
	getFactory()->installEventFilter (item);
}

void OverlayGroupItem::paint (QPainter* iPainter)
{
	for(int i=0; i<m_subitems.size(); i++)
	{
		m_subitems.at(i)->paint(iPainter);
	}
}


//***********************************************************************************************************//
// Animated with Subitems item ******************************************************************************//
//***********************************************************************************************************//
OverlayAnimSubItem::~OverlayAnimSubItem()
{
	if(m_subitems.size()>0)
	{
		for(int i=0; i<m_subitems.size(); i++)
		{
			if(m_subitems[i])
			{
	    		getFactory()->removeEventFilter (m_subitems [i]);
				delete m_subitems[i];
			}
		}
		m_subitems.clear();
	}

	if(m_builder)
	delete m_builder;
}

void OverlayAnimSubItem::animateSubitems ()
{
	if(m_subitems.size() == 0) return;

	getFactory()->requestHostAnimation (getID(), &m_subitems);
}

void OverlayAnimSubItem::animateLonely (float iTimeElapsed)
{
	//animate
	setCurrentPos (animate (iTimeElapsed, getLastPos(), m_qpix_rect_alone));

	if(iTimeElapsed>=0.2f)
		setLastPos (getCurrentPos());

	setOpacity (1.0f);
}

void OverlayAnimSubItem::finalizeSubitems ()
{
    //install event filter on items
    for (int i=0; i<m_subitems.size(); i++)
    	if(m_subitems[i])
    		getFactory()->installEventFilter (m_subitems [i]);
}

void OverlayAnimSubItem::setIsHidden (int itis)
{
	OverlayItem::setIsHidden (itis);	//we catch the setIsHidden here
										//so we can hide child subitems
										//when parent is checked in factory.
										//TODO: this may work just accidentaly !!
										//plus that's crap as called from anim !!
	if(!itis)
    for (int i=0; i<m_subitems.size(); i++)
    {
    	if(m_subitems[i])
    	{
    		m_subitems[i]->setIsHidden (1);
    	}
    }
}

void OverlayAnimSubItem::resetSubitems ()
{
	m_anim_toalone_done = false;

	for(int i=0; i<m_subitems.size(); i++)
		if(m_subitems[i])
			delete m_subitems[i];

	m_subitems.clear();
}

void OverlayAnimSubItem::buildSubItems ()
{
	m_anim_toalone_done = true;

	if(m_builder)
	{
		//!< build subitems
		m_builder->buildItems(	getFactory(),
								&m_subitems,
								getPositionAlone().topLeft().x() );
		animateSubitems ();							//subitem intro animation
	}
}

//slot:
void OverlayAnimSubItem::anim_has_ended (int state, int id)
{
	if (id==getID())
	{
		if(state==OverlayItemsController::TOALONE)
		{
			buildSubItems ();

		}else if(state==OverlayItemsController::TOHOST)
		{
			if(m_builder)
			finalizeSubitems(); //install event filter

			emit subitems_ready (getID());
		}
	}
}

void OverlayAnimSubItem::paint (QPainter* iPainter)
{
	if(isHidden()) return;

	OverlayItem::paint (iPainter);							//draw this item

	if(m_subitems_activated && m_anim_toalone_done) 		//draw sub items
	{
		for(int i=0; i<m_subitems.size(); i++)
			m_subitems[i]->paint (iPainter);
	}
}

bool OverlayAnimSubItem::eventFilter (QObject *object, QEvent *event)
{
    if (object == getFactory()->getParentWidget() && getFactory()->isEnabled() && !getFactory()->isPainting()
    	&&	!this->isHidden() && !this->isDimmed())
    {
        switch (event->type())
        {

		#ifndef GFXVIEW
		case QEvent::MouseMove:
		#else
		case QEvent::GraphicsSceneMouseMove:
		#endif
		{
			QMouseEvent *e = (QMouseEvent *) event;
			int x = e->pos().x();
			int y = e->pos().y();

			if (getCurrentPos().intersects (QRect(x, y, 1, 1)))
			{
				this->setIsOver (true);
				//return true; //!< stop event propagation
			}else
			{
				this->setIsOver (false);
			}
		} break;

		#ifndef GFXVIEW
        case QEvent::MouseButtonPress:
		#else
        case QEvent::GraphicsSceneMousePress:
		#endif
        {
        	QMouseEvent *e = (QMouseEvent *) event;
        	int x = e->pos().x();
        	int y = e->pos().y();

			if (getCurrentPos().intersects (QRect(x, y, 1, 1)))
			{
				this->setIsOver (false);

				m_subitems_activated = !m_subitems_activated;
				setIsActive (!isActive());

				if(m_subitems_activated)
				{
					getFactory()->requestAnimation (getID());		//animate to this item alone layout
				}else												//finished this anim (anim_has_ended, m_anim_toalone_done=true)
																	//will animate to display sub-items (animateSubitems)
				{
					resetSubitems();
					getFactory()->requestAnimation (getID(), true);	//back to initial allmain items display layout
				}

				return true; //!< stop event propagation
			}
        } break;
        }
    }
	return false;
}


//***********************************************************************************************************//
// Numeric Pad Item	*****************************************************************************************//
//***********************************************************************************************************//
void OverlayNumericPadDigit::paint (QPainter* iPainter)
{
	OverlayItem::paint(iPainter);

	if(m_qpix_custom!=NULL)
	{
		iPainter->setRenderHint(QPainter::Antialiasing);
		iPainter->drawPixmap (getCurrentPos(), *m_qpix_custom);
	}
	else
	{
		iPainter->setRenderHint(QPainter::TextAntialiasing);
		QFont pFont("Arial");
		pFont.setPixelSize( 12 );
		pFont.setBold (true);

		iPainter->setFont( pFont );
		iPainter->setPen (Qt::white);

		if (m_digit=='0')
		{
			iPainter->drawText (getCurrentPos().left()+28,
								getCurrentPos().top()+20,
								m_digit);

		}else
		{
			iPainter->drawText (getCurrentPos().left()+12,
								getCurrentPos().top()+20,
								m_digit);
		}
	}
}

bool OverlayNumericPadDigit::eventFilter (QObject *object, QEvent *event)
{
	OverlayItem::eventFilter (object, event);

	if(isOver() && event->type()==QEvent::MouseButtonPress)
	{
		emit digit_pressed (getDigit());
		return true; //!< stop event propagation
	}

	return false;
}

//***********************************************************************************************************//
// Numeric Pad Item	*****************************************************************************************//
//***********************************************************************************************************//
void OverlayNumericPad::buildPad ()
{
	qreal iOpacity = 0.8;
	int left = getCurrentPos().left() +7;
	int bottom = getCurrentPos().bottom() -30 -13;

	//zero digit
	OverlayNumericPadDigit * i0DigitButton = new OverlayNumericPadDigit (getFactory(), 0);
	i0DigitButton->setBckPixmaps ( 	"./images/gloverlay/numeric_pad_0digit_base.png",
									"./images/gloverlay/numeric_pad_0digit_over.png" );
	i0DigitButton->setPosition ( QRect (left, bottom, 63,30) );
	i0DigitButton->setDigit ('0');

	i0DigitButton->setOpacity (iOpacity);

	getFactory()->installEventFilter (i0DigitButton);
	m_pad_buttons.push_back (i0DigitButton);

	//1-9 digits
	int stID = 1;
	bottom = bottom -33;
	for (int h=0; h<3; h++)
	{
		for (int w=0; w<3; w++)
		{
			OverlayNumericPadDigit * iDigitButton = new OverlayNumericPadDigit (getFactory(), stID++);
			iDigitButton->setBckPixmaps ( 	"./images/gloverlay/numeric_pad_digit_base.png",
											"./images/gloverlay/numeric_pad_digit_over.png" );
			iDigitButton->setPosition ( QRect (left + (w*33), bottom -(h*33), 30 ,30) );
			iDigitButton->setDigit (47+stID); //ASCII 48=='0', we start from 49, ie. '1'

			iDigitButton->setOpacity (iOpacity);

			getFactory()->installEventFilter (iDigitButton);
			m_pad_buttons.push_back (iDigitButton);
		}
	}

	//right column (bottom-up)
	stID = 10;
	left = left + (3*33);
	bottom = bottom +33;
	for (int h=0; h<4; h++)
	{
		OverlayNumericPadDigit * iDigitButton = new OverlayNumericPadDigit (getFactory(), stID++);
		if (h<2)
			iDigitButton->setBckPixmaps ( 	"./images/gloverlay/numeric_pad_digit_base.png",
											"./images/gloverlay/numeric_pad_digit_over.png" );
		else if(h==2)
			iDigitButton->setBckPixmaps ( 	"./images/gloverlay/numeric_pad_CANCELdigit_base.png",
											"./images/gloverlay/numeric_pad_CANCELdigit_base.png" );
		else if(h==3)
			iDigitButton->setBckPixmaps ( 	"./images/gloverlay/numeric_pad_OKdigit_base.png",
											"./images/gloverlay/numeric_pad_OKdigit_base.png" );
		iDigitButton->setPosition ( QRect (left , bottom -(h*33), 30 ,30) );

		switch (h)
		{
		case 0: //minus
			iDigitButton->setDigit ('-');
			break;
		case 1: //delete last digit
			iDigitButton->setDigit ('D');
			//custom pixmap
			iDigitButton->setCustomPixmap ("./images/gloverlay/numeric_pad_DELdigit_custom.png");
			break;
		case 2: //clear all
			iDigitButton->setDigit ('C');
			//custom pixmap
			iDigitButton->setCustomPixmap ("./images/gloverlay/numeric_pad_CANCELdigit_custom.png");
			break;
		case 3: //confirm
			iDigitButton->setDigit ('X');
			//custom pixmap
			iDigitButton->setCustomPixmap ("./images/gloverlay/numeric_pad_OKdigit_custom.png");
			break;
		}

		iDigitButton->setOpacity (iOpacity);

		getFactory()->installEventFilter (iDigitButton);
		m_pad_buttons.push_back (iDigitButton);
	}

	//dot '.' digit
	left = left -33;
	OverlayNumericPadDigit * iDigitButton = new OverlayNumericPadDigit (getFactory(), stID);
	iDigitButton->setBckPixmaps ( 	"./images/gloverlay/numeric_pad_digit_base.png",
									"./images/gloverlay/numeric_pad_digit_over.png" );
	iDigitButton->setPosition ( QRect (left, bottom, 30,30) );
	iDigitButton->setDigit ('.');

	iDigitButton->setOpacity (iOpacity);

	getFactory()->installEventFilter (iDigitButton);
	m_pad_buttons.push_back (iDigitButton);

	//set signal/slot
	for (int i=0; i<m_pad_buttons.size(); i++)
		connect (m_pad_buttons[i], SIGNAL (digit_pressed (QChar)), this, SLOT (append_digit (QChar)));
}

void OverlayNumericPad::destroyPad ()
{
	for (int i=0; i<m_pad_buttons.size(); i++)
	if(m_pad_buttons[i])
	{
		getFactory()->removeEventFilter (m_pad_buttons[i]);
		disconnect (m_pad_buttons[i], SIGNAL (digit_pressed (QChar)), this, SLOT (append_digit (QChar)));
		delete m_pad_buttons[i];
	}
}

void OverlayNumericPad::paintPad (QPainter* iPainter)
{
	for (int i=0; i<m_pad_buttons.size(); i++)
		m_pad_buttons[i]->paint (iPainter);
}

void OverlayNumericPad::append_digit (QChar idig)
{
	//std::cout << "================================" << std::endl;

	if(idig.isDigit() && !m_dot_needed)
	{
		if(idig=='0' && !m_dot_allowed && m_hold_digits.isEmpty())
		{
			//avoid '0' as first digit for non decimal fields
		}
		else
		{
			//track decimal adding
			int decadd = -2; //if we start adding ints, don't display the dec part

			//set 'dot is needed' if '0' is the first digit
			if(idig=='0' && m_hold_digits.isEmpty()) m_dot_needed = true;

			//limit decimal digits
			if (m_dot_isthere)
			{
				if(m_cur_decdigits < m_dec_digits)
				{
					m_cur_decdigits++;
					decadd = m_cur_decdigits;
				}
				else { return; }
			}else
			{
				if(m_cur_intdigits > 3) return;
				m_cur_intdigits++;
			}

			//append digit to string
			m_hold_digits.append (idig);

			//emit signal for data changing
			float idata = m_hold_digits.toFloat() * pow(10, m_dec_digits);
			emit dataChanging (idata, decadd); //let compiler deal with float vs int
		}
	}
	else if (idig=='.' && (m_dot_allowed && !m_dot_isthere) )
	{
		m_dot_needed = false;
		m_dot_isthere = true;

		if(m_hold_digits.isEmpty())
		{
			//append 0 if we start typing first digit
			//with a dot, inc also cur_intdigits
			m_cur_intdigits++;
			m_hold_digits.append ('0');
			//emit signal for data changing
			emit dataChanging (0,0);
		}else
		{
			//pass anyway the unchanged data with 0 to engage dot display
			float idata = m_hold_digits.toFloat() * pow(10, m_dec_digits);
			emit dataChanging (idata, 0);
		}

		m_hold_digits.append (idig);
	}
	else if (idig=='-' && (m_sign_allowed && !m_sign_isthere) )
	{
		m_sign_isthere = true;
		m_hold_digits.prepend (idig);
	}
	else
	{
		//go for special digits (del char, clear, confirm)

		if (idig=='C') //clear all
		{
			m_dot_needed = false;
			m_dot_isthere = false;
			m_sign_isthere = false;
			m_cur_decdigits = 0;

			m_hold_digits = "";

			//emit original data back,
			//so we used the m_data in the field launcher to update
			//data visually and backed up original data here in m_data
			emit dataChanged (m_data);
		}
		else if (idig=='D') //delete last digit
		{
			//track decimal removing
			int decremove = -1;

			if(m_cur_intdigits==0) //we need to rebuild string to delete something
			{
				//std::cout << "Rebuilding string .. : ";

				//TODO: !! //if we first modify ie by deleting a digit and then engage
				//the slider, we'll get crap !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

				//we may have erased all the digits manually so nothing do to here
				if(m_data_changed) return;

				if(m_dec_digits) //!< decimal number
				{
					float mydec = ((float)m_data) *(1.0f / pow(10, m_dec_digits));
					m_hold_digits = QString::number (mydec);

					//std::cout << m_hold_digits.toStdString() << std::endl;

					int find_the_dot = m_hold_digits.lastIndexOf(QChar('.'));
					if(find_the_dot==-1) 	//ie. not there coz of 'int floats' like 800.0
					{						//rebuild the ie .000 manually

						m_cur_intdigits = m_hold_digits.length(); //set int digits
						m_cur_decdigits = m_dec_digits -1; //set dec digits (-1 we're removing here)

						//add dot
						m_hold_digits.append('.');
						m_dot_isthere = true;

						//add decimal zeros
						for(int i=0; i<m_dec_digits;i++)
							m_hold_digits.append('0');

					}else
					{
						//being zero-based we get the int nb digits
						//from the dot position
						m_cur_intdigits = find_the_dot;

						int str_lenght = m_hold_digits.length();
						int dot_pos = find_the_dot +1; //1-base dot position

						//we may have ending zeros removed for numb like 2.200
						int addedzeros = 0;
						int deltadecs = str_lenght-dot_pos;
						if( deltadecs < (m_dec_digits) )
						{
							//add decimal zeros
							for(int i=0; i<m_dec_digits-deltadecs;i++)
							{
								m_hold_digits.append('0');
								addedzeros++;
							}
						}


						//lenght-dotpos - the one we're removing'
						m_cur_decdigits = (deltadecs -1) +addedzeros;

						if(m_dec_digits>0) m_dot_isthere = true;
					}

					decremove = m_cur_decdigits;
				}
				else //!< integer number
				{
					m_hold_digits = QString::number (m_data);
					m_cur_intdigits = m_hold_digits.length() -1;
				}
			}
			else //!< delete digits from existing string
			{

				if(m_dot_isthere)
				{
					int find_the_dot = m_hold_digits.lastIndexOf(QChar('.'));
					int zLength = m_hold_digits.length()-1;

					//check if we're removing a decimal digit
					//or the decimal dot itself, and reset according
					if( zLength == find_the_dot)
					{
						m_dot_isthere = false;
						decremove = -2;
					}
					else
					{
						m_cur_decdigits--;
						decremove = m_cur_decdigits;
					}

				}else
				{
					m_cur_intdigits--;
				}
			}

			//remove digit
			m_hold_digits.remove (m_hold_digits.length()-1,1);

			//emit signal for data changing
			float idata = m_hold_digits.toFloat() * pow(10, m_dec_digits);
			emit dataChanging (idata, decremove);
		}
		else if (idig=='X') //confirmation
		{
			if(m_cur_intdigits==0 && !m_data_changed)
			{
				emit dataChanged (m_data);
				return;
			}
			//check limit range
			float intdata = m_hold_digits.toFloat() * pow(10, m_dec_digits);

			if(intdata<m_vrange.x()) 		intdata = m_vrange.x();
			else if(intdata>m_vrange.y()) 	intdata = m_vrange.y();

			emit dataChanged (intdata);
		}
	}

	//data changed once, pad initialized
	m_data_changed = true;

/*
	//std::cout << "Got pad digit: " << QString(idig).toStdString() << std::endl;
	std::cout << "Got pad digit: " << idig.toAscii() << std::endl;
	std::cout << "isDigit (" << idig.isDigit() << ")"<< std::endl;
	std::cout << "Appended to string: " << m_hold_digits.toStdString() << std::endl;

	std::cout << "Int nb digits: " << m_cur_intdigits << std::endl;
	std::cout << "Decimal nb digits: " << m_cur_decdigits << std::endl;
	std::cout << "Dot is there: " << m_dot_isthere << std::endl;

	float xx = m_hold_digits.toFloat() * pow(10, m_dec_digits);
	std::cout << "String int data: " << xx << std::endl;

	std::cout << "Original int data: " << m_data << std::endl;
	std::cout << "Range.min: " << m_vrange.x() << ", Range.max: " << m_vrange.y() << std::endl;
*/
}


//***********************************************************************************************************//
// Slider Item	*********************************************************************************************//
//***********************************************************************************************************//
int OverlaySliderItem::getXSliderFromValue (int val, bool toInt)
{
	int value;
	if(val==-666)
	{
		value = m_data;

		if(value<m_vrange.x()) 		value = m_vrange.x();
		else if(value>m_vrange.y()) value = m_vrange.y();
	}
	else{
		value = val;
	}

	int vMin = m_vrange.x();
	int vMax = m_vrange.y();
	int vDiff  = vMax - vMin;

	int pMin = getCurrentPos().left() +WOFFSET;
	int pMax = pMin + getCurrentPos().width()-(WOFFSET*2);
	int pDiff  = pMax - pMin; //ie. m_slider_width :)

	//Find how far we are into the first range, scale
	//that distance by the ratio of sizes of the ranges,
	//and that's how far we should be into the second range
	int pValue = pMin + ((value - vMin) * pDiff) / vDiff;
	return pValue;
}

int OverlaySliderItem::getValueFromXSlider ()
{
	int value = m_scursor_rect.center().x();

	int vMin = getCurrentPos().left() +WOFFSET;
	int vMax = vMin + getCurrentPos().width()-(WOFFSET*2);
	int vDiff  = vMax - vMin;

	int pMin = m_vrange.x();
	int pMax = m_vrange.y();
	int pDiff  = pMax - pMin;

	//remap range
	int pValue = pMin + ((value - vMin) * pDiff) / vDiff;
	return pValue;
}

void OverlaySliderItem::clearNumericPad ()
{
	getFactory()->removeEventFilter (m_num_pad);
	delete m_num_pad;
	m_num_pad = NULL;
	m_numpad_clicked = false;
	m_numpad_destroy = false;

	getFactory()->setNumPadSlotFilled (false);
}

void OverlaySliderItem::buildNumericPad ()
{
	if(!getFactory()->getNumPadSlotFilled()) 	//check if pad slot is not already used in
	{											//by another item

		getFactory()->setNumPadSlotFilled(true);//set it now then

		//numeric_pad_base
		m_numpad_clicked = true;
		m_num_pad = new OverlayNumericPad (getFactory(), 789);
		m_num_pad->setBckPixmaps ( "./images/gloverlay/numeric_pad_base.png" );
		m_num_pad->setPosition (QRect (m_numfield_rect.left()+(m_numfield_rect.width()/2)-71,
								m_numfield_rect.top()-162,
								143, 150));
		m_num_pad->setOpacity (0.8);

		m_num_pad->setFormat (	this->hasNegativeRange(),
								this->hasDecimals(),
								this->getDecDigitsNb() );
		m_num_pad->setData (	m_data,
								m_vrange );
		//std::cout << "Passing data to numpad: " << m_data << std::endl;

		if(m_show_numpad)
			m_num_pad->setHiddenPad();

		m_num_pad->buildPad();
		getFactory()->installEventFilter (m_num_pad);



		connect (m_num_pad, SIGNAL (dataChanged(int)), this, SLOT (dataPadChanged(int)));
		connect (m_num_pad, SIGNAL (dataChanging(int,int)), this, SLOT (dataPadChanging(int,int)));

		//connect (this, SIGNAL (dataChanged(int,int,int)), this, SLOT (updatePadData(int,int,int)));
	}
}

void OverlaySliderItem::paint (QPainter* iPainter)
{
	OverlayAnimItem::paint (iPainter);			//!< paint background

	//TODO:: OPTIMIZE HERE !!!!!!!!!!!!!!!!!!!!!!!
	//finished painting, do the computation in the
	//dataChange related stuff ...................

	if (getFactory()->isPainting() || m_numpad_updating || m_force_repaint)
	{
		if(m_has_slider)
		{
			//base slider track
			QPoint bsTopLeft ( getCurrentPos().left()+WOFFSET, getCurrentPos().center().y()+HOFFSET );
			m_bslider_rect = QRect (bsTopLeft, QSize(getCurrentPos().width()-(WOFFSET*2), HOFFSET*2));

			//slider cursor
			QPoint cc (getXSliderFromValue(), m_bslider_rect.center().y());
			int top = cc.y() -10;
			int left = cc.x() -5;
			m_scursor_rect = QRect(left,top, 10, 20+2);
		}

		//numerical field
		QPoint nfTopLeft ( (getCurrentPos().left() + (getCurrentPos().width()/4)*3)-8,
							getCurrentPos().top()+10);
		m_numfield_rect = QRect ( nfTopLeft, QSize (((getCurrentPos().width()-10)/4)+2, 16));

		if (!getFactory()->isPainting()) m_numpad_updating = false;

		m_force_repaint = false;
	}

	if(m_has_slider)
	{
		//draw above stuff
		iPainter->setBrush (QColor(34, 34, 34));	//base slider track
		iPainter->setPen (Qt::white);
		iPainter->drawRect (m_bslider_rect);

		int zeroval = 0;
		if(m_vrange.x()>0) zeroval=m_vrange.x();
		int acX = getXSliderFromValue (zeroval);	//cursor active rect
		int scX = m_scursor_rect.center().x();
		if(scX<acX)
		{
			QPoint ppTopLeft 		(scX, m_bslider_rect.top()+1);
			QPoint ppBottomRight 	(acX, m_bslider_rect.bottom()+0);
			m_activefield_rect = QRect (ppTopLeft, ppBottomRight);
		}
		else if (scX>acX)
		{
			QPoint ppTopLeft 		(acX, m_bslider_rect.top()+1);
			QPoint ppBottomRight 	(scX, m_bslider_rect.bottom()+0);
			m_activefield_rect = QRect (ppTopLeft, ppBottomRight);
		}
		else if (scX==acX)
		{
			QPoint ppTopLeft 		(acX, m_bslider_rect.top()+1);
			QPoint ppBottomRight 	(acX, m_bslider_rect.bottom()+0);
			m_activefield_rect = QRect (ppTopLeft, ppBottomRight);
		}

		iPainter->setBrush (QColor(61, 166, 244));
		iPainter->setPen (Qt::NoPen);
		iPainter->drawRect (m_activefield_rect);

		iPainter->setBrush (Qt::white);				//slider cursor
		iPainter->drawRect (m_scursor_rect);
		iPainter->setBrush (Qt::NoBrush);
	}

	//numerical field bck /////////////////////////////////////////////////////////
	if(!m_numpad_clicked)
		iPainter->setBrush (QColor(34, 34, 34));//numerical field
	else
		iPainter->setBrush (QColor(250, 250, 250));
	iPainter->setPen (Qt::white);
	iPainter->drawRect (m_numfield_rect);

	//numerical field text ////////////////////////////////////////////////////////
	iPainter->setRenderHint(QPainter::TextAntialiasing);
	QFont nfFont("Arial");
	nfFont.setPixelSize( 12 );
	if(!m_numpad_clicked)
	{
		iPainter->setPen (Qt::white);
	}
	else
	{
		nfFont.setBold (true);
		iPainter->setPen (QColor(61, 166, 244));
	}
	iPainter->setFont( nfFont );

	QString nfText;
	if(m_decdigits != 0)
	{
		float mydec = ((float)m_data) *m_dec_inv_pow;

		//TODO:find a better way and a better place to ..
		//limit decimal digits to stay into the 5 digits limit
		int decdigits = m_decdigits;
		if(mydec>99 && decdigits>=3) decdigits = m_decdigits;
		else if(mydec>999 && decdigits>=2) decdigits--;

		//deal with displaing decimal digits and dot
		//while editing from numpad
		if(m_numpad_decdigits==-1)
		{
			nfText = QString::number (mydec, 'f', m_decdigits);
		}
		else
		{
			if(m_numpad_decdigits>=0)
			{
				nfText = QString::number (mydec, 'f', m_numpad_decdigits);
				if(m_numpad_decdigits==0) nfText.append('.');

			}else
			{
				nfText = QString::number (mydec, 'f', 0);
			}
		}
	}else
	{
		nfText = QString::number (m_data);
	}

	iPainter->drawText (m_numfield_rect.topLeft().x()+2,
						m_numfield_rect.topLeft().y()+12,
						nfText);


	//text property ///////////////////////////////////////////////////////////////
	QFont pFont("Arial");
	pFont.setPixelSize( 11 );
	pFont.setBold (true);
	iPainter->setFont( pFont );

	iPainter->setPen (Qt::white);
	//QString pText = tr("Min. Contribution");
	iPainter->drawText (getCurrentPos().left()+WOFFSET,
						getCurrentPos().top()+22,
						m_paramname);


	//numeric pad /////////////////////////////////////////////////////////////////
	if(m_numpad_clicked && m_num_pad!=NULL)
	{
		if(!m_numpad_destroy) 	m_num_pad->paint(iPainter);
		else 					clearNumericPad(); //!< destroy numeric pad
	}
}

bool OverlaySliderItem::eventFilter (QObject *object, QEvent *event)
{
    if (object == getFactory()->getParentWidget() && getFactory()->isEnabled() && !getFactory()->isPainting() &&
    		!this->isHidden() && !this->isDimmed())
    {
        switch (event->type())
        {

        case QEvent::KeyPress:
        {
        	QPoint mpos = getFactory()->getParentWidget()->mapFromGlobal (QCursor::pos());
			if (!getCurrentPos().intersects (QRect(mpos.x(), mpos.y(), 1, 1)))	//over whole item rect
			return false;

			QKeyEvent *ek = (QKeyEvent *) event;

			//TODO:REVISIT INCREMENTAL UNIT STRATEGY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			if(ek->key()==Qt::Key_Left)
			{
				if(ek->modifiers()==Qt::ShiftModifier)
				{
					m_data -= 10;
					if(m_data<m_vrange.x())	m_data=m_vrange.x();
					m_scursor_rect.moveCenter (QPoint(getXSliderFromValue (m_data), m_scursor_rect.center().y()));

				}else
				{
					m_data -= 1;
					if(m_data<m_vrange.x())	m_data=m_vrange.x();
					m_scursor_rect.moveCenter (QPoint(getXSliderFromValue (m_data), m_scursor_rect.center().y()));
				}
			}else
			if(ek->key()==Qt::Key_Right)
			{
				if(ek->modifiers()==Qt::ShiftModifier)
				{
					m_data += 10;
					if(m_data>m_vrange.y()) m_data=m_vrange.y();
					m_scursor_rect.moveCenter (QPoint(getXSliderFromValue (m_data), m_scursor_rect.center().y()));

				}else
				{
					m_data += 1;
					if(m_data>m_vrange.y()) m_data=m_vrange.y();
					m_scursor_rect.moveCenter (QPoint(getXSliderFromValue (m_data), m_scursor_rect.center().y()));
				}
			}else
			if(ek->key()==Qt::Key_Up)
			{
				int nval = ceilf (float(m_data*m_dec_inv_pow)) * pow(10, m_decdigits);
				if(m_data==nval) nval+=1 * pow(10, m_decdigits);

				m_data = nval;

				if(m_data>m_vrange.y())
					m_data=m_vrange.y();
				m_scursor_rect.moveCenter (QPoint(getXSliderFromValue (m_data), m_scursor_rect.center().y()));
			}else
			if(ek->key()==Qt::Key_Down)
			{
				int nval = floorf (float(m_data*m_dec_inv_pow)) * pow(10, m_decdigits);
				if(m_data==nval) nval-=1 * pow(10, m_decdigits);

				m_data = nval;

				if(m_data<m_vrange.x())
					m_data=m_vrange.x();

				m_scursor_rect.moveCenter (QPoint(getXSliderFromValue (m_data), m_scursor_rect.center().y()));
			}else
			if(ek->key()==Qt::Key_Space) 						//!bring up numeric pad
			{
				if(m_num_pad==NULL)
						buildNumericPad ();
				else 	m_num_pad->append_digit ('C');
				return false;
			}else
			{
				return false;
			}

			emit dataChanged (m_data, m_decdigits, getID()); 	//!< emit data changed
			updatePadData(m_data); 								//update numpad if there
        } break;


		#ifndef GFXVIEW
        case QEvent::MouseButtonPress:
		#else
        case QEvent::GraphicsSceneMousePress:
		#endif
        {
        	QMouseEvent *e = (QMouseEvent *) event;
        	int x = e->pos().x();
        	int y = e->pos().y();

        	QRect eRect (x, y, 1, 1);
			if (getCurrentPos().intersects (eRect))	//over whole item rect
			{
				if (m_over_cursor)
				{
					m_draggin_cursor = true;
					return true; //!< stop event propagation
				}else
				if (m_numfield_rect.intersects (eRect)) //over numeric input field
				{
					if (!m_numpad_clicked)
					{
						buildNumericPad ();
					}else
					{
						clearNumericPad (); //!< destroy numeric pad
					}
				}else
				if (m_has_slider && m_bslider_rect.intersects (eRect)) //over background slider rect
				{
	        		m_scursor_rect.moveCenter (QPoint(x, m_scursor_rect.center().y()));

	        		m_data = getValueFromXSlider();						//!< update data

	        		if(m_data<m_vrange.x()) m_data=m_vrange.x();
	        		if(m_data>m_vrange.y()) m_data=m_vrange.y();


	        		emit dataChanged (m_data, m_decdigits, getID()); 	//!< emit data changes
	        		updatePadData(m_data);								//update numpad if there
	        		return true; //!< stop event propagation
				}

				return true; //!< if over slider widget, stop event propagation
			}
        } break;

		#ifndef GFXVIEW
        case QEvent::MouseMove:
		#else
        case QEvent::GraphicsSceneMouseMove:
		#endif
        {
        	if(!m_has_slider) return false;

        	QMouseEvent *e = (QMouseEvent *) event;
        	int x = e->pos().x();
        	int y = e->pos().y();

        	if(!m_draggin_cursor)							//!< check if we're dragging
        	{
				QRect eRect (x, y, 1, 1);
				if (getCurrentPos().intersects (eRect)) 	//!< over whole item rect
				{
					if (m_scursor_rect.intersects (eRect))	//!< over cursor slider
					{
						if(m_override_cur_once)
						{
							m_override_cur_once = false;
							m_restore_cur_once = true;
							QApplication::setOverrideCursor(QCursor(Qt::PointingHandCursor));
							m_over_cursor = true;
						}
					}else
					{
						if(m_restore_cur_once)
						{
							m_override_cur_once = true;
							m_restore_cur_once = false;
							QApplication::restoreOverrideCursor();
							m_over_cursor = false;
						}
					}
				}else
				{
					if(m_restore_cur_once)
					{
						m_override_cur_once = true;
						m_restore_cur_once = false;
						QApplication::restoreOverrideCursor();
						m_over_cursor = false;
					}
				}
        	}else											//!< dragging slider cursor
        	{
        		if(x<m_bslider_rect.left()) x = m_bslider_rect.left();
        		if(x>(m_bslider_rect.right()+1)) x = m_bslider_rect.right()+1;

        		m_scursor_rect.moveCenter (QPoint(x, m_scursor_rect.center().y()));

        		m_data = getValueFromXSlider();				//!< update data

        		if(m_data<m_vrange.x()) m_data=m_vrange.x();
        		if(m_data>m_vrange.y()) m_data=m_vrange.y();


        		emit dataChanged (m_data, m_decdigits, getID()); 	//!< emit data changes
        		updatePadData(m_data);								//update numpad if there
        		return true; //!< stop event propagation
        	}
        } break;


#ifndef GFXVIEW
        case QEvent::MouseButtonRelease:
#else
        case QEvent::GraphicsSceneMouseRelease:
#endif
        {
        	m_draggin_cursor = false;
        } break;
        }
    }
	return false;
}


//***********************************************************************************************************//
// Button/Picker item	*************************************************************************************//
//***********************************************************************************************************//
void OverlayButtonItem::paint (QPainter* iPainter)
{
	if(!isActive())
	{
		OverlayAnimItem::paint (iPainter);			//!< paint background
	}
	else
	{
		if(m_qpix_click != NULL)
		{
			iPainter->setRenderHint(QPainter::Antialiasing);
			iPainter->drawPixmap (getCurrentPos(), *m_qpix_click);
		}
	}

	if(!m_paramname.isEmpty() || !m_paramname.isNull())
	{
		iPainter->setRenderHint(QPainter::TextAntialiasing);
		QFont nfFont("Arial");
		nfFont.setPixelSize( 12 );
		iPainter->setFont( nfFont );
		iPainter->setPen (Qt::white);

		iPainter->drawText (getCurrentPos().left()+32,
							getCurrentPos().top()+21,
							m_paramname);
	}
}

bool OverlayButtonItem::eventFilter (QObject *object, QEvent *event)
{
	if(!isActive())
	OverlayItem::eventFilter (object, event);


    if (object == getFactory()->getParentWidget() && getFactory()->isEnabled() && !getFactory()->isPainting() &&
    		!this->isHidden()  && !this->isDimmed())
    {
        switch (event->type())
        {
		case QEvent::MouseButtonPress:
		{
			QMouseEvent *e = (QMouseEvent *) event;
			int x = e->pos().x();
			int y = e->pos().y();

			if (getCurrentPos().intersects (QRect(x, y, 1, 1)))	//over whole item rect
			{
				setIsActive (!isActive());
				emit button_pressed (isActive(), getID());

				return true; //!< stop event propagation
			}
		} break;
        }
    }

    return false;
}


//***********************************************************************************************************//
// Bool item	*********************************************************************************************//
//***********************************************************************************************************//
void OverlayBoolItem::paint (QPainter* iPainter)
{
	if(!m_bool_data)
	{
		OverlayAnimItem::paint (iPainter);			//!< paint background
	}
	else
	{
		if(m_qpix_click != NULL)
		{
			iPainter->setRenderHint(QPainter::Antialiasing);
			iPainter->drawPixmap (getCurrentPos(), *m_qpix_click);
		}
	}
}

bool OverlayBoolItem::eventFilter (QObject *object, QEvent *event)
{
	if(!m_bool_data)
	OverlayItem::eventFilter (object, event);


    if (object == getFactory()->getParentWidget() && getFactory()->isEnabled() && !getFactory()->isPainting() &&
    		!this->isHidden()  && !this->isDimmed())
    {
        switch (event->type())
        {
		case QEvent::MouseButtonPress:
		{
			QMouseEvent *e = (QMouseEvent *) event;
			int x = e->pos().x();
			int y = e->pos().y();

			if (getCurrentPos().intersects (QRect(x, y, 1, 1)))	//over whole item rect
			{
				m_bool_data = !m_bool_data;
				emit button_pressed (m_bool_data, getID());

				return true; //!< stop event propagation
			}
		} break;
        }
    }

    return false;
}



//***********************************************************************************************************//
// Navigator item	*****************************************************************************************//
//***********************************************************************************************************//
/*
int OverlayNavigatorItem::remapBoxWidth (const QSize& isize, int hfix)
{
	//this shoundn't be ever true
	if(isize.width()==0 | isize.height()==0) return hfix;

	//hfix::isize.height() = w::isize.width()
	int w = (hfix * isize.width()) / isize.height();
	return w;
}

void OverlayNavigatorItem::factoryResized ()
{
	GLViewer* mWidget = getFactory()->getParentWidget();

	if(mWidget->fbNeedsNavigator())
	{ 	this->setIsHidden (false); }
	else
	{
		this->setIsHidden (true);
		return;
	}


	float zFactor = 1.0f;
    if(mWidget->getFbZoomMode())
    {
    	zFactor *= ((float)mWidget->getFbScenePercent() / (float)100);
    	zFactor *= ((float)mWidget->getFbScenePercent() / (float)100);
    }
	int fbWidth = mWidget->getFbSceneWidth() *zFactor;
	int fbHeight = mWidget->getFbSceneHeight() *zFactor;

	//we start with the item height and from there we get
	//the width based on widget proportions
	QRect qRect = this->getCurrentPos();
	int width = remapBoxWidth (QSize(fbWidth,fbHeight), qRect.height());
	qRect.setWidth (width);
	this->setCurrentPos (qRect);

	//get subregion size
	float dW = (float)mWidget->getWidgetWidth() / (float)fbWidth;

	if(dW>1) dW = 1.0f;
	float dH = (float)mWidget->getWidgetHeight() / (float)fbHeight;
	if(dH>1) dH = 1.0f;

	int subwidth = qRect.width() *dW;
	int subheight = qRect.height() *dH;

	//set subregion rect
	m_subregion = QRect (qRect.left(),qRect.top(),subwidth,subheight);
	m_subregion.moveCenter (qRect.center());
};
*/
qreal OverlayNavigatorItem::remapBoxWidth (const QSizeF& isize, int hfix)
{
	//this shoundn't be ever true
	if(isize.width()==0 | isize.height()==0) return hfix;

	//hfix::isize.height() = w::isize.width()
	int w = (hfix * isize.width()) / isize.height();
	return w;
}
void OverlayNavigatorItem::factoryResized ()
{
	GLViewer* mWidget = getFactory()->getParentWidget();

	if(mWidget->fbNeedsNavigator())
	{ 	this->setIsHidden (false); /*unhide navigator*/ }
	else
	{
		this->setIsHidden (true);
		return;
	}


	qreal zFactor = 1.0f;
    if(mWidget->getFbZoomMode())
    {
    	zFactor *= ((qreal)mWidget->getFbScenePercent() / (qreal)100);
    	zFactor *= ((qreal)mWidget->getFbScenePercent() / (qreal)100);
    }
    qreal fbWidth = mWidget->getFbSceneWidth() *zFactor;
    qreal fbHeight = mWidget->getFbSceneHeight() *zFactor;

	//we start with the item height and from there we get
	//the width based on widget proportions
	QRect qRect (this->getCurrentPos());
	qreal width = remapBoxWidth (QSize(fbWidth,fbHeight), qRect.height());
	qRect.setWidth (width);
	this->setCurrentPos (qRect);

	//get subregion size
	qreal dW = (float)mWidget->getWidgetWidth() / (float)fbWidth;

	if(dW>1) dW = 1.0f;
	qreal dH = (float)mWidget->getWidgetHeight() / (float)fbHeight;
	if(dH>1) dH = 1.0f;

	qreal subwidth = qRect.width() *dW;
	qreal subheight = qRect.height() *dH;

	//set subregion rect
	m_subregion = QRectF (qRect.left(),qRect.top(),subwidth,subheight);
	m_subregion.moveCenter (qRect.center());
};

void OverlayNavigatorItem::global_anim_ended_slot (int state, int id)
{
	if(state==OverlayItemsController::TODISPLAY)
	{
		//resize navigator based on widget
		//connect resize factory slot to stay up to date
		factoryResized ();
		connect (getFactory(), 	SIGNAL (resized()),
				 this, 			SLOT (factoryResized()) );
	}
}

void OverlayNavigatorItem::paint (QPainter* iPainter)
{
	if(isHidden()) return;

	OverlayAnimItem::paint (iPainter);

	//draw subregion rect
	//qreal oOpacity = iPainter->opacity();
	//iPainter->setOpacity (0.4);
	iPainter->setBrush (QColor(250,55,55,100)/*Qt::darkGray*/);
	iPainter->setPen (QColor(255,255,255,100)/*Qt::white*/);

	iPainter->drawRect (m_subregion);
	//iPainter->setOpacity (oOpacity);
}

bool OverlayNavigatorItem::eventFilter (QObject *object, QEvent *event)
{
    if (object == getFactory()->getParentWidget() && getFactory()->isEnabled() && !getFactory()->isPainting() &&
    		!this->isHidden()  && !this->isDimmed())
    {
        switch (event->type())
        {
		case QEvent::MouseButtonPress:
		{
			if(m_force_blockme<0)
			{
				m_force_blockme ++;
				return true;;
			}

			QMouseEvent *e = (QMouseEvent *) event;
			int x = e->pos().x();
			int y = e->pos().y();

			//press over subregion rect
			if (m_subregion.contains (x, y))
			{
				//start draggin
				m_rr_isdragging = true;

				if(getFactory()->getParentWidget()->getRenderState()<1)
					m_renderer_alredy_paused = true;
				else
					getFactory()->getParentWidget()->pauseRenderer();

				QApplication::setOverrideCursor(QCursor(Qt::ClosedHandCursor));
				return true; //!< stop event propagation
			}
		} break;
		case QEvent::MouseMove:
		{
			if(m_rr_isdragging)
			{
				GLViewer* mWidget = getFactory()->getParentWidget();

				QMouseEvent *me = (QMouseEvent *) event;
				qreal dX = me->pos().x();
				qreal dY = me->pos().y();


				qreal zFactor = 1.0f;
			    if(mWidget->getFbZoomMode())
			    {
			    	zFactor *= ((qreal)mWidget->getFbScenePercent() / (qreal)100);
			    	zFactor *= ((qreal)mWidget->getFbScenePercent() / (qreal)100);
			    }
			    qreal fb_scene_width = mWidget->getFbSceneWidth() *zFactor;
			    qreal fb_scene_height = mWidget->getFbSceneHeight() *zFactor;

				//check boundaries (this very item rect)
				if(fb_scene_width > mWidget->getWidgetWidth())
				{
					//we move from center, so get half subregion
					//and check borders from there
					qreal subWidth = m_subregion.width()/2.0;

					if((dX-subWidth)<getCurrentPos().left()) 		dX = getCurrentPos().left() 	+ subWidth;
					if((dX+subWidth)>getCurrentPos().right()) 		dX = getCurrentPos().right() 	- subWidth;
				}else
				{
					//stick to center if no negative offset
					dX = getCurrentPos().center().x();
				}

				if(fb_scene_height > mWidget->getWidgetHeight())
				{
					qreal subHeight = m_subregion.height()/2.0;

					if((dY-subHeight)<getCurrentPos().top()) 		dY = getCurrentPos().top() 		+ subHeight;
					if((dY+subHeight)>getCurrentPos().bottom()) 	dY = getCurrentPos().bottom() 	- subHeight;
				}else
				{
					dY = getCurrentPos().center().y();
				}

				//move subregion
				m_subregion.moveCenter(QPoint(dX,dY));

				//get subregion offset
				qreal wSubOffset = m_subregion.left() - getCurrentPos().left();
				qreal hSubOffset = m_subregion.top() - getCurrentPos().top();

				//get the offset in fb coords
				qreal wFbOffset = (fb_scene_width * wSubOffset) / getCurrentPos().width();
				qreal hFbOffset = (fb_scene_height * hSubOffset) / getCurrentPos().height();

/*
				std::cout 	<< "widgetSize " 	<< mWidget->getWidgetWidth() << ", " << mWidget->getWidgetHeight() << std::endl;
				std::cout 	<< "sceneSize " 	<< mWidget->getFbSceneWidth() << ", " << mWidget->getFbSceneHeight() << std::endl;
				std::cout 	<< "sceneOffset " 	<< mWidget->getOffsetWidth() << ", " << mWidget->getOffsetHeight() << std::endl;


				std::cout 	<< "itemRect: " << getCurrentPos().left() << ", " << getCurrentPos().top() << ", "
							<< getCurrentPos().right() << ", " << getCurrentPos().bottom() << " (" << getCurrentPos().width() << "," << getCurrentPos().height() << ")" << std::endl;
				std::cout 	<< "subRegion: " << m_subregion.left() << ", " << m_subregion.top() << ", "
							<< m_subregion.right() << ", " << m_subregion.bottom() << " (" << m_subregion.width() << "," << m_subregion.height() << ")" << std::endl;

				std::cout 	<< "SubOffsets "	<< wSubOffset << ", "	<< hSubOffset << ", "
												<< wFbOffset << ", "	<< hFbOffset << std::endl;

				std::cout << "========================================================" << std::endl;
*/

				//invert the offset, ie. from right to left
				//(TODO:check if that belongs to righthand coords or OpenGL !!!!!!!!!!!!!!!)
				wFbOffset = fb_scene_width - (wFbOffset + mWidget->getWidgetWidth());

				//consistency check for small int discrepancies from above
				int maxWoffset = fb_scene_width-mWidget->getWidgetWidth(); //max w offset
				if( wFbOffset > maxWoffset) wFbOffset = maxWoffset;
				if( wFbOffset < 0) wFbOffset = 0;
				//only zero or negative, for positive offs (fb contained into widget size),
				//we shoudn't be here anyway
				if(mWidget->getOffsetWidth()<=0) mWidget->setOffsetWidth (-(wFbOffset));

				//H offset
				int maxHoffset = fb_scene_height-mWidget->getWidgetHeight();
				if( hFbOffset > maxHoffset) hFbOffset = maxHoffset;
				if( hFbOffset < 0) hFbOffset = 0;
				if(mWidget->getOffsetHeight()<=0) mWidget->setOffsetHeight (-(hFbOffset));

				mWidget->updateRenderRegion();

				return true; //!< stop event propagation
			}
		} break;
		case QEvent::MouseButtonRelease:
		{
			if(m_rr_isdragging)
			{
				m_rr_isdragging = false;

				if(!m_renderer_alredy_paused)
					getFactory()->getParentWidget()->restartRenderer();
				m_renderer_alredy_paused = false;

				QApplication::restoreOverrideCursor();
			}
		} break;
        }
    }

	return false;
};



//***********************************************************************************************************//
// Controller factory	*************************************************************************************//
//***********************************************************************************************************//
#ifndef GFXVIEW
OverlayItemsController::OverlayItemsController (GLViewer *widget, OverlayItemsBuilder * iBuilder)
#else
OverlayItemsController::OverlayItemsController (QGraphicsScene *widget, OverlayItemsBuilder * iBuilder)
#endif
: m_widget (widget)
, m_builder (iBuilder)
, m_host_ctrls (NULL)

, m_qpixmap (NULL)
, m_timer (NULL)

, m_is_enabled (false)
, m_is_painting (false)
, m_anim_cur_item (-1)
, m_anim_last_item (-1)
, m_elapsed (0)
, m_anim_todisplay (false)
, m_anim_mode (0)
, m_anim_state (-1)

, m_deleteon_toback_anim (false)
, m_numpad_slot_filled (false)

, m_layout_type (0)
, m_items_startpos (56)

, m_hasno_subitems (false)
{
	m_widget_size.setWidth (m_widget->getWidgetWidth());
	m_widget_size.setHeight (m_widget->getWidgetHeight());

	//!< register this device
	registerDevice ();

    //global animation timer
    m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(anim_ctrl()));
}

OverlayItemsController::~OverlayItemsController()
{
	if(m_qpixmap) delete m_qpixmap;
	delete m_timer;

	for(int i=0; i<m_ctrls.size(); i++)
		delete m_ctrls[i];
	m_ctrls.clear();

	m_host_ctrls =NULL;
	m_widget =NULL;
}

void OverlayItemsController::registerDevice ()
{
	m_widget->installEventFilter(this);

	connect (this, 		SIGNAL (devicePainting(bool)),
			m_widget, 	SLOT (deviceIsPainting(bool)) );

	//connect (m_widget, 	SIGNAL (init()),
	//		this, 		SLOT (viewerInit()) );
	connect (m_widget, 	SIGNAL (resized()),
			this, 		SLOT (viewerResized()) );
	connect (m_widget, 	SIGNAL (painting(QPainter*)),
			this, 		SLOT (viewerPaint(QPainter*)) );
}

//void OverlayItemsController::viewerResized (int iwidth, int iheight)
void OverlayItemsController::viewerResized ()
{
	//this is for items that may belong to a widget/fb resize
	emit resized ();


	//skip it if the widget size is the same, ie. we just resizeGL
	//the framebuffer between fullwidget the framed fb modes
	if( m_widget_size.width() == m_widget->getWidgetWidth() &&
		m_widget_size.height() == m_widget->getWidgetHeight() )
	return;

	//update local widget size (we use it only for the comparison above)
	int oHeight = m_widget_size.height();
	m_widget_size.setWidth (m_widget->getWidgetWidth());
	m_widget_size.setHeight (m_widget->getWidgetHeight());

	//this actually updates the respective layouts with
	//a new height, as other coords are fixed, it will
	//be used than by the item builders if needed.
	//it also updates the local controller pixmap
	setPositionFromLayout (m_layout_type);

	if(m_ctrls.size()==0) return; //exit if no items


	//checked ctrls whole width looking to rebuild items
	bool needRebuild = false;

	//begin whole width with a custom 30 offset + (8+this pixmap) + item startpos
	int wwidth = 30 + 8 + m_qpix_rect.width() + m_items_startpos;

	//subitems line width
	if(m_host_ctrls!=NULL)
	{
		//add the parent item width to the whole
		wwidth += m_ctrls[0]->getCurrentPos().width();

		for(int i=0; i<(*m_host_ctrls).size(); i++)
		{
			//skip items on the same column
			int vH = oHeight - (*m_host_ctrls).at(i)->getCurrentPos().height() -8;
			if((*m_host_ctrls).at(i)->getCurrentPos().top() == vH) {
				//add item width to whole width
				wwidth += (*m_host_ctrls).at(i)->getCurrentPos().width();
			}
		}
		if(wwidth>=m_widget_size.width()) needRebuild = true;
	}
	else //items line width
	{
		for(int i=0; i<m_ctrls.size(); i++)
			wwidth += m_ctrls.at(i)->getCurrentPos().width();

		if(wwidth>=m_widget_size.width()) needRebuild = true;
	}


	//get the new base position
	//TODO:switch that '78' based on layouts eventually
	int hpos = m_widget_size.height() -78;

	//here we don't need a rebuild, but only to update heights
	//for items to repaint in the new updated position
	if(!needRebuild)
	{
		//nothing to do it if height didn't change
		if(oHeight != m_widget_size.height() && m_layout_type!=TOPLEFT)
		{
			//update subitems hposition
			if(m_host_ctrls!=NULL)
			{
				for(int i=0; i<(*m_host_ctrls).size(); i++)
				{
					int vPos = hpos;

					//TODO:WORKS BUT ITEMS ON DIFF HEIGHTS GET SWAPPED !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
					//get the usual hline to check for items on different heights
					int vH = oHeight - (*m_host_ctrls).at(i)->getCurrentPos().height() -8;
					if((*m_host_ctrls).at(i)->getCurrentPos().top() != vH)
						vPos +=37; //add half widget (355) + 2 as mid space offset

					//std::cout << "ciemprez " << i << ", " << vH
					//<< ", " << (*m_host_ctrls).at(i)->getCurrentPos().top() << ", " << vPos << std::endl;

					//reset position
					(*m_host_ctrls).at(i)->resetHPosition (vPos);
				}
				//std::cout << "######################################" << std::endl;
			}

			//update items hposition
			for(int i=0; i<m_ctrls.size(); i++)
			m_ctrls.at(i)->resetHPosition (hpos);
		}
	}
	else //gonna rebuild and animate items
	{
		//this is stuff for rebuilding the current set of items
	  	//when ie the widget size is that that forces items to
	  	//be re-placed in a different layout ..................

		//sub items animation
		if(m_host_ctrls!=NULL)
		{
			m_ctrls[m_anim_last_item]->resetSubitems();//!< delete subitems

			m_anim_state=TOALONE;
			OverlayAnimSubItem * animctrl = static_cast<OverlayAnimSubItem *> (m_ctrls[m_anim_last_item]);
			animctrl->buildSubItems(); //engage anim directly from animsubitem class

			//update parent items too (but just the position)
			//there's the parent animsubitem visible, the
			//others are hidden, reposition them all
			for(int i=0; i<m_ctrls.size(); i++)
			m_ctrls.at(i)->resetHPosition (hpos);

			return;
		}

		//items animation
		if(m_ctrls.size()==0) return;
		reset();

		this->setIsPainting (true);				//!< lock widget refresh and stop render

		m_is_enabled = true;

		if(m_builder->buildItems (this, &m_ctrls, m_items_startpos)) //!< build items
		{
			//install event filter on items
			//if(m_hasno_subitems==false)
			for (int i=0; i<m_ctrls.size(); i++)
			connect(this, 		 SIGNAL (anim_ended(int,int)),
					m_ctrls [i], SLOT 	(anim_has_ended(int,int)));

			emit items_ready ();
		}

		m_timer->start (ANINSTEP);				//!< start items animation
	}
}

int OverlayItemsController::getParentHeight () const { return m_widget->getWidgetHeight(); }
int OverlayItemsController::getParentWidth () const { return m_widget->getWidgetWidth(); }
GLViewer * OverlayItemsController::getParentWidget () const { return m_widget; }
void OverlayItemsController::installEventFilter (OverlayItem* iItem) { m_widget->installEventFilter (iItem); }

void OverlayItemsController::setBckPixmaps (const QString & iBck) { m_qpixmap = new QPixmap (iBck); }

void OverlayItemsController::setLayout (int ilay) { m_layout_type = ilay; setPositionFromLayout (ilay); }
void OverlayItemsController::setPositionFromLayout (int ilay)
{
	switch (m_layout_type)
	{
	case BOTTOMLEFT :
		m_qpix_rect = QRect (8, m_widget->getWidgetHeight()-56, 48, 48);
		break;
	case BOTTOMRIGHT :
		m_qpix_rect = QRect (m_widget->getWidgetWidth() -8, m_widget->getWidgetHeight()-56, 48, 48);
		break;
	case TOPLEFT :
		m_qpix_rect = QRect (8, 8, 48, 48);
		break;
	case TOPRIGHT :
		m_qpix_rect = QRect (m_widget->getWidgetWidth() -8, 56, 48, 48);
		break;
	}
}

void OverlayItemsController::reset ()
{
	m_is_enabled = false;
	m_is_painting = false;
	m_anim_cur_item = -1;
	m_anim_last_item = -1;
	m_elapsed = 0;
	m_anim_todisplay = false;
	m_anim_mode = 0;
	m_anim_state = -1;

	m_numpad_slot_filled = false;

	for(int i=0; i<m_ctrls.size(); i++)
		delete m_ctrls[i];
	m_ctrls.clear();

	m_host_ctrls =NULL;
}

void OverlayItemsController::requestAnimation (int iID, bool isReDisplay)
{
	m_anim_todisplay = isReDisplay;
	if(m_anim_todisplay) m_anim_cur_item = -1;		//get back to initial display layout
	else 				m_anim_cur_item = iID;		//get to subitems display layout ...
													//ie. hide others, re-pos selected
	if(!this->isPainting() && m_elapsed==0)
	{
		this->setIsPainting (true);
		m_timer->start (ANINSTEP);					//!< start animation
	}
}

void OverlayItemsController::requestHostAnimation (int iID, QVector<OverlayItem*> * iQVectorItems)
{
	if(!this->isPainting() && m_elapsed==0 &&
		m_anim_state==TOALONE && m_anim_last_item==iID)//ensure we are ready for settings
	{												//and that we come from the same item
		m_host_ctrls = iQVectorItems;
		m_anim_cur_item = iID;

		m_anim_mode = HOST;							//init mode to animate settings sub-items
		this->setIsPainting (true);

		m_timer->start (ANINSTEP);					//!< start entry settings animation
	}
}

void OverlayItemsController::anim_ctrl()
{
	if(m_elapsed==ANIMTIME) 						//stop animation
	{
		m_elapsed = 0;
		this->setIsPainting (false);

		m_anim_last_item = m_anim_cur_item;
		m_anim_cur_item = -1;

		m_timer->stop();							//!< stop any animation started
		emit anim_ended (m_anim_state,m_anim_last_item);//!< emit end of animation signal


		//Animationn finished ...
		if(m_is_enabled && m_anim_state==TODISPLAY)	//finished in-animation
		{
			//items are displayed, install events
			for (int i=0; i<m_ctrls.size(); i++)
				m_widget->installEventFilter (m_ctrls [i]);
		}

		if(m_anim_mode==HOST && m_anim_state==TOHOST)
		{											//finished sub-items intro
			//sub-items are displayed,
			//back to defaul anim setup
			m_anim_mode = REGULAR;
		}

		if(!m_is_enabled && m_anim_state==TOBACK)	//finished out-animation
		{
			//we may use also the anim_has_ended
			//slot to cleanup stuff automatically
			for(int i=0; i<m_ctrls.size(); i++)
				if(m_ctrls[i])
				{
					m_widget->removeEventFilter (m_ctrls [i]);
					delete m_ctrls[i];				//!< delete items
				}
			m_ctrls.clear();
			m_host_ctrls = NULL;
		}

	}else
	{												//!< elapsed time
		m_elapsed = (m_elapsed + qobject_cast<QTimer*>(sender())->interval()) % ANIMTIME;


		if(m_elapsed==0) 							//ie. we reached animtime
		{
			m_elapsed = ANIMTIME;					//flag to end anim
		}
		else										//!< broadcast animation to items
		{
			float dd = float(m_elapsed)/float(ANIMTIME);

			if(m_anim_mode == REGULAR)
			{
				if(m_anim_cur_item==-1)
				{
					if(m_is_enabled || m_anim_todisplay)	//anim to display (end) pos
					{
						m_anim_state = TODISPLAY;
						for(int i=0; i<m_ctrls.size(); i++){
							if(m_ctrls[i]->isHidden()!=2)
							m_ctrls[i]->setIsHidden (false); //TODO:THISISCRAP !
							m_ctrls[i]->animateForward (dd);
						}
					}else
					{								//anim to original start pos
						m_anim_state = TOBACK;
						for(int i=0; i<m_ctrls.size(); i++)
							m_ctrls[i]->animateBackward (dd);
					}
				}else
				{									//anim to a target pos (hiding others)
					m_anim_state = TOALONE;
					for(int i=0; i<m_ctrls.size(); i++)
					{
						if(i==m_anim_cur_item) m_ctrls[i]->animateLonely (dd);
						else m_ctrls[i]->setIsHidden (true);	//TODO:THISISCRAP !
					}
				}

			}else if(m_anim_mode == HOST)			//anim subitems
			{
				if(m_host_ctrls!=NULL)
				{
					m_anim_state=TOHOST;
					QVector<OverlayItem*> h_host_ctrls = *m_host_ctrls;
					//QVector<OverlayItem*> h_host_ctrls = *m_ctrls[m_anim_cur_item];
					for(int i=0; i<h_host_ctrls.size(); i++)
						h_host_ctrls[i]->animateForward (dd);
				}
			}
		}
	}

	m_widget->update(); 							//!< refresh widget manually
}

void OverlayItemsController::paint (QPainter* iPainter)
{
	iPainter->setOpacity (0.6);
	iPainter->drawPixmap (m_qpix_rect, *m_qpixmap);

	//broadcast paint event to items
	for(int i=0; i<m_ctrls.size(); i++)
		if(m_ctrls[i])
			m_ctrls[i]->paint (iPainter);
}

bool OverlayItemsController::eventFilter (QObject *object, QEvent *event)
{
    if (object == m_widget)
    {
        switch (event->type())
        {

#ifndef GFXVIEW
        case QEvent::MouseButtonPress:
#else
        case QEvent::GraphicsSceneMousePress:
#endif
        {
			QMouseEvent *e = (QMouseEvent *) event;
			int x = e->pos().x();
			int y = e->pos().y();

			if (m_qpix_rect.intersects (QRect(x, y, 1, 1)))
			{
				m_is_enabled = !m_is_enabled;
				this->setIsPainting (true);				//!< lock widget refresh and stop render
				m_anim_todisplay = false;

				if(m_is_enabled)
				{
					if(m_builder)
					{
						if(m_builder->buildItems (this, &m_ctrls, m_items_startpos)) //!< build items
						{
							//install event filter on items
							//if(m_hasno_subitems==false)
							for (int i=0; i<m_ctrls.size(); i++)
							connect(this, 		 SIGNAL (anim_ended(int,int)),
									m_ctrls [i], SLOT 	(anim_has_ended(int,int)));

							emit items_ready ();
						}
					}
				}

				//for the kind of anim we implemnt ...
				//delete sub-items here, they'd be anyway
				//deleted when parent items are deleted
				if(!m_is_enabled && m_anim_state==TOHOST)
					m_ctrls[m_anim_last_item]->resetSubitems();//!< delete subitems

				//if returning and user chosen so, delete immediately
				if(m_deleteon_toback_anim && !m_is_enabled &&
						(m_anim_state==TODISPLAY || m_anim_state==TOHOST))
				{
					for(int i=0; i<m_ctrls.size(); i++)
						if(m_ctrls[i])
						{
							m_widget->removeEventFilter (m_ctrls [i]);
							delete m_ctrls[i];				//!< delete items
						}
					m_ctrls.clear();
					m_host_ctrls = NULL;
				}

				m_timer->start (ANINSTEP);

				return true; //!< stop event propagation
			}
        } break;
        }
    }
	return false;
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Renderer settings //////////////////////////////////////////////////////////////////////////////////////////

// subitems builder
bool OverlayRenderSettingsBuilder::buildItems ( OverlayItemsController * const& iFactory,
												QVector<OverlayItem*> * const& iSubitems,
												int iPos )
{
	int h = iFactory->getParentHeight()-78;

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
	}
}

// bind renderer settings
void OverlayRendererSettingsItem::valueChanged_slot (int data, int decdigits, int id)
{
	GLViewer * tViewer = getFactory()->getParentWidget();

	switch (id)
	{
	case 0:	//MaxDepth GLViewer
		if(tViewer->getRendererMaxDepth() != data )
		tViewer->setRendererMaxDepth (data);
		break;
	case 1:	//SPP
		if(tViewer->getRendererSPP() != data )
		tViewer->setRendererSPP (data);
		break;
	case 2:	//MinContribution
		float rdata = data / pow(10,decdigits);
		if(tViewer->getRendererMinContribution() != rdata )
		tViewer->setRendererMinContribution (rdata);
		break;
	}
}


// Camera settings //////////////////////////////////////////////////////////////////////////////////////////

// subitems builder
bool OverlayCameraSettingsBuilder::buildItems ( OverlayItemsController * const& iFactory,
												QVector<OverlayItem*> * const& iSubitems,
												int iPos )
{
	int h = iFactory->getParentHeight()-78;

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
	int h = iFactory->getParentHeight()-78;

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
bool OverlayRendererCtrlsBuilder::buildItems (OverlayItemsController * const& iFactory,
											QVector<OverlayItem*> * const& iItems,
											int iPos )
{
	int h = iFactory->getParentHeight()-78;


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




///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Framebuffer settings ///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Main render controller item builder ////////////////////////////////////////////////////////////////////////
bool OverlayFramebufferCtrlsBuilder::buildItems (	OverlayItemsController * const& iFactory,
													QVector<OverlayItem*> * const& iItems,
													int iPos )
{
	//int h = iFactory->getParentHeight()-78;
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


