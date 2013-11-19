/*
 * glviewer.cpp
 *
 *  Created on: Nov 28, 2012
 *      Author: max
 */

#include "glviewer.h"

GLViewer::GLViewer(int argc, char *argv[]) :
g_renderState(RSTOPPED)				//!< Start stopped

, g_width(512), g_height(512)

, m_fb_width(0), m_fb_height(0)
, m_fb_render_width(0), m_fb_render_height(0)
, m_fb_rscene_width(0), m_fb_rscene_height(0)

, m_fb_percent(100)
, m_fb_zoom(false)
, m_fb_ispanning(false)

, m_fb_woffset(0), m_fb_hoffset(0)
, m_fb_oddoffset_x(0), m_fb_oddoffset_y(0)

, m_fb_mode(0)

, m_init_resizing(0)

, m_device_is_painting(false)

, g_iCounter(0)
, g_minIterations(0)
, g_renderregion(false)
, g_iVerbose(1)
, m_rregion_delayed(false)

, m_rcamera(NULL)
, m_rregion(NULL)
, m_renderer_ctrl(NULL)
, m_framebuffer_ctrl(NULL)
{

	setFocusPolicy(Qt::StrongFocus); //set initial focus here
	setMouseTracking(true);

	//! Create render device
	rrDevice = new embree::RomboRenderDevice(argc, argv);

	// parse command line
	if (argc > 1) {
		rrDevice->parseCmdLine(argc, argv);
		rrDevice->buildScene("");
	}
}

GLViewer::~GLViewer() {
	//rrDevice is being deleted in flushScene()

	delete m_rcamera;
	delete m_rregion;
	delete m_renderer_ctrl;
	delete m_framebuffer_ctrl;

	delete m_fbo;
	for (int i = 0; i < 4; i++) {
		delete m_rr_pixelbuffer[i];
		m_rr_pixelbuffer[i] = NULL;
		m_rr_onBoardMemPtr[i] = NULL;
	}
}

// SLOT: called when File.open from MainWindow
void GLViewer::parseSceneAndRender(const std::string& iPath)
{
	if (!rrDevice) {
		int argc = 1;
		char* argv[] = { (char*) "", (char*) "" };
		rrDevice = new embree::RomboRenderDevice(argc, argv);

		rrDevice->newFramebuffer(g_width, g_height);
		rrDevice->updateCamera();
	}

	if (rrDevice->buildScene(iPath)) {
		if (rrDevice->sceneHasSize())
			setFbSceneSize(	rrDevice->getFramebufferWidth(),
							rrDevice->getFramebufferHeight());
		else
			setFbSceneSize(g_width, g_height);

		m_fb_mode = SCENESIZE;

		//!viewer devices
		m_rcamera = new RenderCamera(this);
		m_rregion = new RenderRegion(this);

		//renderer overlay settings ctrls
		m_renderer_ctrl = new OverlayRendererCtrls(this);
		m_renderer_ctrl->setLayout(OverlayItemsController::BOTTOMLEFT);
		m_renderer_ctrl->setBckPixmaps(
				"./images/gloverlay/main_settings_in.png");

		//framebuffer overlay settings ctrls
		m_framebuffer_ctrl = new OverlayFramebufferCtrls(this);
		m_framebuffer_ctrl->setItemsHasNoSubItems();
		m_framebuffer_ctrl->setLayout(OverlayItemsController::TOPLEFT);
		m_framebuffer_ctrl->setItemsStartingPos(28);
		m_framebuffer_ctrl->setBckPixmaps(
				"./images/gloverlay/main_settings_square.png");
		m_framebuffer_ctrl->setDeleteItemsAtToBackAnim();

		//init viewer renderer
		g_renderState = RSTARTED;
		g_minIterations = 5;

		if (g_iVerbose >= 1)	//!< EMIT scene verbosity
			emit verboseStream(rrDevice->getSceneStats().c_str());

		//update viewport and go to render frame
		m_init_resizing = 0;

#ifndef GFXVIEW
		this->resizeGL(g_width, g_height);
#else
		this->initializeGL();

		int tW, tH;
		tW = g_width;
		tH = g_height;
		g_width = g_width-1;
		g_height = g_height-1;

		this->resizeGL (tW, tH);
		update();
#endif
	}
}

// SLOT:
void GLViewer::flushScene() {
	rrDevice->clearScene();

	delete rrDevice;
	rrDevice = NULL;

	g_renderState = RSTOPPED;
}

// SLOT: called from menu:render:verbosity
void GLViewer::setRenderVerbosity(int iV) {
	g_iVerbose = iV;
}
// SLOT: called from menu:render:play/pause/stop
void GLViewer::setRenderStatus(int iR) {
	g_renderState = iR;
	update();
}

// SLOT: called from renderregion
void GLViewer::setRenderRegion(int iX0, int iY0, int iX1, int iY1) {

	//std::cout << "iX0: " << iX0 << ", iY0: " << iY0 << ", iX1: " << iX1
	//		<< ", iY1: " << iY1 << std::endl;

	//normalize rregion coords into fb coords
	iX0 -= m_fb_woffset;
	iY0 -= m_fb_hoffset;
	iX1 -= m_fb_woffset;
	iY1 -= m_fb_hoffset;

	//sanity check for w,h before to pass them to renderer
	if (iX0 < 0)
		iX0 = 0;
	if (iY0 < 0)
		iY0 = 0;
	if (iX1 > m_fb_render_width)
		iX1 = m_fb_render_width;
	if (iY1 > m_fb_render_width)
		iY1 = m_fb_render_height;
	/*
	std::cout << "oX0: " << iX0 << ", oY0: " << iY0 << ", oX1: " << iX1
			<< ", oY1: " << iY1 << ", m_fb_woffset: " << m_fb_woffset
			<< ", m_fb_hoffset: " << m_fb_hoffset << std::endl;
	*/

	//set renderer window
	rrDevice->setRenderRegionCoords(iX0, iY0, iX1, iY1);

	//shortcuts for gl render loop
	m_rr_size = QSize(iX1 - iX0, iY1 - iY0);
	m_rr_topLeft = QPoint(iX0, iY0);
	m_rr_bottomRight = QPoint(iX1, iY1);
}

QRect GLViewer::getRenderRegionCoords(bool widcoords) {
	if (widcoords) {
		QPoint offs((m_fb_woffset) /*+m_fb_oddoffset_x*/,
				(m_fb_hoffset) /*+m_fb_oddoffset_y*/);
		return QRect(m_rr_topLeft + offs, m_rr_bottomRight + offs);
	} else {
		return QRect(m_rr_topLeft, m_rr_bottomRight);
	}
}

/******************************************************************************/
/*                      Mouse and Keyboard                                    */
/******************************************************************************/
void GLViewer::keyPressEvent(QKeyEvent* e) {

	if (g_renderState == RSTOPPED)
		return;

	//!< Verbosity console (inc/dec) ////////////////////////////////////////
	if (e->key() == Qt::Key_V && e->modifiers() == Qt::ShiftModifier) {
		g_iVerbose++;
	} else if (e->key() == Qt::Key_V) {
		g_iVerbose--;
		if (g_iVerbose < 0)
			g_iVerbose = 0;
	}
	//|< RenderState ////////////////////////////////////////////////////////
	else if (e->key() == Qt::Key_R) {
		g_renderState = !g_renderState;
		if (g_renderState)
			emit rendererStatus(1);
		else
			emit rendererStatus(2); //!< emitters
	}
	//!< RenderRegion ///////////////////////////////////////////////////////
	else if (e->key() == Qt::Key_X) {
		g_renderregion = !g_renderregion;
		initRenderRegion();
	}
	//!< Renderer MaxDepth //////////////////////////////////////////////////
	else if (e->key() == Qt::Key_Less) {
		int mdepth = this->getRendererMaxDepth() - 1;
		if (mdepth < 1)
			mdepth = 1;

		this->setRendererMaxDepth(mdepth);
		this->resetAccumulation();
	} else if (e->key() == Qt::Key_Greater) {
		int mdepth = this->getRendererMaxDepth() + 1;
		if (mdepth > 128)
			mdepth = 32;

		this->setRendererMaxDepth(mdepth);
		this->resetAccumulation();
	}
}

/////////////////////////////////////////////////////////////////////////////
// MousePressEvent //////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#ifndef GFXVIEW
void GLViewer::mousePressEvent(QMouseEvent* e)
#else
		void GLViewer::mousePressEvent(QGraphicsSceneMouseEvent* e)
#endif
		{
	if (g_renderState == RSTOPPED) {
		e->accept();
		return;
	}

	//!< RenderRegion mouse mode ////////////////////////////////////////////
	if (g_renderregion)	// delegated to rregion class ///////////////////////
	{
		e->accept();
		return;
	}
}

/////////////////////////////////////////////////////////////////////////////
// MouseMoveEvent ///////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#ifndef GFXVIEW
void GLViewer::mouseMoveEvent(QMouseEvent* e)
#else
		void GLViewer::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
#endif
		{
	if (g_renderState == RSTOPPED) {
		e->accept();
		return;
	}

	//!< RenderRegion painting //////////////////////////////////////////////
	if (g_renderregion) // delegated to rregion class ////////////////////////
	{
		e->accept();
		return;
	}
}

/////////////////////////////////////////////////////////////////////////////
// MouseReleaseEvent ////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#ifndef GFXVIEW
void GLViewer::mouseReleaseEvent(QMouseEvent* e)
#else
		void GLViewer::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
#endif
		{
	//!< RenderRegion finishing /////////////////////////////////////////////
	if (g_renderregion) // delegated to rregion class ////////////////////////
	{
	}
}

/******************************************************************************/
/*                     		  	Utilities	                                  */
/******************************************************************************/
void GLViewer::setRenderRegionHasInvalidatedContext(bool inv) {
	m_rregion->setHasInvalidatedContext(inv);
}

void GLViewer::initRenderRegion() {
	//g_renderregion = !g_renderregion;
	rrDevice->setRenderRegion(g_renderregion);
	if (!m_rregion->init(g_renderregion)) {
		//getting back to camera mode ...
		//check if render context is still valid
		if (m_rregion->hasInvalidatedContext()) {
			//reset it if not
			this->resetAccumulation();
		}
	}
}

void GLViewer::updateRenderRegion() {
	m_rregion->hostPanned();

#ifdef DONT
	const QRectF rr = m_rregion->getBBoxRect();

	int iX0 = rr.topLeft().x();
	int iY0 = rr.topLeft().y();
	int iX1 = rr.bottomRight().x();
	int iY1 = rr.bottomRight().y();

#ifdef RIGHTHANDLEDCOORDSYS
	//last_rect is in screen coords, ie. x might be inverted, reverse that
	iX0 = g_width - iX0; iX1 = g_width - iX1;
#endif

	iX0 -= m_fb_woffset;
	iY0 -= m_fb_hoffset;
	iX1 -= m_fb_woffset;
	iY1 -= m_fb_hoffset;

	//sanity check for w,h before to pass them renderer
	if(iX0<0) iX0 = 0;
	if(iY0<0) iY0 = 0;
	if(iX1>m_fb_width) iX1 = m_fb_render_width;
	if(iY1>m_fb_height) iY1 = m_fb_render_height;
	/*
	 std::cout 	<< 	 "oX0: " << iX0
	 << ", oY0: " << iY0
	 << ", oX1: " << iX1
	 << ", oY1: " << iY1
	 << ", m_fb_woffset: " << m_fb_woffset
	 << ", m_fb_hoffset: " << m_fb_hoffset << std::endl;
	 */
	//shortcuts for gl render loop
	m_rr_size = QSize( iX1-iX0, iY1-iY0 );
	m_rr_topLeft = QPoint( iX0, iY0 );
	m_rr_bottomRight = QPoint( iX1, iY1 );
#endif
}

/******************************************************************************/
/*                     		  	QPainter	                                  */
/******************************************************************************/
void GLViewer::drawInstructions(QPainter *painter, int context) {
	QString text = tr("");
	QColor defCol(0, 0, 0, 127);
	if (context == 0) {
		text = tr("Click and drag with the left mouse button "
				"to rotate the camera, right to zoom, middle to pan.");
		defCol = QColor(0, 0, 0, 127);
	} else if (context == 1) { //rregion
		text =
				tr(
						"Click and drag with the left mouse button "
								"to draw the rregion box, use shift + left button for painting.");
		defCol = QColor(250, 20, 20, 127);
	}
	QFontMetrics metrics = QFontMetrics(font());
	int border = qMax(4, metrics.leading());

	QRect rect = metrics.boundingRect(0, 0, width() - 2 * border,
			int(height() * 0.125), Qt::AlignCenter | Qt::TextWordWrap, text);
	painter->setRenderHint(QPainter::TextAntialiasing);
	painter->fillRect(QRect(0, height(), width(), rect.height() + 2 * border),
			defCol);
	painter->setPen(Qt::white);
	painter->fillRect(QRect(0, 0, width(), rect.height() + 2 * border), defCol);
	painter->drawText((width() - rect.width()) / 2, border, rect.width(),
			rect.height(), Qt::AlignCenter | Qt::TextWordWrap, text);
}

/******************************************************************************/
/*                      OpenGL and Render                                     */
/******************************************************************************/
void GLViewer::saveGLState() {
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
}
void GLViewer::restoreGLState() {
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();
}

//!< Manage PBOs
void GLViewer::setRenderRegionSlot(int iSlot, int iState) {
	//release everything PBO related
	if (iSlot == -1 && iState == -1) {
		for (int i = 0; i < 4; i++) {
			if (m_rr_pixelbuffer[i]) {
				delete m_rr_pixelbuffer[i];
				m_rr_onBoardMemPtr[i] = NULL;
			}
		}
		m_rr_pbo_available = false;
		m_rr_pbo_active = -1;
		return;
	}

	//it's active, now it's deactivated
	if (iState == 3) {
		m_rr_pbo_active = -1;
		m_rr_pbo_available = false;
	}
	//it has been created, now it's active
	else if (iState == 2) {
		m_rr_pbo_active = iSlot;
		m_rr_pbo_available = true;
	}
	//create PBO
	else if (iState == 1) {
		QGLBuffer *pbuffer;

		pbuffer = new QGLBuffer(QGLBuffer::PixelUnpackBuffer);
		pbuffer->setUsagePattern(QGLBuffer::StreamDraw);
		pbuffer->create();

		if (pbuffer) {
			//binf buffer
			if (!pbuffer->bind()) {
				std::cout << "Could not bind PBO to the OpenGL context"
						<< std::endl;
				return;
			} else {
				std::cout << "Allocating vram for OpenGL PBO" << std::endl;
			}

			//allocate
			pbuffer->allocate(
					4 * sizeof(float)
							* (m_rr_size.width() * m_rr_size.height()));

			//map to device vram
			m_rr_onBoardMemPtr[iSlot] = (tVec4f*) pbuffer->map(QGLBuffer::ReadOnly);

			int X0 = m_rr_topLeft.x();
			int Y0 = m_rr_topLeft.y();
			int X1 = m_rr_bottomRight.x();
			int Y1 = m_rr_bottomRight.y();
			int w = m_rr_size.width();
			int h = m_rr_size.height();

			//fill pbuf with rregion pixel data
			int o = 0;
			for (size_t y = Y0; y <= Y1 - 1; y++) {
				for (size_t x = X0; x <= X1 - 1; x++) {
					//int i = y*g_width+x;
					m_rr_onBoardMemPtr[iSlot][o] =
							//! get RGB slot from framebuffer
							*((tVec4f*) rrDevice->getFramebufferAt(x, y));
					o++;
				}
			}

			//release device ptr
			pbuffer->unmap();
			//un-bind buffer
			pbuffer->release();

			glFlush();

			//!< link created pbuffer to QGLBuffer slot
			m_rr_pixelbuffer[iSlot] = pbuffer;
			m_rr_pbo_active = iSlot;
			m_rr_pbo_available = true;//we've one pbuffer available, enable PBO RR system
		}
	}
	//delete PBO
	else if (iState == 0) {
		//release stuff
		if (m_rr_pixelbuffer[iSlot]) {
			delete m_rr_pixelbuffer[iSlot];
			m_rr_pixelbuffer[iSlot] = NULL;
			m_rr_onBoardMemPtr[iSlot] = NULL;

			//if it was the one active disable slots alltogether
			if (m_rr_pbo_active == iSlot) {
				m_rr_pbo_active = -1;
				m_rr_pbo_available = false;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// InitializeGL /////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void GLViewer::initializeGL()	//not used with QGraphicsScene
{
	emit init();								//!emit init signal

	//std::cout<<"initializeGL()"<<std::endl;
	glClearColor(0.25, 0.25, 0.25, 1.0);
	glDisable(GL_DEPTH_TEST);

	//get a new main qglframebuffer
	m_fbo = new QGLFramebufferObject(g_width, g_height);

	//rregion PBO setup
	for (int i = 0; i < 4; i++)	//reset slot buffers
		m_rr_pixelbuffer[i] = NULL;

	m_rr_pbo_available = false;
	m_rr_pbo_active = -1;

	if (g_iVerbose >= 1) {
		emit verboseStream("OpenGL initialization");
	}
}
;

/////////////////////////////////////////////////////////////////////////////
// ResizeGL /////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void GLViewer::resizeGL(int width, int height) {
	g_width = width;
	g_height = height;

	bool needaccumreset = false;
	float zFactor = 1.0f;

	//setup full screen widget vs fb size with offsets
	if (m_fb_mode == SCENESIZE) {
		//multiply render fb by percent
		int tFbSceneW = m_fb_rscene_width;
		int tFbSceneH = m_fb_rscene_height;
		if (!m_fb_zoom) {
			//multiply render scene size by percent
			tFbSceneW *= (m_fb_percent / (float) 100);
			tFbSceneH *= (m_fb_percent / (float) 100);
		} else {
			//zoom factor
			zFactor *= ((float) m_fb_percent / (float) 100);
			zFactor *= ((float) m_fb_percent / (float) 100);
		}

		m_fb_render_width = tFbSceneW;
		m_fb_render_height = tFbSceneH;

		//!< set QGL Fb size, it never goes over widget size
		m_fb_width = m_fb_render_width;
		if (m_fb_width > g_width)
			m_fb_width = g_width;

		m_fb_height = m_fb_render_height;
		if (m_fb_height > g_height)
			m_fb_height = g_height;

		//factor zoom into fb offset calc
		int fb_render_width = m_fb_render_width * zFactor;
		int fb_render_height = m_fb_render_height * zFactor;
		int diffoff = 0;

		//!< get width offset
		if (fb_render_width > g_width) {
			//widget get centered over render fb
			diffoff = fb_render_width - g_width;
			m_fb_woffset = (diffoff / 2);
			m_fb_woffset = -(m_fb_woffset);
		} else {
			//fb get centered over widget
			diffoff = g_width - (m_fb_width * zFactor);
			m_fb_woffset = (diffoff / 2);
		}
		//setup a +1 for odd differences between widget and fb width/heigth
		//ie. we manually move one pizel side to match rregion vs full screen
		if (diffoff % 2 != 0)
			m_fb_oddoffset_x = 1;
		else
			m_fb_oddoffset_x = 0;

		//!< get height offset
		if (fb_render_height > g_height) {
			diffoff = fb_render_height - g_height;
			m_fb_hoffset = (diffoff / 2);
			m_fb_hoffset = -(m_fb_hoffset);
		} else {
			diffoff = g_height - (m_fb_height * zFactor);
			m_fb_hoffset = (diffoff / 2);
		}
		if (diffoff % 2 != 0)
			m_fb_oddoffset_y = 1;
		else
			m_fb_oddoffset_y = 0;
	} else if (m_fb_mode == STICHTOWIDGET) {
		needaccumreset = true;

		m_fb_render_width = m_fb_width = g_width;
		m_fb_render_height = m_fb_height = g_height;

		m_fb_woffset = 0;
		m_fb_hoffset = 0;
		m_fb_oddoffset_x = 0;
		m_fb_oddoffset_y = 0;
	}

	int viewer_fb_width = m_fb_width;
	int viewer_fb_height = m_fb_height;

	if (m_fb_mode != STICHTOWIDGET && m_fb_zoom) {
		viewer_fb_width *= zFactor;
		if (viewer_fb_width > g_width)
			viewer_fb_width = g_width;

		viewer_fb_height *= zFactor;
		if (viewer_fb_height > g_height)
			viewer_fb_height = g_height;
	}

	emit resized();								//!emit resized signal

	if (g_renderregion)
		m_fb_ispanning = true;		//force a main fb refresh
									//or on a resize we'l end up with
									//only the rregion box displayed

	//setup GL state
	glClearColor(0.25, 0.25, 0.25, 1.0);
	glDisable(GL_DEPTH_TEST);

	//update buffers
	if (!m_init_resizing || needaccumreset) {
		if (g_renderregion) {
			g_renderregion = false;					//!< reset renderregion
			rrDevice->setRenderRegion(false); 		//!< go full framebuffer

			m_rregion_delayed = true;				//!< delayed rregion
		}

		//new render framebuffer
		rrDevice->newFramebuffer(m_fb_render_width, m_fb_render_height);
		rrDevice->updateCamera();

		//delete fbo and create a new one
		//delete m_fbo;
		//m_fbo = new QGLFramebufferObject(viewer_fb_width, viewer_fb_height);

	}

	//delete fbo and create a new one ...
	//this is mainly needed when render fb is larger than widget
	//and user changes manually widget size, it does not
	//refresh the new QGLFb offset position if not rebuild here
	delete m_fbo;
	m_fbo = new QGLFramebufferObject(viewer_fb_width, viewer_fb_height);

	//glViewport(0, 0, (GLsizei)m_fb_width, (GLsizei)m_fb_height);

#ifdef RIGHTHANDLEDCOORDSYS
	glRasterPos2i(1, 1);
	glPixelZoom(-1.0f * zFactor, -1.0f * zFactor);//!< invert both x and y(usual for OGL) view
#else
			//TODO:UPDATE ZOOM FACTOR !
			glRasterPos2i(-1, 1);
			glPixelZoom(1.0f, -1.0f);
#endif

	// verbosity emission
	if (g_iVerbose >= 1) {
		QString str = "Background resize : ";
		str += QString::number(g_width) + " ";
		str += QString::number(g_height);
		emit verboseStream(str);
	}

	if (!m_init_resizing || needaccumreset)
		g_iCounter = 0;	//reset iteration counter

	m_init_resizing = 1;
	update();
}

/////////////////////////////////////////////////////////////////////////////
// PaintGL //////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void GLViewer::paintGL()
{
	if (g_renderState == RSTOPPED)
		return;

	//!< Render accumulation into render framebuffer ////////////////////////
	double rt;
	if (!m_device_is_painting
			&& (g_renderState || g_iCounter < g_minIterations)) {
		rt = rrDevice->renderFrame(g_resetAccumulation); // render frame ///
	}
	g_resetAccumulation = false;

	//!< OpenGL prepare rendering ///////////////////////////////////////////
	QPainter painter(this);
	painter.beginNativePainting();

	saveGLState();	//!< save GL state

	double dt0; //display timer
	dt0 = rrDevice->getSecs();

	//data pointer to rendered fbuffer //////////////////////////////////////
	float* ptr = (float*) rrDevice->mapFramebufferPtr();

	//framebuffer gl rendering
	if (!g_renderregion | m_fb_ispanning) {
		int woffset = 0;
		if (m_fb_woffset <= 0)
			woffset = abs(m_fb_woffset);
		int hoffset = 0;
		if (m_fb_hoffset <= 0)
			hoffset = abs(m_fb_hoffset);
		/*
		 std::cout 	<< "GLoffsets " << m_fb_woffset << ", " << m_fb_hoffset << std::endl;
		 std::cout 	<< "Localoffsets " << woffset << ", " << hoffset << std::endl;
		 std::cout 	<< "FbSize " << m_fb_width << ", " << m_fb_height << std::endl;
		 std::cout 	<< "RenderSize " << m_fb_render_width << ", " << m_fb_render_height << std::endl;
		 std::cout	<< "==============================================================" << std::endl;
		 */
		m_fbo->bind();

		glPixelStorei(GL_UNPACK_ROW_LENGTH, m_fb_render_width);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, woffset);
		glPixelStorei(GL_UNPACK_SKIP_ROWS, hoffset);

		glRasterPos2i(
#ifdef RIGHTHANDLEDCOORDSYS
				g_width
						- (woffset != 0 ?
								0 : ((m_fb_woffset * 2) + m_fb_oddoffset_x)),
#else
				0,
#endif
				hoffset != 0 ? 0 : ((m_fb_hoffset * 2) + m_fb_oddoffset_y));

		/*if(g_renderregion)
		 {
		 glPixelTransferf(GL_RED_SCALE, 0.5f);
		 glPixelTransferf(GL_GREEN_SCALE, 0.5f);
		 glPixelTransferf(GL_BLUE_SCALE, 0.5f);
		 }*/

		//!> draw render fb to GL framebuffer
		glDrawPixels((GLsizei) m_fb_width, (GLsizei) m_fb_height, GL_RGB, GL_FLOAT, ptr);

		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

		m_fbo->release();

		//draw full GL fb to the screen
		m_fbo->drawTexture(
				QPoint(m_fb_woffset <= 0 ? 0 : m_fb_woffset,
						m_fb_hoffset <= 0 ? 0 : m_fb_hoffset), m_fbo->texture(),
				GL_TEXTURE_2D);
	}

	//RenderRegion
	if (g_renderregion) {
		//we leave untouched the pixels loaded in the last fb full draw,
		//we blit just the rregion box of new pixels into main fb
		//we draw on top of that a QGLBuffer if there're saved rregions

		//get divider for rregion actual pixels
		int divpixels = 0;
		bool isFullyCovered = false;

		if (m_rr_pbo_available && !m_rregion->isPainting())
			if (m_rregion->getDividerPixels() == 0)
				isFullyCovered = true;
			else
				divpixels = m_rr_size.width() - m_rregion->getDividerPixels();

		if (!isFullyCovered) //we may have saved-fb taking all the bbox space
		{

			int X0 = m_rr_topLeft.x();
			int Y0 = m_rr_topLeft.y();
			//std::cout << "X0-Y0 " << X0 << ", " << Y0 << std::endl;

			int glX0 = X0 + (m_fb_woffset * 2) + m_fb_oddoffset_x; //x0+off->get back widget rr coords + off-> add offset.. x0+off*2
			int glY0 = Y0 + (m_fb_hoffset * 2) + m_fb_oddoffset_y;
			//std::cout << "glX0-glY0 " << glX0 << ", " << glY0 << std::endl;

			int X1 = m_rr_bottomRight.x();
			int Y1 = m_rr_bottomRight.y();
			//std::cout << "X1-Y1 " << X1 << ", " << Y1 << std::endl;

			int glW = m_rr_size.width();
			int glH = m_rr_size.height();
			//std::cout << "glW-glH " << glW << ", " << glH << std::endl;
			//std::cout << "--------------------------------" << std::endl;

			if (m_fb_woffset < 0) {
				glX0 = X0 + m_fb_woffset; //widget coords
				//clamp rregion only to visible widget portion
				if (glX0 < 0) {
					glX0 = 0;
					X0 = glX0 - m_fb_woffset;
				}
				if (X1 > g_width)
					X1 = g_width;
				glW = X1 - glX0; //size based on clamp
			}

			if (m_fb_hoffset < 0) {
				glY0 = Y0 + m_fb_hoffset;
				if (glY0 < 0) {
					glY0 = 0;
					Y0 = glY0 - m_fb_hoffset;
				}
				if (Y1 > g_height)
					Y1 = g_height;
				glH = Y1 - glY0;
			}

			m_fbo->bind();

			//setup to upload rregion box pixels only from render fb,
			//while leaving the others unchanged,
			//ie. ptr [y*g_width+x], kinda 'blitting' the gl fb
			glPixelStorei(GL_UNPACK_ROW_LENGTH, m_fb_render_width);
			glPixelStorei(GL_UNPACK_SKIP_PIXELS, X0);		//X0
			glPixelStorei(GL_UNPACK_SKIP_ROWS, Y0);			//Y0

			//set top-left corner where begin to draw them into GL fb
			glRasterPos2i(
#ifdef RIGHTHANDLEDCOORDSYS
					g_width - glX0,
#else
					glX0,
#endif
					glY0);

			//draw (stamp, blit) pixels into main fb,
			//with width(minus divpixels) and height from rregion bbox
			glDrawPixels((GLsizei) glW - divpixels, (GLsizei) glH, GL_RGB, GL_FLOAT, ptr);

			//restore stuff and unbind fb
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
			glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
			glRasterPos2i(1, 1);

			m_fbo->release();
		}

		//draw main fb to the screen
		m_fbo->drawTexture(
				QPoint(m_fb_woffset <= 0 ? 0 : m_fb_woffset,
						m_fb_hoffset <= 0 ? 0 : m_fb_hoffset), m_fbo->texture(),
				GL_TEXTURE_2D);

		//draw saved fb portion on top
		if (m_rr_pbo_available && !m_rregion->isPainting()) {
			divpixels = m_rregion->getDividerPixels();
			if (m_rr_size.width() - divpixels > 2) //if rregion does not take all bbox width
					{ //it seems we have accumulated some offset .. so we use 2

				m_rr_pixelbuffer[m_rr_pbo_active]->bind();

				glRasterPos2i(
#ifdef RIGHTHANDLEDCOORDSYS
						m_fb_woffset >= 0 ?
								g_width
										- (m_rr_topLeft.x() + m_fb_woffset
												+ m_fb_oddoffset_x)
										- divpixels :
								g_width - (m_rr_topLeft.x() + m_fb_woffset)
										- divpixels,
#else
						m_rr_topLeft.x(),
#endif
						m_fb_hoffset >= 0 ?
								m_rr_topLeft.y() + m_fb_hoffset
										+ m_fb_oddoffset_y : //TODO:: +m_fb_oddoffset_y, doesn't work correctly here ?!!
								m_rr_topLeft.y() + m_fb_hoffset);

				glPixelStorei(GL_UNPACK_ROW_LENGTH, m_rr_size.width());
				glPixelStorei(GL_UNPACK_SKIP_PIXELS, divpixels);	//divider
				glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);				//0

				glDrawPixels(	(GLsizei) m_rr_size.width() - divpixels,
								(GLsizei) m_rr_size.height(), GL_RGB, GL_FLOAT, 0);

				glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
				glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
				glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
				glRasterPos2i(1, 1);

				m_rr_pixelbuffer[m_rr_pbo_active]->release();
			}
		}
	}

	m_fb_ispanning = false;

	//release ptr to fb buffer
	rrDevice->unmapFramebufferPtr();

	//update, actually engaging both cpu render and gl render loops
	if (!m_device_is_painting) //otherwise ctrled by the devices
		update();

	restoreGLState();	//!< restore GL state for QPainter //////////////////

	//!< Painter overlay ////////////////////////////////////////////////////
	//painter.setRenderHint(QPainter::Antialiasing);
	//drawInstructions(&painter, g_renderregion);//temp

	//if(this->underMouse()) //TODO::expand on this !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	emit painting(&painter);

	painter.endNativePainting();
	painter.end();

	//!< Verbosity //////////////////////////////////////////////////////////
	if (g_iVerbose >= 2 && (g_renderState || g_iCounter < g_minIterations)) {

		// finish measure rendering time
		double dt = rrDevice->getSecs() - dt0;

		// output rendertime to console
		QString rtr = "Iteration :";
		rtr += QString::number(g_iCounter);
		rtr += "\n";
		rtr += "Render : ";
		rtr += QString::number(1.0f / rt) + " fps, ";
		rtr += QString::number(rt * 1000.0f) + " ms\n";

		// output display time to console
		rtr += "Display : ";
		rtr += QString::number(1.0f / dt) + " fps, ";
		rtr += QString::number(dt * 1000.0f) + " ms";

		emit verboseStream(rtr);//!< emit verbose ///////////////////////////
	}

	//when fb.size==widget.size and rregion enabled,
	//we update for mininter the full framebuffer and then
	//here re-set stuff to go back to renderregion mode
	if (m_rregion_delayed && g_iCounter > g_minIterations) {
		g_renderregion = true;					//!< set renderregion
		rrDevice->setRenderRegion(true); 		//!< update render device
		rrDevice->setRenderRegionCoords(m_rr_topLeft.x(), m_rr_topLeft.y(),
										m_rr_bottomRight.x(), m_rr_bottomRight.y());
		m_rregion_delayed = false;
	}

	//when the render is paused GL is still running for g_minIterations /////
	if (g_renderState || g_iCounter <= g_minIterations)
		g_iCounter++;
}
;

