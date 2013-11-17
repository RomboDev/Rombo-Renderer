/*
 * romborender.h
 *
 *  Created on: Nov 22, 2012
 *      Author: max
 */

#ifndef ROMBORENDER_H_
#define ROMBORENDER_H_


#define RIGHTHANDLEDCOORDSYS		//!< Righ handled coord system
#define DEBUGSCENEMEM_xxx
#define NOLOCALHANDLES_xxx


#include "sys/platform.h"
#include "sys/constants.h"
#include "sys/filename.h"
#include "sys/ref.h"
#include "sys/stl/string.h"

#include "image/image.h"

#include "lexers/streamfilters.h"
#include "lexers/parsestream.h"

#include "device/loaders/loaders.h"

#include "math/math.h"
#include "math/vec2.h"
#include "math/vec3.h"
#include "math/vec4.h"
#include "math/col3.h"
#include "math/affinespace.h"

#include "device/device.h"
#include "device/handle.h"


// include GLUT for display
// TODO: check if needed also here for Qt
#if defined(__MACOSX__)
#  include <OpenGL/gl.h>
#  include <GLUT/glut.h>
#  include <ApplicationServices/ApplicationServices.h>
#elif defined(__WIN32__)
#  include <windows.h>
#  include <GL/gl.h>
#  include <GL/glut.h>
#else
#  include <GL/gl.h>
#  include <GL/glut.h>
#endif

#include "QtOpenGL/QGLWidget"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QGLBuffer>


namespace embree
{

class RomboRenderDevice
{

public:
	RomboRenderDevice ( int argc, char *argv[] );
	~RomboRenderDevice (){}

//interface:
	void createNewDevice() { g_device = Device::rtCreateDevice("default", 0); }


	//!< Framebuffer //////////////////////////////////////////////////////////////////////////////////////////
	__forceinline void setRenderRegion(bool irr) { g_renderregion = irr; }
	__forceinline void setRenderRegionCoords( 	size_t rr_start_x, size_t rr_start_y,
												size_t rr_stop_x, size_t rr_stop_y )
	{
		g_rr_start.x = rr_start_x; g_rr_start.y = rr_start_y;
		g_rr_stop.x = rr_stop_x; g_rr_stop.y = rr_stop_y;
	}
	__forceinline size_t getRenderRegionCoord( const int& icoord )
	{
		switch( icoord)
		{
		case 0: return g_rr_start.x;
		case 1: return g_rr_start.y;
		case 2: return g_rr_stop.x;
		case 3: return g_rr_stop.y;
		default: return 0;
		}
	}


	__forceinline void* mapFramebufferPtr (void) const { return g_device->rtMapFrameBuffer(g_frameBuffer); }
	__forceinline void unmapFramebufferPtr (void) { g_device->rtUnmapFrameBuffer(g_frameBuffer); }
	__forceinline void* getFramebufferAt (size_t& x_i, size_t& y_i) const { return g_device->rtGetFrameBufferAt(g_frameBuffer, x_i, y_i); }

	__forceinline size_t getFramebufferWidth () const { return g_width; }
	__forceinline size_t getFramebufferHeight () const { return g_height; }
	bool sceneHasSize () const { return g_scene_has_size; }

	//build a new framebuffer
	__forceinline
	void newFramebuffer( const size_t w, const size_t h)
	{

		#ifdef NOLOCALHANDLES
		if(g_frameBuffer!=NULL)
		g_device->rtDestroyFrameBufferData( g_frameBuffer );

		delete g_frameBuffer;
		g_frameBuffer = NULL;
		#endif
		g_width=w; g_height=h;

		if(!g_renderregion)
		{
			//init rregion start/stop, used later in rtRenderFrame
			g_rr_start.x = 0; g_rr_start.y = 0;
			g_rr_stop.x = g_width; g_rr_stop.y = g_height;
		}

		//!< WE DON?T REALLY EVER USE A CROPPED FRAMEBUFFER
		g_frameBuffer = g_device->rtNewFrameBuffer("RGB_FLOAT32",g_width, g_height, g_numBuffers );

	}

	//!< render framebuffer
	__forceinline
	double renderFrame( const int accumulation )
    {
		int accumulate = accumulation ? 0 : g_refine;

		double rt0, rt; //render and display timers

		rt0 = getSeconds();
		// render into framebuffer
		g_device->rtRenderFrame(g_renderer,
								g_camera,
								g_render_scene,
								g_tonemapper,
								g_frameBuffer,
								g_rr_start.x, g_rr_start.y,
								g_rr_stop.x, g_rr_stop.y,
								accumulate);
		rt = getSeconds()-rt0;

		// swap internal buffers ... TODO:only for g_buffer>1 ??!
		//g_device->rtSwapBuffers(g_frameBuffer);

		return rt;	//return render time
    }


	//!< Camera ///////////////////////////////////////////////////////////////////////////////////////////////
	__forceinline float getCameraFOV () const { return g_camFieldOfView; }
	inline void setCameraFOV (float ifov) { g_camFieldOfView = ifov; }

	__forceinline float getCameraRadius () const { return g_camRadius; }
	inline void setCameraRadius (const float ir) { g_camRadius = ir; }
	inline void incCameraRadius () { g_camRadius++; }

	//new camera from up-to-date settings
	void updateCamera();

	//update cam based on mouse move
	void updateCameraMove(int mouseMode, int x, int y, int prevx, int prevy, int wid_width);

	//center camera on mouse pointer
	inline void focusCamera(const int x, const int y)
	{
        Vector3f p;
		#ifdef RIGHTHANDLEDCOORDSYS
        bool hit = g_device->rtPick(g_camera, (g_width-x) / float(g_width), y / float(g_height), g_render_scene, p.x, p.y, p.z);
		#else
        bool hit = g_device->rtPick(g_camera, x / float(g_width), y / float(g_height), g_render_scene, p.x, p.y, p.z);
		#endif
        if (hit) {
          Vector3f v = normalize(g_camLookAt - g_camPos);
          Vector3f d = p - g_camPos;
          g_camLookAt = g_camPos + v*dot(d,v);
          g_camSpace = AffineSpace3f::lookAtPoint(g_camPos, g_camLookAt, g_camUp);
        }

        updateCamera();
	}

	//focus camera on mouse pointer
	inline void centerCamera(const int x, const int y)
	{
        Vector3f p;
		#ifdef RIGHTHANDLEDCOORDSYS
        bool hit = g_device->rtPick(g_camera, (g_width-x) / float(g_width), y / float(g_height), g_render_scene, p.x, p.y, p.z);
		#else
        bool hit = g_device->rtPick(g_camera, x / float(g_width), y / float(g_height), g_render_scene, p.x, p.y, p.z);
		#endif
        if (hit) {
          Vector3f delta = p - g_camLookAt;
          Vector3f right = cross(normalize(g_camUp),normalize(g_camLookAt-g_camPos));
          Vector3f offset = dot(delta,right)*right + dot(delta,g_camUp)*g_camUp;
          g_camLookAt = p;
          g_camPos += offset;
          g_camSpace = AffineSpace3f::lookAtPoint(g_camPos, g_camLookAt, g_camUp);
        }

        updateCamera();
	}

	//print camera transform
	std::string getCameraTransformStr()
	{
	      AffineSpace3f cam(g_camSpace.l, g_camSpace.p);

	      std::ostringstream stream;
	      stream	<< " " << std::endl
	    		  	<< "Camera transform :" << std::endl
	    		  	<< "-vp " << g_camPos.x    << " " << g_camPos.y    << " " << g_camPos.z    << " " << std::endl
	                << "-vi " << g_camLookAt.x << " " << g_camLookAt.y << " " << g_camLookAt.z << " " << std::endl
	                << "-vu " << g_camUp.x     << " " << g_camUp.y     << " " << g_camUp.z     << " " << std::endl;
	      return stream.str();
	}


	//!< Renderer /////////////////////////////////////////////////////////////////////////////////////////////
	__forceinline int getRendererSPP () const { return g_spp; }
	__forceinline int getRendererMaxDepth () const { return g_depth; }
	__forceinline float getRendererMinContribution () const { return g_mincontribution; }

	void resetRenderer ()
	{
		g_renderer = g_device->rtNewRenderer("pathtracer");

		if (g_depth >= 0) g_device->rtSetInt1(g_renderer, "maxDepth", g_depth);
		g_device->rtSetInt1(g_renderer, "sampler.spp", g_spp);
		g_device->rtSetFloat1(g_renderer, "minContribution", g_mincontribution);
		if (g_backplate) g_device->rtSetImage(g_renderer, "backplate", g_backplate);

		g_device->rtCommit(g_renderer);
	}
	void setRendererSPP (int iSPP) { g_spp = iSPP; resetRenderer(); }
	void setRendererMaxDepth (int iMaxDepth) { g_depth = iMaxDepth; resetRenderer(); }
	void setRendererMinContribution (float iMinContribution) { g_mincontribution = iMinContribution; resetRenderer(); }


	//!< Tonemapper ///////////////////////////////////////////////////////////////////////////////////////////
	__forceinline float getTonemapperGamma () const { return g_gamma; }
	void setTonemapperGamma (float igamma) { g_gamma = igamma; }
	__forceinline bool getTonemapperVignetting () const { return g_vignetting; }
	void setTonemapperVignetting (bool ivignette) { g_vignetting = ivignette; }

	__forceinline float getTonemapperSensitivity () const { return g_sensitivity; }
	void setTonemapperSensitivity (float isensitivity) { g_sensitivity = isensitivity; }
	__forceinline float getTonemapperExposure () const { return g_exposure; }
	void setTonemapperExposure (float iexp) { g_exposure = iexp; }
	__forceinline float getTonemapperFStop () const { return g_fstop; }
	void setTonemapperFStop (float ifstop) { g_fstop = ifstop; }

	void resetTonemapper ()
	{
		  g_tonemapper = g_device->rtNewToneMapper("default");

		  g_device->rtSetFloat1 (g_tonemapper, "gamma", g_gamma);
		  g_device->rtSetBool1 (g_tonemapper, "vignetting", g_vignetting);

		  g_device->rtSetFloat1 (g_tonemapper, "sensitivity", g_sensitivity);
		  g_device->rtSetFloat1 (g_tonemapper, "exposure", g_exposure);
		  g_device->rtSetFloat1 (g_tonemapper, "fstop", g_fstop);

		  g_device->rtCommit(g_tonemapper);
	}


	//!< Scene ////////////////////////////////////////////////////////////////////////////////////////////////
	void parseCmdLine(int argc, char** argv);
	bool buildScene(const std::string&);
	void clearScene();
	void forceClearAccellMem() { /*g_device->rtCleanThatFuckingMem();*/	}
	std::string getSceneStats()	const { return g_device->rtGetSceneStats(g_render_scene).c_str(); }

	void createGlobalObjects ();
	void clearGlobalObjects ();


	//!< Utilities ////////////////////////////////////////////////////////////////////////////////////////////
	__forceinline double getSecs() { return getSeconds(); }



private:
	//static Device* g_device; //|< main device

	////////////////////////////////////////////////////////////////////////////////
	/// Automatic reference counting for local Rombo Handles
	////////////////////////////////////////////////////////////////////////////////
/*
 	template<typename Type>
	class Handle
	{
	public:
		__forceinline Handle ( void ) : handle(NULL) {}
		__forceinline Handle (NullTy) : handle(NULL) {}
		__forceinline Handle( Type const input ) : handle(input) {}

		__forceinline Handle( const Handle& input ) : handle(input.handle) {
			if (handle) g_device->rtIncRef(handle);
		}

		__forceinline ~Handle( void ) {
		  if (handle) g_device->rtDecRef(handle);
		}

		__forceinline Handle& operator =( const Handle& input )
		{
		  if (input.handle) g_device->rtIncRef(input.handle);
		  if (handle) g_device->rtDecRef(handle);
		  handle = input.handle;
		  return *this;
		}

		__forceinline Handle& operator =( NullTy ) {
		  if (handle) g_device->rtDecRef(handle);
		  handle = NULL;
		  return *this;
		}

		__forceinline int getRefCount( void )
		{
			int refnb, hdnb;
			g_device->rtGetReferenceCount(handle, "ciemprez", refnb, hdnb);
			return hdnb;
		}

		__forceinline void destroy( void )
		{
			if (handle) g_device->rtDecRef(handle);
			//g_device->rtForceDestroy(handle);
		}

		__forceinline operator bool() const { return handle != NULL; }
		__forceinline operator Type() const { return handle; }

	private:
		Type handle;
	};
*/

private:

	void displayMode ();
	void outputMode (const FileName& fileName);


	void parseCommandLine (Ref<ParseStream> cin, const FileName& path);
	void parseDebugRenderer (Ref<ParseStream> cin);
	void parsePathTracer (Ref<ParseStream> cin, const FileName& path);

    void setupScene(const AffineSpace3f& camera, float s);


private:

	// Global objects
#ifndef NOLOCALHANDLES
	Handle<Device::RTScene> 					g_render_scene;
	Handle<Device::RTRenderer> 					g_renderer;
	Handle<Device::RTToneMapper> 				g_tonemapper;
	Handle<Device::RTFrameBuffer> 				g_frameBuffer;
	Handle<Device::RTImage> 					g_backplate;
	Handle<Device::RTCamera> 					g_camera;
#else
	Device::RTScene								g_render_scene;
	Device::RTRenderer 							g_renderer;
	Device::RTToneMapper 						g_tonemapper;
	Device::RTFrameBuffer 						g_frameBuffer;
	Device::RTImage 							g_backplate;
	Device::RTCamera 							g_camera;
#endif
	std::vector<Handle<Device::RTPrimitive> > 	g_prims;


	// rendering settings
	std::string g_accel;
	std::string g_tri;

	int g_depth;            	//!< recursion depth
	int g_spp;              	//!< samples per pixel for ordinary rendering
	float g_mincontribution;		//!< min contribution for samples

	// output settings
	int 	g_numBuffers;       //!< number of buffers of the framebuffer

	float 	g_gamma;
	bool 	g_vignetting;
    float 	g_sensitivity;
    float 	g_exposure;
    float 	g_fstop;

	size_t 	g_numThreads;
	int 	g_refine;           //!< refinement mode
	// regression testing mode
	bool 	g_regression;

	// framebuffer size
	size_t 	g_width;
	size_t 	g_height;
	bool g_scene_has_size;

	bool 	g_renderregion;
	Vec2i	g_rr_start;
	Vec2i	g_rr_stop;

	//!< Camera

	// camera settings
	Vector3f g_camPos;
	Vector3f g_camLookAt;
	Vector3f g_camUp;
	float g_camFieldOfView;
	float g_camRadius;

	// orbit camera model
	AffineSpace3f g_camSpace;
	float g_cam_theta;
	float g_cam_phi;
	float g_cam_psi;

	// keyboard controls
	float g_speed;


	// hold cmd line params
	int argc;
	char** argv;
};


}	//end namespace embree
#endif /* ROMBORENDER_H_ */
