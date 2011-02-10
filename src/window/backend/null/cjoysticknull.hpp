#ifndef EE_WINDOWCJOYSTICKNULL_HPP
#define EE_WINDOWCJOYSTICKNULL_HPP

#include "../../cjoystick.hpp"

namespace EE { namespace Window { namespace Backend { namespace Null {

class EE_API cJoystickNull : public cJoystick {
	public:
		cJoystickNull( const Uint32& index );

		~cJoystickNull();

		void 		Close();

		void 		Open();

		void		Update();

		Uint8		GetHat( const Int32& index );

		Int16		GetAxis( const Int32& axis );

		eeVector2i	GetBallMotion( const Int32& ball );

		bool		Plugged() const;
	protected:
};

}}}}

#endif