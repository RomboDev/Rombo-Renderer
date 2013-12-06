/*
 * glviewer.h
 *
 *  Created on: Nov 6, 2013
 *      Author: max
 */

#ifndef GLVIEWER_H_
#define GLVIEWER_H_


#include <QGLWidget>
#include <QGLBuffer>
#include <QGLFramebufferObject>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QUndoCommand>

#define GFXVIEW_XXX
#ifdef GFXVIEW
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
class RenderGLView;
#endif

#include "renderdevice.h"

#include "glviewer/IGLViewerDevice.h"
#include "glviewer/glrendercamera.h"
#include "glviewer/glrenderregion.h"
#include "glviewer/gloverlaycontrols.h"

class RenderCamera;
class RenderRegion;
class OverlayItemsController;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Undo/Redo -- TO BE MOVED !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
class UndoRedo : public QObject
{
	Q_OBJECT
public:
	UndoRedo (QObject* iWidget) : m_glUndoStack (new QUndoStack(iWidget)) {}
	~UndoRedo (){ delete m_glUndoStack; }

	QUndoStack* getStack() const { return m_glUndoStack; }

	void emitUndoRedo (int id, int slot, QVariant idata) { emit undo_redo (id, slot, idata); }
	void connectDevice (OverlayItemsController* iSettingCtrl);

signals:
	void undo_redo (int id, int slot, QVariant idata);

private:
	// Undo/Redo stack
	QUndoStack *m_glUndoStack;
};

class UndoRedoBase
{
public:
	UndoRedoBase (QObject* iWidget ) : m_undoredo (new UndoRedo (iWidget)) {}
	~UndoRedoBase (){ delete m_undoredo; }

	void emitUndoRedo (int id, int slot, QVariant idata) { m_undoredo->emitUndoRedo (id, slot, idata); }

	void undoCmd () { m_undoredo->getStack()->undo(); }
	void redoCmd () { m_undoredo->getStack()->redo(); }
	void pushCmd (QUndoCommand * iCommand) { m_undoredo->getStack()->push(iCommand); }

	void addUndoRedoToDevice (OverlayItemsController* iSettingCtrl)
	{
		m_undoredo->connectDevice (iSettingCtrl);
	}
private:
	UndoRedo * m_undoredo;
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class GLViewer :
#ifndef GFXVIEW
  public QGLWidget,
#else
  public QGraphicsScene,
#endif
  public UndoRedoBase
{
	Q_OBJECT

public:
	GLViewer ( int argc, char *argv[] );
	~GLViewer ();

#ifdef GFXVIEW
	friend RenderGLView;
	bool isSceneReady()
    {
    	if( g_renderState==RSTOPPED && g_minIterations==0)
    		return false;
    	else return true;
    }
	void forceRepaint ()
	{
		QGraphicsView * parentView = this->views()[0];
		if(parentView)
		{
			std::cout << "GLViewer->force repainting: " << this->sceneRect().height() << ", " << this->sceneRect().width() << std::endl;
			/*QWidget * viewport = parentView->viewport();
			viewport->update();*/
			//parentView->scene()->update(this->sceneRect());
			parentView->invalidateScene();
			parentView->scene()->update();
			//this->invalidate();
		}else
		{
			std::cout << "GLViewer->failed getting parent scene" << std::endl;
		}
	}
#endif

	enum OVERLAYCONTROLLERS
	{
		RENDERERSETTINGS = 0,
		FRAMEBUFFERSETTINGS
	};

	//!< Framebuffer //////////////////////////////////////////////////////////////////////////////////////////
	int getWidgetWidth() const { return g_width; }
	int getWidgetHeight() const { return g_height; }
	size_t getFbWidth() const { return m_fb_width; }
	size_t getFbHeight() const { return m_fb_height; }

	int getOffsetWidth() const { return m_fb_woffset; }
	int getOffsetHeight() const { return m_fb_hoffset; }
	void setOffsetWidth (int ioff, bool ispan=true) { m_fb_woffset = ioff; m_fb_ispanning = ispan; }
	void setOffsetHeight (int ioff, bool ispan=true) { m_fb_hoffset = ioff; m_fb_ispanning = ispan; }

	void setFbSceneWidth (size_t iwidth)	{ m_fb_rscene_width = iwidth; m_init_resizing = 0; resizeGL (g_width,g_height); }
	void setFbSceneHeight (size_t iheight)	{ m_fb_rscene_height = iheight; m_init_resizing = 0; resizeGL (g_width,g_height); }
	void setFbSceneSize (size_t iwidth, size_t iheight)
	{
		m_fb_rscene_width = iwidth;
		m_fb_rscene_height = iheight;
	}

	size_t getFbSceneWidth () const { return m_fb_render_width; }
	size_t getFbSceneHeight () const { return m_fb_render_height; }

	bool fbNeedsNavigator ()
	{
		if(m_fb_mode==STICHTOWIDGET) return false;

	    int tFbSceneW = m_fb_rscene_width;
	    int tFbSceneH = m_fb_rscene_height;
	    tFbSceneW *= (m_fb_percent /(float)100);
	    tFbSceneH *= (m_fb_percent /(float)100);

	    if(tFbSceneW > g_width) return true;
	    if(tFbSceneH > g_height) return true;

		return false;
	}

	void setFbScenePercent (int iper, bool only=false)	{ m_fb_percent = iper; if(!only) {m_init_resizing = m_fb_zoom; resizeGL (g_width,g_height);} }
	int getFbScenePercent () const { return m_fb_percent; }

	void setFbZoomMode (bool izoom=true) { m_fb_zoom = izoom; m_init_resizing = 0; resizeGL (g_width,g_height); }
	bool getFbZoomMode () const { return m_fb_zoom; }

	enum FRAMEBUFFERMODE
	{
		STICHTOWIDGET = 0,
		MATCHWIDGET  = 1,
		SCENESIZE = 2,
	};

	inline bool isFullFbWidget() const { return m_fb_woffset==0 && m_fb_hoffset==0; }

	void setFramebufferMode (FRAMEBUFFERMODE istich=STICHTOWIDGET) { m_fb_mode = istich; m_init_resizing = 0; resizeGL (g_width,g_height); }
	int getFramebufferMode () const { return m_fb_mode; }


	//!< Renderer /////////////////////////////////////////////////////////////////////////////////////////////
	void resetAccumulation ()
	{
		g_iCounter = 0;
		g_resetAccumulation = true;
	}
	void pauseRenderer ()
	{
		if(g_renderState<=0) return;

		g_iCounter = g_minIterations;
		g_resetAccumulation = false;
		g_renderState = 0;
	}
	void restartRenderer ()
	{
		if(g_renderState<0) return;

		g_iCounter = 0;
		g_resetAccumulation = false;
		g_renderState = 1;
	}

	bool isRenderRegion() const { return g_renderregion; }
	void setIsRenderRegion (bool irr) { g_renderregion = irr; }
	void setRenderRegionHasInvalidatedContext (bool inv);
	void initRenderRegion ();
	void updateRenderRegion ();

	int getRenderState() const { return g_renderState; }
	bool renderIsStopped() const { return g_renderState==RSTOPPED; }
	bool renderIsPaused() const { return g_renderState==RPAUSED; }
	bool renderIsStarted() const { return g_renderState==RSTARTED; }

	int getRendererSPP () const { return rrDevice->getRendererSPP(); }
	int getRendererMaxDepth () const { return rrDevice->getRendererMaxDepth(); }
	float getRendererMinContribution () const { return rrDevice->getRendererMinContribution(); }

	void setRendererSPP (int iSPP) { rrDevice->setRendererSPP (iSPP); resetAccumulation(); }
	void setRendererMaxDepth (int iMaxDepth) { rrDevice->setRendererMaxDepth (iMaxDepth); resetAccumulation(); }
	void setRendererMinContribution (float iMinContribution) { rrDevice->setRendererMinContribution (iMinContribution); resetAccumulation(); }

	OverlayItemsController* getRendererSettingGlDevice () const { return m_renderer_ctrl; }

	//!< Camera ///////////////////////////////////////////////////////////////////////////////////////////////
	void updateRenderCamera() { rrDevice->updateCamera(); }
	void centerRenderCamera (int iX, int iY){ rrDevice->centerCamera( iX, iY); }
	void focusRenderCamera (int iX, int iY){ rrDevice->focusCamera( iX, iY); }
	void updateRenderCameraMove (int iMouseMode, int iX, int iY, int iMcX, int iMcY, int wid_width){ rrDevice->updateCameraMove( iMouseMode, iX, iY, iMcX, iMcY, wid_width ); }
	void setRenderCameraFOV (float ifov) { rrDevice->setCameraFOV (ifov); }
	float getRenderCameraFOV () { return rrDevice->getCameraFOV(); }
	void incRenderCameraRadius() { rrDevice->incCameraRadius(); }
	void setRenderCameraRadius(const float ir) { rrDevice->setCameraRadius(ir); }
	float getRenderCameraRadius() const { return rrDevice->getCameraRadius(); }
	void printRenderCameraTransformStr() { emit verboseStream (rrDevice->getCameraTransformStr().c_str()); }


	//!< Tonamapper ///////////////////////////////////////////////////////////////////////////////////////////
	float getTonemapperGamma () const { return rrDevice->getTonemapperGamma(); }
	void setTonemapperGamma (float igamma) { rrDevice->setTonemapperGamma(igamma); rrDevice->resetTonemapper(); }
	bool getTonemapperVignetting () const { return rrDevice->getTonemapperVignetting(); }
	void setTonemapperVignetting (bool ivignette) { rrDevice->setTonemapperVignetting(ivignette); rrDevice->resetTonemapper(); }

	float getTonemapperSensitivity () const { return rrDevice->getTonemapperSensitivity(); }
	void setTonemapperSensitivity (float isensitivity) { rrDevice->setTonemapperSensitivity(isensitivity); rrDevice->resetTonemapper(); }
	float getTonemapperExposure () const { return rrDevice->getTonemapperExposure(); }
	void setTonemapperExposure (float iexp) { rrDevice->setTonemapperExposure(iexp); rrDevice->resetTonemapper(); }
	float getTonemapperFStop () const { return rrDevice->getTonemapperFStop(); }
	void setTonemapperFStop (float ifstop) { rrDevice->setTonemapperFStop(ifstop); rrDevice->resetTonemapper(); }


	//!< RenderRegion /////////////////////////////////////////////////////////////////////////////////////////
	QRect getRenderRegionCoords (bool widcoords);


signals:
	void verboseStream(QString);
	void rendererStatus(int);

	void init ();
	void resized ();
	void painting (QPainter*);

public slots:
	void parseSceneAndRender(const std::string&);
	void flushScene();
	void setRenderVerbosity(int iV);
	void setRenderStatus(int iR);
	void setRenderRegion(int iX0, int iY0, int iX1, int iY1);
	void setRenderRegionSlot (int iSlot, int iState);

	void deviceIsPainting (int ispainting) { m_device_is_painting = ispainting; }

protected:
	// OpenGL
    void initializeGL ();
    void resizeGL (int width, int height);
#ifndef GFXVIEW
    void paintGL ();
#else
    void resize (int width, int height)
    {
        if (g_width == size_t(width) && g_height == size_t(height)) return;

        g_width = width; g_height = height;
        rrDevice->newFramebuffer (g_width, g_height);
        rrDevice->updateCamera();
    }
    void drawBackground(QPainter *painter, const QRectF &rect);
#endif

    // Mouse Events
#ifndef GFXVIEW
    void mouseMoveEvent (QMouseEvent *e);
    void mousePressEvent (QMouseEvent *e);
    void mouseReleaseEvent (QMouseEvent *e);
#else
    void mouseMoveEvent(QGraphicsSceneMouseEvent  *e);
    void mousePressEvent(QGraphicsSceneMouseEvent  *e);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent  *e);
#endif

    // Keyboard Events
    void keyPressEvent (QKeyEvent *e);

private:
	void saveGLState();
	void restoreGLState();
	void drawInstructions(QPainter *painter, int context=0);

private:
	embree::RomboRenderDevice *rrDevice;		//!< Rombo Render Device

	//framebuffer stuff
	struct tVec4f { float x, y, z/*, w*/; };	//!< Util for PBOs
	QGLFramebufferObject *m_fbo;

    QGLBuffer * m_rr_pixelbuffer [4];
    tVec4f * 	m_rr_onBoardMemPtr [4];
    bool 		m_rr_pbo_available;
    int 		m_rr_pbo_active;

	QPoint m_rr_topLeft;
	QPoint m_rr_bottomRight;
	QSize  m_rr_size;

	// render state
	int g_renderState;
	enum RENDERSTATE
	{
		RSTOPPED = -1,
		RPAUSED  = 0,
		RSTARTED = 1
	};

	size_t 	g_width;					//!< widget size
	size_t 	g_height;

	size_t 	m_fb_width;					//!< GL framebuffer size
	size_t 	m_fb_height;

	size_t 	m_fb_render_width;			//!< render device framebuffer actual size
	size_t 	m_fb_render_height;

	size_t 	m_fb_rscene_width;			//!< render device framebuffer scene size
	size_t 	m_fb_rscene_height;

	int 	m_fb_percent;
	bool 	m_fb_zoom;
	bool 	m_fb_ispanning;

	int 	m_fb_woffset;				//!< offset between widget and render fb
	int 	m_fb_hoffset;
	int 	m_fb_oddoffset_x;			//mod for odd differences
	int 	m_fb_oddoffset_y;

	int 	m_fb_mode;

	int		m_init_resizing;

	//accumulation
	unsigned int g_iCounter;			// iteration counter
	unsigned int g_minIterations;		// for 'paused' mode, to get a bit of refinement
	bool g_resetAccumulation;			// clear accumulation

	// render verbosity
	int g_iVerbose;

	int m_device_is_painting;

	// render camera
	RenderCamera * m_rcamera;

	// render region
	RenderRegion * m_rregion;
	bool g_renderregion;
	bool m_rregion_delayed;

	// renderer setting controls
	OverlayItemsController * m_renderer_ctrl;

	// framebuffer controls
	OverlayItemsController * m_framebuffer_ctrl;
};


#endif /* GLVIEWER_H_ */
