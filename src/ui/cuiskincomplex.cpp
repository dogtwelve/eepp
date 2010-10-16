#include "cuiskincomplex.hpp"
#include "../graphics/cshapegroupmanager.hpp"

namespace EE { namespace UI {

static const char SideSuffix[ cUISkinComplex::SideCount ][4] = {
	"ml", "mr","d","u","ul","ur","dl","dr","m"
};

std::string cUISkinComplex::GetSideSuffix( const Uint32& Side ) {
	eeASSERT( Side < cUISkinComplex::SideCount );

	return std::string( SideSuffix[ Side ] );
}

cUISkinComplex::cUISkinComplex( const std::string& Name ) :
	cUISkin( Name, UISkinComplex )
{
	for ( Int32 x = 0; x < cUISkinState::StateCount; x++ )
		for ( Int32 y = 0; y < SideCount; y++ )
			mShape[ x ][ y ] = NULL;

	SetSkins();
}

cUISkinComplex::~cUISkinComplex() {

}

void cUISkinComplex::Draw( const eeFloat& X, const eeFloat& Y, const eeFloat& Width, const eeFloat& Height, const Uint32& State ) {
	cShape * tShape = mShape[ State ][ UpLeft ];

	eeSize uls;

	if ( NULL != tShape ) {
		uls = tShape->RealSize();

		tShape->Draw( X, Y, mColor[ State ] );
	}

	tShape = mShape[ State ][ DownLeft ];

	eeSize dls;

	if ( NULL != tShape ) {
		dls = tShape->RealSize();

		tShape->Draw( X, Y + Height - dls.Height(), mColor[ State ] );
	}

	tShape = mShape[ State ][ UpRight ];

	eeSize urs;

	if ( NULL != tShape ) {
		urs = tShape->RealSize();

		tShape->Draw( X + Width - urs.Width(), Y, mColor[ State ] );
	}

	tShape = mShape[ State ][ DownRight ];

	eeSize drs;

	if ( NULL != tShape ) {
		drs = tShape->RealSize();

		tShape->Draw( X + Width - drs.Width(), Y + Height - drs.Height(), mColor[ State ] );
	}

	tShape = mShape[ State ][ Left ];

	if ( NULL != tShape ) {
		tShape->DestHeight( Height - uls.Height() - dls.Height() );

		tShape->Draw( X, Y + uls.Height(), mColor[ State ] );

		tShape->ResetDestWidthAndHeight();

		if ( uls.Width() == 0 )
			uls.x = tShape->RealSize().Width();
	}

	tShape = mShape[ State ][ Up ];

	if ( NULL != tShape ) {
		tShape->DestWidth( Width - uls.Width() - urs.Width() );

		tShape->Draw( X + uls.Width(), Y, mColor[ State ] );

		tShape->ResetDestWidthAndHeight();

		if ( urs.Height() == 0 )
			urs.y = tShape->RealSize().Height();

		if ( uls.Height() == 0 )
			uls.y = tShape->RealSize().Height();
	}

	tShape = mShape[ State ][ Right ];

	if ( NULL != tShape ) {
		if ( urs.Width() == 0 )
			urs.x = tShape->RealSize().Width();

		tShape->DestHeight( Height - urs.Height() - drs.Height() );

		tShape->Draw( X + Width - urs.Width(), Y + urs.Height(), mColor[ State ] );

		tShape->ResetDestWidthAndHeight();
	}

	tShape = mShape[ State ][ Down ];

	if ( NULL != tShape ) {
		tShape->DestWidth( Width - dls.Width() - drs.Width() );

		tShape->Draw( X + dls.Width(), Y + Height - tShape->RealSize().Height(), mColor[ State ] );

		tShape->ResetDestWidthAndHeight();
	}

	tShape = mShape[ State ][ Center ];

	if ( NULL != tShape ) {
		tShape->DestWidth( Width - uls.Width() - urs.Width() );
		tShape->DestHeight( Height - uls.Height() - urs.Height() );

		tShape->Draw( X + uls.Width(), Y + uls.Height(), mColor[ State ] );

		tShape->ResetDestWidthAndHeight();
	}
}

void cUISkinComplex::SetSkin( const Uint32& State ) {
	eeASSERT ( State < cUISkinState::StateCount );

	for ( Uint32 Side = 0; Side < SideCount; Side++ ) {

		cShape * Shape = cShapeGroupManager::instance()->GetShapeByName( std::string( mName + "_" + cUISkin::GetSkinStateName( State ) + "_" + SideSuffix[ Side ] ) );

		if ( NULL != Shape )
			mShape[ State ][ Side ] = Shape;
	}
}

cShape * cUISkinComplex::GetShape( const Uint32& State ) const {
	eeASSERT ( State < cUISkinState::StateCount );

	return mShape[ State ][ Center ];
}

void cUISkinComplex::StateNormalToState( const Uint32& State ) {
	if ( NULL == mShape[ State ][ 0 ] ) {
		for ( Uint32 Side = 0; Side < SideCount; Side++ ) {
			mShape[ State ][ Side ] = mShape[ cUISkinState::StateNormal ][ Side ];
		}
	}
}

cUISkinComplex * cUISkinComplex::Copy( const std::string& NewName, const bool& CopyColorsState ) {
	cUISkinComplex * SkinC = eeNew( cUISkinComplex, ( NewName ) );

	if ( CopyColorsState ) {
		SkinC->mColorDefault = mColorDefault;

		memcpy( &SkinC->mColor[0], &mColor[0], cUISkinState::StateCount * sizeof(eeColorA) );
	}

	memcpy( &SkinC->mShape[0], &mShape[0], cUISkinState::StateCount * SideCount * sizeof(cShape*) );

	return SkinC;
}

cUISkin * cUISkinComplex::Copy() {
	return Copy( mName, true );
}

}}