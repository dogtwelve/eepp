#ifndef EE_MATHSIZE_HPP
#define EE_MATHSIZE_HPP

#include <eepp/math/vector2.hpp>

namespace EE { namespace Math {

/** @brief A template class to manipulate sizes */
template<typename T>
class tSize : public Vector2<T>
{
	public:
		/** Default constructor, creates a tSize(0,0) */
		tSize();

		/** Creates a tSize of the width and height */
		tSize( const T& Width, const T& Height );

		/** Creates a copy of a size */
		tSize( const tSize<T>& Size );

		/** Creates a size from a Vector2 */
		tSize( const Vector2<T>& Vec );

		/** @return The size width */
		const T& Width() const;

		/** @return The size height */
		const T& Height() const;

		/** Set a new width */
		void Width( const T& width );

		/** Set a new height */
		void Height( const T& height );
};

template <typename T>
tSize<T>::tSize() :
	Vector2<T>( 0, 0 )
{
}

template <typename T>
tSize<T>::tSize( const T& Width, const T& Height ) :
	Vector2<T>( Width, Height )
{
}

template <typename T>
tSize<T>::tSize( const tSize<T>& Size ) :
	Vector2<T>( Size.Width(), Size.Height() )
{
}

template <typename T>
tSize<T>::tSize( const Vector2<T>& Vec ) :
	Vector2<T>( Vec.x, Vec.y )
{
}

template <typename T>
const T& tSize<T>::Width() const {
	return this->x;
}

template <typename T>
const T& tSize<T>::Height() const {
	return this->y;
}

template <typename T>
void tSize<T>::Width( const T& width ) {
	this->x = width;
}

template <typename T>
void tSize<T>::Height( const T& height ) {
	this->y = height;
}

typedef tSize<eeInt> eeSize;
typedef tSize<eeFloat> eeSizef;

}}

#endif
