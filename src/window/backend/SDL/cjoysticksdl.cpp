#include "cjoysticksdl.hpp"

namespace EE { namespace Window { namespace Backend { namespace SDL {

cJoystickSDL::cJoystickSDL( const Uint32& index ) :
	cJoystick( index ),
	mJoystick( NULL )
{	
}

cJoystickSDL::~cJoystickSDL() {
}

void cJoystickSDL::Open() {
	mJoystick 	= SDL_JoystickOpen( mIndex );

	if ( NULL != mJoystick ) {
		mName 		= SDL_JoystickName( mIndex );
		mHats		= SDL_JoystickNumHats( mJoystick );
		mButtons	= SDL_JoystickNumButtons( mJoystick );
		mAxes		= SDL_JoystickNumAxes( mJoystick );
		mBalls		= SDL_JoystickNumBalls( mJoystick );

		mButtonDown	= mButtonDownLast = mButtonUp = 0;

		if ( mButtons > 32 )
			mButtons = 32;
	}
}

void cJoystickSDL::Close() {
	if( SDL_JoystickOpened( mIndex ) )
		SDL_JoystickClose( mJoystick );

	mJoystick 	= NULL;
	mName		= "";
	mHats = mButtons = mAxes = mBalls = 0;
}

void cJoystickSDL::Update() {
	if ( NULL != mJoystick ) {
		ClearStates();

		for ( Int32 i = 0; i < mButtons; i++ ) {
			UpdateButton( i, 0 != SDL_JoystickGetButton( mJoystick, i ) );
		}

	}
}

Uint8 cJoystickSDL::GetHat( const Int32& index ) {
	if ( index >= 0 && index < mHats )
		return SDL_JoystickGetHat( mJoystick, index );

	return HAT_CENTERED;
}

Int16 cJoystickSDL::GetAxis( const Int32& axis ) {
	if ( axis >= 0 && axis < mAxes )
		return SDL_JoystickGetAxis( mJoystick, axis );

	return 0;
}

eeVector2i cJoystickSDL::GetBallMotion( const Int32& ball ) {
	eeVector2i v;

	if ( ball >= 0 && ball < mBalls )
		SDL_JoystickGetBall( mJoystick, ball, &v.x, &v.y );

	return v;
}


bool cJoystickSDL::Plugged() const {
	return NULL != mJoystick;
}


}}}}
