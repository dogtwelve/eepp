#ifndef EE_WINDOWCCLIPBOARDSFML_HPP
#define EE_WINDOWCCLIPBOARDSFML_HPP

#ifdef EE_BACKEND_SFML_ACTIVE

#include <eepp/window/base.hpp>
#include <eepp/window/cclipboard.hpp>

namespace EE { namespace Window { namespace Backend { namespace SFML {

class EE_API cClipboardSFML : public cClipboard {
	public:
		virtual ~cClipboardSFML();

		std::string GetText();

		String GetWideText();

		void SetText( const std::string& Text );
	protected:
		friend class cWindowSFML;

		cClipboardSFML( Window::cWindow * window );

		void Init();
};

}}}}

#endif

#endif