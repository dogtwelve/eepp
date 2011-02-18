#ifndef EE_WINDOWCCURSORNULL_HPP
#define EE_WINDOWCCURSORNULL_HPP

#include "../../ccursor.hpp"

using namespace EE::Window;

namespace EE { namespace Window { namespace Backend { namespace Null {

class cCursorNull : public cCursor {
	protected:
		friend class cCursorManagerNull;

		cCursorNull( cTexture * tex, const eeVector2i& hotspot, const std::string& name, cWindow * window );

		cCursorNull( cImage * img, const eeVector2i& hotspot, const std::string& name, cWindow * window );

		cCursorNull( const std::string& path, const eeVector2i& hotspot, const std::string& name, cWindow * window );

		void Create();
};

}}}}

#endif
