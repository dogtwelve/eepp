#include "cjoystickmanagersdl.hpp"
#include "cjoysticksdl.hpp"

#ifdef EE_BACKEND_SDL_ACTIVE

namespace EE { namespace Window { namespace Backend { namespace SDL {

cJoystickManagerSDL::cJoystickManagerSDL() :
	cJoystickManager()
{
}

cJoystickManagerSDL::~cJoystickManagerSDL() {
}

void cJoystickManagerSDL::Update() {
	if ( mInit ) {
		SDL_JoystickUpdate();

		for ( eeUint i = 0; i < mCount; i++ )
			if ( NULL != mJoysticks[i] )
				mJoysticks[i]->Update();
	}
}

void cJoystickManagerSDL::Open() {
	eeInt error = SDL_InitSubSystem( SDL_INIT_JOYSTICK );

	if ( !error ) {
		mCount = SDL_NumJoysticks();

		for ( eeUint i = 0; i < mCount; i++ )
			Create(i);

		mInit = true;
	}
}

void cJoystickManagerSDL::Close() {
	if ( SDL_WasInit( SDL_INIT_JOYSTICK ) ) {
		SDL_QuitSubSystem( SDL_INIT_JOYSTICK );
		
		mInit = false;
	}
}

void cJoystickManagerSDL::Create( const Uint32& index ) {
	if ( NULL != mJoysticks[ index ] )
		mJoysticks[ index ]->ReOpen();
	else
		mJoysticks[ index ] = eeNew( cJoystickSDL, ( index ) );
}

}}}}

#endif
