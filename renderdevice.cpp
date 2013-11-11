/*
 * romborender.cpp
 *
 *  Created on: Nov 22, 2012
 *      Author: max
 */
#include "renderdevice.h"

namespace embree
{
//static variables declaration
Device* RomboRenderDevice::g_device;

RomboRenderDevice::RomboRenderDevice( int argc, char *argv[] )
: g_render_scene (NULL)
, g_renderer (NULL)
, g_tonemapper (NULL)
, g_frameBuffer (NULL)
, g_backplate (NULL)

, g_accel ("default")
, g_tri ("default")
, g_depth (-1)
, g_spp (1)
, g_mincontribution (0.01f)

, g_numBuffers (1)

, g_gamma (1.0f)
, g_vignetting (false)
, g_sensitivity (100.0f)
, g_exposure (5.0f)
, g_fstop (5.6f)

, g_numThreads (0)
, g_refine (1)
, g_regression (false)

, g_width (512)
, g_height (512)
, g_scene_has_size (false)

, g_renderregion(false)
, g_rr_start (0,0)
, g_rr_stop (512,512)

, g_camera (NULL)
, g_camPos (Vec3f(0.0f,0.0f,0.0f))
, g_camLookAt (Vec3f(1.0f,0.0f,0.0f))
, g_camUp     (Vec3f(0,1,0))
, g_camFieldOfView (64.0f)
, g_camRadius (0.0f)
, g_cam_theta (0.f)
, g_cam_phi (0.f)
, g_cam_psi (0.f)
, g_speed (1.0f)

, argc (argc)
, argv (argv)
{
	g_device = Device::rtCreateDevice("default");
	createGlobalObjects();
}


bool RomboRenderDevice::buildScene(const std::string& iPath)
{
/*
	if(!iPath.empty())	//then we come from file load/open scene ...
	{
	//parse scene
	FileName file(iPath);
	parseCommandLine(new ParseStream(new LineCommentFilter(file, "#")), file.path());
	}

    if (g_prims.size())
    {
#ifdef DEBUGSCENEMEM
    	std::cout << "RenderDevice -> total primitives : " << g_prims.size() << std::endl;
#endif
    	displayMode();
    	return true;
    }
*/
    return false;
}

void RomboRenderDevice::clearScene()
{
/*
#ifdef DEBUGSCENEMEM
    std::cout << "RenderDevice -> clearing scene " << std::endl;
#endif

#ifdef NOLOCALHANDLES
    g_device->rtClearScene( g_render_scene );
    g_device->rtClearPrims( (Device::RTPrimitive*) &g_prims[0], g_prims.size() );
#endif
    //!< g_device->rtClearPrims( (Device::RTPrimitive*) &g_prims[0], g_prims.size() );

    clearGlobalObjects();

#ifdef DEBUGSCENEMEM
    std::cout << "RenderDevice -> finished clean-up" << std::endl;
#endif
*/
}

/******************************************************************************/
/*                      Global Objects                                  	  */
/******************************************************************************/
void RomboRenderDevice::createGlobalObjects()
{
/*
  g_renderer = g_device->rtNewRenderer("pathtracer");
  if (g_depth >= 0) g_device->rtSetInt1(g_renderer, "maxDepth", g_depth);
  g_device->rtSetInt1(g_renderer, "sampler.spp", g_spp);
  g_device->rtCommit(g_renderer);

  g_tonemapper = g_device->rtNewToneMapper("default");
  g_device->rtSetFloat1(g_tonemapper, "gamma", g_gamma);
  g_device->rtSetBool1(g_tonemapper, "vignetting", g_vignetting);
  g_device->rtCommit(g_tonemapper);

  this->newFramebuffer(g_width, g_height);

  g_backplate = NULL;
*/
}

void RomboRenderDevice::clearGlobalObjects()
{
/*
#ifndef NOLOCALHANDLES
  g_render_scene = null;
  g_camera = null;

  g_renderer = null;
  g_tonemapper = null;
  g_frameBuffer = null;
  g_backplate = null;

  g_prims.clear();

#else
  if(g_frameBuffer!=NULL)
  g_device->rtDestroyFrameBufferData( g_frameBuffer );

  delete g_renderer;
  delete g_tonemapper;
  delete g_frameBuffer;
  delete g_backplate;
  g_renderer = NULL;
  g_tonemapper = NULL;
  g_frameBuffer = NULL;
  g_backplate = NULL;

  delete g_camera;
  g_camera = NULL;

  g_prims.clear();

  delete g_render_scene;
  g_render_scene = NULL;
#endif

  delete g_device;
  g_device = NULL;
*/
}


/******************************************************************************/
/*                      Camera and Scene                                      */
/******************************************************************************/
void RomboRenderDevice::updateCamera(/*const AffineSpace3f& space*/)
{
/*
	// ADDED 19-10-2012
	AffineSpace3f iSpace = g_camSpace;
#ifdef RIGHTHANDLEDCOORDSYS_NOTUSED_ATM
	Vec3f invert(-1.f,1.f,1.f);
	iSpace = space.scale(invert) * space;
#endif

#ifdef NOLOCALHANDLES
  if(g_camera!=NULL){
	  delete g_camera;
	  g_camera = NULL;
  }
#endif
  //! pinhole camera
  if (g_camRadius == 0.0f)
  {
#ifdef NOLOCALHANDLES
	Device::RTCamera camera = g_device->rtNewCamera("pinhole");
#else
	Handle<Device::RTCamera> camera = g_device->rtNewCamera("pinhole");
#endif
    g_device->rtSetTransform(camera, "local2world", copyToArray(iSpace));
    g_device->rtSetFloat1(camera, "angle", g_camFieldOfView);
    g_device->rtSetFloat1(camera, "aspectRatio", float(g_width) / float(g_height));
    g_device->rtCommit(camera);

    g_camera = camera;
  }
  //! depth of field camera
  else
  {
#ifdef NOLOCALHANDLES
	Device::RTCamera camera = g_device->rtNewCamera("depthoffield");
#else
	Handle<Device::RTCamera> camera = g_device->rtNewCamera("depthoffield");
#endif
    g_device->rtSetTransform(camera, "local2world", copyToArray(iSpace));
    g_device->rtSetFloat1(camera, "angle", g_camFieldOfView);
    g_device->rtSetFloat1(camera, "aspectRatio", float(g_width) / float(g_height));
    g_device->rtSetFloat1(camera, "lensRadius", g_camRadius);
    g_device->rtSetFloat1(camera, "focalDistance", length(g_camLookAt - g_camPos));
    g_device->rtCommit(camera);

    g_camera = camera;
  }
*/
}


/******************************************************************************/
/*                      Render frame                                  */
/******************************************************************************/
void RomboRenderDevice::displayMode()
{
/*
  if (!g_renderer) throw std::runtime_error("no renderer set");
  AffineSpace3f camSpace = AffineSpace3f::lookAtPoint(g_camPos, g_camLookAt, g_camUp);
  float speed = 0.02f * length(g_camLookAt - g_camPos);
  setupScene (camSpace, speed);	//!< Create scene
*/
}

void RomboRenderDevice::outputMode(const FileName& fileName)
{
/*
  if (!g_renderer) throw std::runtime_error("no renderer set");

  // render image
  Handle<Device::RTCamera> camera;// = createCamera(AffineSpace3f::lookAtPoint(g_camPos, g_camLookAt, g_camUp))
  Handle<Device::RTScene> scene; //*= createScene()
  g_device->rtSetInt1(g_renderer, "showprogress", 1);
  g_device->rtCommit(g_renderer);
  //g_device->rtRenderFrame(g_renderer, camera, scene, g_tonemapper, g_frameBuffer, 0);
  for (int i=0; i < g_numBuffers; i++) g_device->rtSwapBuffers(g_frameBuffer);

  // store to disk
  void* ptr = g_device->rtMapFrameBuffer(g_frameBuffer);
  Ref<Image3f> image = new Image3f(g_width, g_height, (Col3f*)ptr);
  storeImage(image.cast<Image>(), fileName);
  g_device->rtUnmapFrameBuffer(g_frameBuffer);
*/
}


/******************************************************************************/
/*                      Command line parsing                                  */
/******************************************************************************/
void RomboRenderDevice::parseDebugRenderer(Ref<ParseStream> cin)
{
/*
	#ifdef NOLOCALHANDLES
	delete g_renderer;
	g_renderer = NULL;
	#endif
	g_renderer = g_device->rtNewRenderer("debug");
	if (g_depth >= 0) g_device->rtSetInt1(g_renderer, "maxDepth", g_depth);
	g_device->rtSetInt1(g_renderer, "sampler.spp", g_spp);


	if (cin->peek() != "{") goto finish;
	cin->drop();

	while (cin->peek() != "}")
	{
		std::string tag = cin->getString();
		cin->force("=");
		if (tag == "depth") g_device->rtSetInt1(g_renderer, "maxDepth", cin->getInt());
		else std::cout << "unknown tag \"" << tag << "\" in debug renderer parsing" << std::endl;
	}
	cin->drop();

	finish:
		g_device->rtCommit(g_renderer);
*/
}

void RomboRenderDevice::parsePathTracer(Ref<ParseStream> cin, const FileName& path)
{
/*
	#ifdef NOLOCALHANDLES
	delete g_renderer;
	g_renderer = NULL;
	#endif
	g_renderer = g_device->rtNewRenderer("pathtracer");
	if (g_depth >= 0) g_device->rtSetInt1(g_renderer, "maxDepth", g_depth);
	g_device->rtSetInt1(g_renderer, "sampler.spp", g_spp);
	g_device->rtSetFloat1(g_renderer, "minContribution", g_mincontribution);
	if (g_backplate) g_device->rtSetImage(g_renderer, "backplate", g_backplate);


	if (cin->peek() != "{") goto finish;
	cin->drop();

	while (cin->peek() != "}")
	{
		std::string tag = cin->getString();
		cin->force("=");

		if (tag == "depth")
		{
			g_depth = cin->getInt();
			g_device->rtSetInt1 (g_renderer, "maxDepth", g_depth);
		}
		else if (tag == "spp")
		{
			g_spp = cin->getInt();
			g_device->rtSetInt1 (g_renderer, "sampler.spp", g_spp);
		}
		else if (tag == "minContribution")
		{
			g_mincontribution = cin->getFloat();
			g_device->rtSetFloat1 (g_renderer, "minContribution", g_mincontribution);
		}
		else if (tag == "backplate")
		{
			g_device->rtSetImage (g_renderer, "backplate", loadImage(path + cin->getFileName(), g_device));
		}
		else
		{
			std::cout << "unknown tag \"" << tag << "\" in renderer parsing" << std::endl;
		}
	}
	cin->drop();

	finish:
		g_device->rtCommit(g_renderer);
*/
}


void RomboRenderDevice::parseCmdLine(int argc, char** argv)
{
///	parseCommandLine(new ParseStream(new CommandLineStream(argc, argv)), FileName());
}

void RomboRenderDevice::parseCommandLine(Ref<ParseStream> cin, const FileName& path)
{
/*
  while (true)
  {
    std::string tag = cin->getString();
    if (tag == "") return;

    // parse command line parameters from a file
    else if (tag == "-c") {
      FileName file = path + cin->getFileName();
      parseCommandLine(new ParseStream(new LineCommentFilter(file, "#")), file.path());
    }

    //read model from file
    else if (tag == "-i") {
      std::vector<Device::RTPrimitive> prims = loadScene(path + cin->getFileName(), g_device);
      g_prims.insert(g_prims.end(), prims.begin(), prims.end());
    }

#ifndef DEBUGSCENEMEM
    // triangulated sphere
    else if (tag == "-trisphere")
    {
      Handle<Device::RTShape> sphere = g_device->rtNewShape("sphere");
      const Vec3f P = cin->getVec3f();
      g_device->rtSetFloat3(sphere, "P", P.x, P.y, P.z);
      g_device->rtSetFloat1(sphere, "r", cin->getFloat());
      g_device->rtSetInt1(sphere, "numTheta", cin->getInt());
      g_device->rtSetInt1(sphere, "numPhi", cin->getInt());
      g_device->rtCommit(sphere);

      Handle<Device::RTMaterial> material = g_device->rtNewMaterial("matte");
      g_device->rtSetFloat3(material, "reflection", 1.0f, 0.0f, 0.0f);
      g_device->rtCommit(material);
      g_prims.push_back(g_device->rtNewShapePrimitive(sphere, material, NULL));
    }

    // ambient light source
    else if (tag == "-ambientlight") {
      Handle<Device::RTLight> light = g_device->rtNewLight("ambientlight");
      const Col3f L = cin->getCol3f();
      g_device->rtSetFloat3(light, "L", L.r, L.g, L.b);
      g_device->rtCommit(light);
      g_prims.push_back(g_device->rtNewLightPrimitive(light, NULL));
    }

    // point light source
    else if (tag == "-pointlight") {
      Handle<Device::RTLight> light = g_device->rtNewLight("pointlight");
      const Vec3f P = cin->getVec3f();
      const Col3f I = cin->getCol3f();
      g_device->rtSetFloat3(light, "P", P.x, P.y, P.z);
      g_device->rtSetFloat3(light, "I", I.r, I.g, I.b);
      g_device->rtCommit(light);
      g_prims.push_back(g_device->rtNewLightPrimitive(light, NULL));
    }

    // directional light source
    else if (tag == "-directionallight" || tag == "-dirlight") {
      Handle<Device::RTLight> light = g_device->rtNewLight("directionallight");
      const Vec3f D = cin->getVec3f();
      const Col3f E = cin->getCol3f();
      g_device->rtSetFloat3(light, "D", D.x, D.y, D.z);
      g_device->rtSetFloat3(light, "E", E.r, E.g, E.b);
      g_device->rtCommit(light);
      g_prims.push_back(g_device->rtNewLightPrimitive(light, NULL));
    }

    // distant light source
    else if (tag == "-distantlight") {
      Handle<Device::RTLight> light = g_device->rtNewLight("distantlight");
      const Vec3f D = cin->getVec3f();
      const Col3f L = cin->getCol3f();
      g_device->rtSetFloat3(light, "D", D.x, D.y, D.z);
      g_device->rtSetFloat3(light, "L", L.r, L.g, L.b);
      g_device->rtSetFloat1(light, "halfAngle", cin->getFloat());
      g_device->rtCommit(light);
      g_prims.push_back(g_device->rtNewLightPrimitive(light, NULL));
    }

    // triangular light source
    else if (tag == "-trianglelight") {
      Vec3f P = cin->getVec3f();
      Vec3f U = cin->getVec3f();
      Vec3f V = cin->getVec3f();
      Vec3f L = cin->getVec3f();

      Handle<Device::RTLight> light = g_device->rtNewLight("trianglelight");
      g_device->rtSetFloat3(light, "v0", P.x, P.y, P.z);
      g_device->rtSetFloat3(light, "v1", P.x + U.x, P.y + U.y, P.z + U.z);
      g_device->rtSetFloat3(light, "v2", P.x + V.x, P.y + V.y, P.z + V.z);
      g_device->rtSetFloat3(light, "L",  L.x, L.y, L.z);
      g_device->rtCommit(light);
      g_prims.push_back(g_device->rtNewLightPrimitive(light, NULL));
    }

    // quad light source
    else if (tag == "-quadlight")
    {
      Vec3f P = cin->getVec3f();
      Vec3f U = cin->getVec3f();
      Vec3f V = cin->getVec3f();
      Vec3f L = cin->getVec3f();

      Handle<Device::RTLight> light0 = g_device->rtNewLight("trianglelight");
      g_device->rtSetFloat3(light0, "v0", P.x + U.x + V.x, P.y + U.y + V.y, P.z + U.z + V.z);
      g_device->rtSetFloat3(light0, "v1", P.x + U.x, P.y + U.y, P.z + U.z);
      g_device->rtSetFloat3(light0, "v2", P.x, P.y, P.z);
      g_device->rtSetFloat3(light0, "L",  L.x, L.y, L.z);
      g_device->rtCommit(light0);
      g_prims.push_back(g_device->rtNewLightPrimitive(light0, NULL));

      Handle<Device::RTLight> light1 = g_device->rtNewLight("trianglelight");
      g_device->rtSetFloat3(light1, "v0", P.x + U.x + V.x, P.y + U.y + V.y, P.z + U.z + V.z);
      g_device->rtSetFloat3(light1, "v1", P.x, P.y, P.z);
      g_device->rtSetFloat3(light1, "v2", P.x + V.x, P.y + V.y, P.z + V.z);
      g_device->rtSetFloat3(light1, "L",  L.x, L.y, L.z);
      g_device->rtCommit(light1);
      g_prims.push_back(g_device->rtNewLightPrimitive(light1, NULL));
    }

    // HDRI light source
    else if (tag == "-hdrilight")
    {
      Handle<Device::RTLight> light = g_device->rtNewLight("hdrilight");
      const Col3f L = cin->getCol3f();
      g_device->rtSetFloat3(light, "L", L.r, L.g, L.b);

#ifdef RIGHTHANDLEDCOORDSYS
      //rotate dome -90 degree
      Vec3f v1(1.f,0.f,0.f);        Vec3f v2(0.f,1.f,0.f);
      Vec3f v3(0.f,0.f,1.f);        Vec3f p(0.f,0.f,0.f);
      AffineSpace3f local2world (v1,v2,v3,p);

      Vec3f rAx(0.f,1.f,0.f);
      Vec3f sAx(-1.f,1.f,1.f);
      float rRot = -90.f* M_PI / 180.f;
      AffineSpace3f l2w = local2world * local2world.rotate(rAx, rRot);
      //AffineSpace3f l2w = local2world * local2world.scale(sAx);

      g_device->rtSetTransform(light, "local2world", copyToArray( l2w ));
#endif

      g_device->rtSetImage(light, "image", loadImage(path + cin->getFileName(), g_device));
      g_device->rtCommit(light);
      g_device->rtClear(light);
      g_prims.push_back(g_device->rtNewLightPrimitive(light, NULL));
    }

    // parse camera parameters
    else if (tag == "-vp")     g_camPos         = Vec3f(cin->getVec3f());
    else if (tag == "-vi")     g_camLookAt      = Vec3f(cin->getVec3f());
    else if (tag == "-vd")     g_camLookAt      = g_camPos + cin->getVec3f();
    else if (tag == "-vu")     g_camUp          = cin->getVec3f();
    else if (tag == "-angle")  g_camFieldOfView = cin->getFloat();
    else if (tag == "-fov")    g_camFieldOfView = cin->getFloat();
    else if (tag == "-radius") g_camRadius      = cin->getFloat();


    //frame buffer size
   else if (tag == "-size") {
      g_width = cin->getInt();
      g_height = cin->getInt();
      g_scene_has_size = true;
      //g_frameBuffer = g_device->rtNewFrameBuffer("RGB_FLOAT32", g_width, g_height, 1);
    }


    // acceleration structure to use
    else if (tag == "-accel") g_accel = cin->getString();

    // triangle representation to use
    else if (tag == "-tri") g_tri = cin->getString();


    // set renderer
    else if (tag == "-renderer")
    {
      std::string renderer = cin->getString();
      if      (renderer == "debug"     ) parseDebugRenderer(cin);
      //else if (renderer == "profile"   ) parseProfileRenderer(cin, path);
      else if (renderer == "pt"        ) parsePathTracer(cin, path);
      else if (renderer == "pathtracer") parsePathTracer(cin, path);
      //else if (renderer == "skin"      ) parseSkinRenderer(cin, path);
      else throw std::runtime_error("unknown renderer: " + renderer);
    }

    // set gamma
    else if (tag == "-gamma") {
      g_device->rtSetFloat1(g_tonemapper, "gamma", g_gamma = cin->getFloat());
      g_device->rtCommit(g_tonemapper);
    }

    // set vignetting
    else if (tag == "-vignetting") {
      g_device->rtSetBool1(g_tonemapper, "vignetting", g_vignetting = cin->getInt());
      g_device->rtCommit(g_tonemapper);
    }

    // set sensitivity
    else if (tag == "-sensitivity") {
      g_device->rtSetFloat1(g_tonemapper, "sensitivity", g_sensitivity = cin->getFloat());
      g_device->rtCommit(g_tonemapper);
    }

    // set exposure
    else if (tag == "-exposure") {
      g_device->rtSetFloat1(g_tonemapper, "exposure", g_exposure = cin->getFloat());
      g_device->rtCommit(g_tonemapper);
    }

    // set exposure
    else if (tag == "-fstop") {
      g_device->rtSetFloat1(g_tonemapper, "fstop", g_fstop = cin->getFloat());
      g_device->rtCommit(g_tonemapper);
    }

    // set recursion depth
    else if (tag == "-depth") {
      g_device->rtSetInt1(g_renderer, "maxDepth", g_depth = cin->getInt());
      g_device->rtCommit(g_renderer);
    }

    // set samples per pixel
    else if (tag == "-spp") {
      g_device->rtSetInt1(g_renderer, "sampler.spp", g_spp = cin->getInt());
      g_device->rtCommit(g_renderer);
    }

    // set the backplate
    else if (tag == "-backplate") {
      g_device->rtSetImage(g_renderer, "backplate", g_backplate = loadImage(path + cin->getFileName(), g_device));
      g_device->rtCommit(g_renderer);
    }
#endif

    // render frame
    //else if (tag == "-o") outputMode(path + cin->getFileName());

    // regression testing
    //else if (tag == "-regression")
    //{
    //  g_refine = false;
    //  g_regression = true;
      //GLUTDisplay(AffineSpace3f::lookAtPoint(g_camPos, g_camLookAt, g_camUp), 0.01f);
    //}

    // print version information
    else if (tag == "-version") {
      std::cout << "Rombo renderer version 0.01" << std::endl;
      exit(1);
    }

    // skip unknown command line parameter
    else {
      std::cerr << "unknown command line parameter: " << tag << " ";
      while (cin->peek() != "" && cin->peek()[0] != '-') std::cerr << cin->getString() << " ";
      std::cerr << std::endl;
    }
  }
*/
}


/////////////////////////////////////////////////////////////////////
void RomboRenderDevice::updateCameraMove(int mouseMode, int x, int y, int prevx, int prevy, int wid_width)
{
/*
#ifdef RIGHTHANDLEDCOORDSYS
    //x = g_width-x;
    x = wid_width-x;
#endif
  float dClickX = float(prevx - x);
  float dClickY = float(prevy - y);

  //prevx = x; prevy = y;


  // Rotate camera around look-at point (LMB + mouse move)
  if (mouseMode == 1) {
    float angularSpeed = 0.05f / 180.0f * float(pi);
    float mapping = 1.0f;
    if (g_camUp[1] < 0) mapping = -1.0f;
    g_cam_theta -= mapping * dClickX * angularSpeed;
    g_cam_phi += dClickY * angularSpeed;

    if (g_cam_theta < 0) g_cam_theta += 2.0f * float(pi);
    if (g_cam_theta > 2.0f*float(pi)) g_cam_theta -= 2.0f * float(pi);
    if (g_cam_phi < -1.5f*float(pi)) g_cam_phi += 2.0f*float(pi);
    if (g_cam_phi > 1.5f*float(pi)) g_cam_phi -= 2.0f*float(pi);

    float cosPhi = cosf(g_cam_phi);
    float sinPhi = sinf(g_cam_phi);
    float cosTheta = cosf(g_cam_theta);
    float sinTheta = sinf(g_cam_theta);
    float dist = length(g_camLookAt - g_camPos);
    g_camPos = g_camLookAt + dist * Vec3f(cosPhi * sinTheta, -sinPhi, cosPhi * cosTheta);
    Vec3f viewVec = normalize(g_camLookAt - g_camPos);
    Vec3f approxUp(0.0f, 1.0f, 0.0f);
    if (g_cam_phi < -0.5f*float(pi) || g_cam_phi > 0.5*float(pi)) approxUp = -approxUp;
    Vec3f rightVec = normalize(cross(viewVec, approxUp));
    AffineSpace3f rotate = AffineSpace3f::rotate(viewVec, g_cam_psi);
    g_camUp = xfmVector(rotate, cross(rightVec, viewVec));
  }
  // Pan camera (MMB + mouse move)
  if (mouseMode == 2) {
    float panSpeed = 0.00025f;
    float dist = length(g_camLookAt - g_camPos);
    Vec3f viewVec = normalize(g_camLookAt - g_camPos);
    Vec3f strafeVec = cross(g_camUp, viewVec);
    Vec3f deltaVec = strafeVec * panSpeed * dist * float(dClickX)
      + g_camUp * panSpeed * dist * float(-dClickY);
    g_camPos += deltaVec;
    g_camLookAt += deltaVec;
  }
  // Dolly camera (RMB + mouse move)
  if (mouseMode == 3) {
    float dollySpeed = 0.01f;
    float delta;
    if (fabsf(dClickX) > fabsf(dClickY)) delta = float(dClickX);
    else delta = float(-dClickY);
    float k = powf((1-dollySpeed), delta);
    float dist = length(g_camLookAt - g_camPos);
    Vec3f viewVec = normalize(g_camLookAt - g_camPos);
    g_camPos += dist * (1-k) * viewVec;
  }
  // Roll camera (ALT + LMB + mouse move)
  if (mouseMode == 4) {
    float angularSpeed = 0.1f / 180.0f * float(pi);
    g_cam_psi -= dClickX * angularSpeed;
    Vec3f viewVec = normalize(g_camLookAt - g_camPos);
    Vec3f approxUp(0.0f, 1.0f, 0.0f);
    if (g_cam_phi < -0.5f*float(pi) || g_cam_phi > 0.5*float(pi)) approxUp = -approxUp;
    Vec3f rightVec = normalize(cross(viewVec, approxUp));
    AffineSpace3f rotate = AffineSpace3f::rotate(viewVec, g_cam_psi);
    g_camUp = xfmVector(rotate, cross(rightVec, viewVec));
  }

  g_camSpace = AffineSpace3f::lookAtPoint(g_camPos, g_camLookAt, g_camUp);

  updateCamera();
*/
}


/******************************************************************************/
/*                      OpenGL and Render                                     */
/******************************************************************************/
void RomboRenderDevice::setupScene(const AffineSpace3f& camera, float s)
{
/*
	g_camSpace = camera;
	g_speed = s;

	g_render_scene = //createScene()
	g_device->rtNewScene((g_accel+" "+g_tri).c_str(), (Device::RTPrimitive*)(g_prims.size() == 0 ? NULL : &g_prims[0]), g_prims.size());


	// initialize orbit camera model
	Vec3f viewVec = normalize(g_camLookAt - g_camPos);
	g_cam_theta = atan2f(-viewVec.x, -viewVec.z);
	g_cam_phi = asinf(viewVec.y);
	Vec3f approxUp(0.0f, 1.0f, 0.0f);
	if (g_cam_phi < -0.5f*float(pi) || g_cam_phi > 0.5*float(pi)) approxUp = -approxUp;
	Vec3f rightVec = normalize(cross(viewVec, approxUp));
	Vec3f upUnrotated = cross(rightVec, viewVec);
	g_cam_psi = atan2f(dot(rightVec, g_camUp), dot(upUnrotated, g_camUp));

	updateCamera();
*/
}

}	//namespace end
