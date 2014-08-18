// Copyright (c) 2010-2011 Zipline Games, Inc. All Rights Reserved.
// http://getmoai.com

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <moai-core/MOAIVideoModesMgr.h>
#include <moai-core/host.h>
#include <host-modules/aku_modules.h>
#include <host-sdl/SDLHost.h>

#include <SDL.h>

#define UNUSED(p) (( void )p)

namespace InputDeviceID {
	enum {
		DEVICE,
		TOTAL,
	};
}

namespace InputSensorID {
	enum {
		KEYBOARD,
		TEXT_INPUT,
		POINTER,
		MOUSE_LEFT,
		MOUSE_MIDDLE,
		MOUSE_RIGHT,
		TOUCH,
		MOUSE_WHEEL,
		TOTAL,
	};
}

static SDL_Window* sWindow = 0;

//================================================================//
// aku callbacks
//================================================================//

void	_AKUEnterFullscreenModeFunc		();
void	_AKUExitFullscreenModeFunc		();
void 	_AKUSetWindowDisplayMode		(const MOAIVideoModesMgr::Mode& mode);
void 	_AKUResizeWindow				(int width, int height);
void	_AKUOpenWindowFunc				( const char* title, int width, int height, bool borderlessWindow );

//----------------------------------------------------------------//
void _AKUEnterFullscreenModeFunc () {

	SDL_SetWindowFullscreen(sWindow, SDL_WINDOW_FULLSCREEN);
}

//----------------------------------------------------------------//
void _AKUExitFullscreenModeFunc () {

	SDL_SetWindowFullscreen(sWindow, 0);
}

void _AKUSetWindowDisplayMode(const MOAIVideoModesMgr::Mode& mode) {

	bool bFullscreen = SDL_GetWindowFlags(sWindow) & SDL_WINDOW_FULLSCREEN;

	if (bFullscreen)
	{
		SDL_DisplayMode sdlDisplayMode;
		sdlDisplayMode.format = SDL_GetWindowPixelFormat(sWindow);
		sdlDisplayMode.w = mode.width;
		sdlDisplayMode.h = mode.height;
		sdlDisplayMode.refresh_rate = mode.rate;
		sdlDisplayMode.driverdata = 0;
		
		_AKUExitFullscreenModeFunc();
		SDL_SetWindowDisplayMode(sWindow, &sdlDisplayMode);
		_AKUEnterFullscreenModeFunc();
	}
	else
	{
		SDL_SetWindowSize(sWindow, mode.width, mode.height);
		SDL_SetWindowPosition(sWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	}

	_AKUResizeWindow(mode.width, mode.height);
}

void _AKUResizeWindow(int width, int height) {

	AKUSetScreenSize(width, height);
	AKUSetViewSize(width, height);
}

//----------------------------------------------------------------//
void _AKUOpenWindowFunc ( const char* title, int width, int height, bool borderlessWindow ) {
	
	if ( !sWindow ) {

		Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

		if ( borderlessWindow ) {
			flags |= SDL_WINDOW_BORDERLESS;
		}
		else {
			flags |= SDL_WINDOW_RESIZABLE;
		}

		sWindow = SDL_CreateWindow ( title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);

		SDL_GL_CreateContext ( sWindow );
		SDL_GL_SetSwapInterval ( 1 );
		AKUDetectGfxContext ();
		AKUSetScreenSize ( width, height );
	}
}

//================================================================//
// helpers
//================================================================//
static void QueryVideoModes		();
static void	Finalize			();
static void	Init				( int argc, char** argv );
static void	MainLoop			();
static void	PrintMoaiVersion	();

//----------------------------------------------------------------//
void QueryVideoModes()
{
	int videoDisplays = SDL_GetNumVideoDisplays();

	// Loop over all the monitors
	for(int i = 0; i < videoDisplays; ++i)
	{
		// Loop over all the modes for the current monitor
		int numDisplayModes = SDL_GetNumDisplayModes(i);

		for(int j = 0; j < numDisplayModes; ++j)
		{
			SDL_DisplayMode mode;
			SDL_GetDisplayMode(i, j, &mode);

			MOAIVideoModesMgr::Get().AddDisplayMode(mode.w, mode.h, mode.refresh_rate);
		}
	}
}

//----------------------------------------------------------------//
void Finalize () {

	AKUModulesAppFinalize ();
	AKUAppFinalize ();
	
	SDL_Quit ();
}

//----------------------------------------------------------------//
void Init ( int argc, char** argv ) {

	SDL_Init ( SDL_INIT_EVERYTHING );

	#ifdef _DEBUG
		PrintMoaiVersion ();
		printf ( "DEBUG BUILD\n" );
	#endif

	AKUAppInitialize ();
	AKUModulesAppInitialize ();

	AKUCreateContext ();
	AKUModulesContextInitialize ();
	AKUModulesRunLuaAPIWrapper ();

	AKUSetInputConfigurationName ( "SDL" );

	MOAIVideoModesMgr::Get().SetWindowModeFunc(_AKUSetWindowDisplayMode);
	QueryVideoModes();

	AKUReserveInputDevices			( InputDeviceID::TOTAL );
	AKUSetInputDevice				( InputDeviceID::DEVICE, "device" );
	
	AKUReserveInputDeviceSensors	( InputDeviceID::DEVICE, InputSensorID::TOTAL );
	AKUSetInputDeviceKeyboard		( InputDeviceID::DEVICE, InputSensorID::KEYBOARD,		"keyboard" );
	AKUSetInputDeviceKeyboard		( InputDeviceID::DEVICE, InputSensorID::TEXT_INPUT,		"textInput" );
	AKUSetInputDevicePointer		( InputDeviceID::DEVICE, InputSensorID::POINTER,		"pointer" );
	AKUSetInputDeviceButton			( InputDeviceID::DEVICE, InputSensorID::MOUSE_LEFT,		"mouseLeft" );
	AKUSetInputDeviceButton			( InputDeviceID::DEVICE, InputSensorID::MOUSE_MIDDLE,	"mouseMiddle" );
	AKUSetInputDeviceButton			( InputDeviceID::DEVICE, InputSensorID::MOUSE_RIGHT,	"mouseRight" );
	AKUSetInputDeviceWheel			( InputDeviceID::DEVICE, InputSensorID::MOUSE_WHEEL,	"mouseWheel" );

	AKUSetFunc_EnterFullscreenMode ( _AKUEnterFullscreenModeFunc );
	AKUSetFunc_ExitFullscreenMode ( _AKUExitFullscreenModeFunc );
	AKUSetFunc_OpenWindow ( _AKUOpenWindowFunc );
	
	AKUModulesParseArgs ( argc, argv );
	
	atexit ( Finalize ); // do this *after* SDL_Init
}

//----------------------------------------------------------------//
void MainLoop () {

	Uint32 lastFrame = SDL_GetTicks();
	
	bool running = true;
	while ( running ) {
	
		SDL_Event sdlEvent;
		
		while ( SDL_PollEvent ( &sdlEvent )) {  
			   
			switch ( sdlEvent.type )  {
			
				case SDL_QUIT:
				
					running = false;
					break;
				
				case SDL_KEYDOWN:
				case SDL_KEYUP:	{
					int key = sdlEvent.key.keysym.sym;
					if (key & 0x40000000) key = (key & 0x3FFFFFFF) + 256;
			
					AKUEnqueueKeyboardEvent ( InputDeviceID::DEVICE, InputSensorID::KEYBOARD, key, sdlEvent.key.type == SDL_KEYDOWN );

				} 	break;

				case SDL_TEXTINPUT:
					AKUEnqueueKeyboardEvent ( InputDeviceID::DEVICE, InputSensorID::TEXT_INPUT, (int)sdlEvent.text.text[0], false );
					break;
					
				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
	
					switch ( sdlEvent.button.button ) {
					
						case SDL_BUTTON_LEFT:
							AKUEnqueueButtonEvent ( InputDeviceID::DEVICE, InputSensorID::MOUSE_LEFT, ( sdlEvent.type == SDL_MOUSEBUTTONDOWN ));
							break;
						
						case SDL_BUTTON_MIDDLE:
							AKUEnqueueButtonEvent ( InputDeviceID::DEVICE, InputSensorID::MOUSE_MIDDLE, ( sdlEvent.type == SDL_MOUSEBUTTONDOWN ));
							break;
						
						case SDL_BUTTON_RIGHT:
							AKUEnqueueButtonEvent ( InputDeviceID::DEVICE, InputSensorID::MOUSE_RIGHT, ( sdlEvent.type == SDL_MOUSEBUTTONDOWN ));
							break;
					}

					break;

				case SDL_MOUSEMOTION:
				
					AKUEnqueuePointerEvent ( InputDeviceID::DEVICE, InputSensorID::POINTER, sdlEvent.motion.x, sdlEvent.motion.y );
					break;
					
				case SDL_MOUSEWHEEL:

					AKUEnqueueWheelEvent ( InputDeviceID::DEVICE, InputSensorID::MOUSE_WHEEL, sdlEvent.wheel.y ); // Note: we are ignoring the x wheel event
					break;

				case SDL_WINDOWEVENT:

					if ( sdlEvent.window.windowID == SDL_GetWindowID(sWindow)) {
						
						switch ( sdlEvent.window.event) {
							
							case SDL_WINDOWEVENT_RESIZED:
								
								int w, h;
								SDL_GetWindowSize(sWindow, &w, &h);
								_AKUResizeWindow( w, h );
								break;
						}
					}
					break;

				
			}
		}
		
		AKUModulesUpdate ();
		
		AKURender ();
		SDL_GL_SwapWindow ( sWindow );
		
		Uint32 frameDelta = ( Uint32 )( AKUGetSimStep () * 1000.0 );
		Uint32 currentFrame = SDL_GetTicks ();
		Uint32 delta = currentFrame - lastFrame;
		
		if ( delta < frameDelta ) {
			SDL_Delay ( frameDelta - delta );
		}
		lastFrame = SDL_GetTicks();
	}
}

//----------------------------------------------------------------//
void PrintMoaiVersion () {

	static const int length = 255;
	char version [ length ];
	AKUGetMoaiVersion ( version, length );
	printf ( "%s\n", version );
}

//================================================================//
// SDLHost
//================================================================//

//----------------------------------------------------------------//
int SDLHost ( int argc, char** argv ) {

	Init ( argc, argv );

	if ( sWindow ) {
		MainLoop ();
	}

	return 0;
}
