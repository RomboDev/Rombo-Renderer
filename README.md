Rombo Renderer
==============

Embree based path tracer with support for DOF, BRDFs and arealights, together with tonemapping, a photographic camera and basic .obj import. 
QT based GUI with advanced features like a resizable render region with one-click slots to save and compare prev renders on-the-fly (ie. like softimageXSI render region), a brush like mouse impl to 'paint' where renderer will focus its efforts, animated widgets with touch support (ie. custom num pad and sliders), advanced framebuffer able to render in-view only pixels while dealing with full resolution (ie. supports 16k res where only on-screen framebuffer pixels are actually rendered.. to test full resolution image parts without having to render the full image), thumbnails viewer that saves settings with it for easy switch, GUI centered implementation (ie. you won't never have a slow GUI while rendering because the renderer is micro-paused while doing any GUI work), full output console and editable scene parameters.


![Screenshot1](https://github.com/RomboDev/rombo/blob/alpha_001/screenshots/rombo_snapshot.png?raw=true)
![Screenshot2](https://github.com/RomboDev/rombo/blob/alpha_001/screenshots/rombo_snapshot4b.png?raw=true)
![Screenshot3](https://github.com/RomboDev/rombo/blob/alpha_001/screenshots/rombo_snapshot8.png?raw=true)

