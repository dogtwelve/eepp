#ifndef EE_SYSTEMCUTILS_H
#define EE_SYSTEMCUTILS_H

#include <eepp/declares.hpp>

namespace EE { namespace System {

class BitOp {
	public:

	/** Write a bit into the Key in the position defined.
	* @param Key The Key to write
	* @param Pos The Position of the bit
	* @param BitWrite 0 for write 0, any other to write 1.
	*/
	template <typename T>
	static inline void WriteBitKey( T * Key, unsigned int Pos, unsigned int BitWrite ) {
		( BitWrite ) ? ( * Key ) |= ( 1 << Pos ) : ( * Key ) &= ~( 1 << Pos );
	}

	/** Read a bit from a 32 bit key, in the position defined
	* @param Key The Key to read
	* @param Pos The Position in the key to read
	* @return True if the bit is 1
	*/
	template <typename T>
	static inline bool ReadBitKey( T * Key, Uint32 Pos ) {
		return 0 != ( ( * Key ) & ( 1 << Pos ) );
	}

	/** Write a bit flag value
	* @param Key The Key to write or remove
	* @param Val The Value to write or remove
	* @param BitWrite 0 to remove, any value to write
	*/
	template <typename T>
	static inline void SetBitFlagValue( T * Key, Uint32 Val, Uint32 BitWrite ) {
		( BitWrite ) ? ( * Key ) |= Val : ( * Key ) &= ~Val;
	}

	/** Swap the bytes order for a 16 bit value */
	static inline Uint16 Swap16( Uint16 Key ) {
		return static_cast<Uint16>( ( Key << 8 ) | ( Key >> 8 ) );
	}

	/** Swap the bytes order for a 32 bit value */
	static inline Uint32 Swap32( Uint32 Key ) {
		return static_cast<Uint32>( ( Key << 24 ) | ( ( Key << 8 ) & 0x00FF0000 ) | ( ( Key >> 8 ) & 0x0000FF00 ) | ( Key >> 24 ) );
	}

	/** Swap the bytes order for a 64 bit value */
	static inline Uint64 Swap64( Uint64 Key ) {
		Uint32 hi, lo;

		/* Separate into high and low 32-bit values and swap them */
		lo = static_cast<Uint32> ( Key & 0xFFFFFFFF );
		Key >>= 32;
		hi = static_cast<Uint32>( Key & 0xFFFFFFFF );
		Key = Swap32( lo );
		Key	<<= 32;
		Key |= Swap32( hi );

		return Key;
	}

	/** Swap little endian 16 bit value to big endian */
	static inline Uint32 SwapBE16( Uint32 Key ) {
		#if EE_ENDIAN == EE_LITTLE_ENDIAN
			return Swap16( Key );
		#else
			return Key;
		#endif
	}

	/** Swap big endian 16 bit value to little endian */
	static inline Uint32 SwapLE16( Uint32 Key ) {
		#if EE_ENDIAN == EE_BIG_ENDIAN
			return Swap16( Key );
		#else
			return Key;
		#endif
	}

	/** Swap little endian 32 bit value to big endian */
	static inline Uint32 SwapBE32( Uint32 Key ) {
		#if EE_ENDIAN == EE_LITTLE_ENDIAN
			return Swap32( Key );
		#else
			return Key;
		#endif
	}

	/** Swap big endian 32 bit value to little endian */
	static inline Uint32 SwapLE32( Uint32 Key ) {
		#if EE_ENDIAN == EE_BIG_ENDIAN
			return Swap32( Key );
		#else
			return Key;
		#endif
	}

	/** Swap little endian 64 bit value to big endian */
	static inline Uint32 SwapBE64( Uint32 Key ) {
		#if EE_ENDIAN == EE_LITTLE_ENDIAN
			return Swap64( Key );
		#else
			return Key;
		#endif
	}

	/** Swap big endian 64 bit value to little endian */
	static inline Uint32 SwapLE64( Uint32 Key ) {
		#if EE_ENDIAN == EE_BIG_ENDIAN
			return Swap64( Key );
		#else
			return Key;
		#endif
	}
};

}

}
#endif
