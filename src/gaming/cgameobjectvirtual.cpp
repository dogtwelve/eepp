#include "cgameobjectvirtual.hpp"

#include "cmap.hpp"
#include "clayer.hpp"
#include "../graphics/cprimitives.hpp"
using namespace EE::Graphics;

namespace EE { namespace Gaming {

cGameObjectVirtual::cGameObjectVirtual( Uint32 DataId, const Uint32& Flags, Uint32 Type, const eeVector2f& Pos ) :
	cGameObject( Flags ),
	mType( Type ),
	mDataId( DataId ),
	mPos( Pos ),
	mLayer( NULL ),
	mShape( NULL )
{
}

cGameObjectVirtual::cGameObjectVirtual( cShape * Shape, const Uint32& Flags, Uint32 Type, const eeVector2f& Pos ) :
	cGameObject( Flags ),
	mType( Type ),
	mDataId( 0 ),
	mPos( Pos ),
	mLayer( NULL ),
	mShape( Shape )
{
	if ( NULL != Shape )
		mDataId = Shape->Id();
}

cGameObjectVirtual::~cGameObjectVirtual() {
}

Uint32 cGameObjectVirtual::Type() const {
	return GAMEOBJECT_TYPE_VIRTUAL;
}

Uint32 cGameObjectVirtual::RealType() const {
	return mType;
}

eeSize cGameObjectVirtual::Size() {
	if ( NULL != mShape )
		return mShape->RealSize();

	if ( NULL != mLayer )
		return mLayer->Map()->TileSize();

	return eeSize( 32, 32 );
}

void cGameObjectVirtual::Draw() {
	if ( NULL != mShape ) {
		mShape->Draw( mPos.x, mPos.y, eeColorA(), 0.f, 1.f, ALPHA_NORMAL, RenderTypeFromFlags() );
	} else {
		cPrimitives P;

		eeColorA C( mDataId );
		C.Alpha = 255;

		P.SetColor( C );

		if ( NULL != mLayer ) {
			eeSize ts = mLayer->Map()->TileSize();
			P.DrawRectangle( mPos.x, mPos.y, ts.x ,ts.y, 0, 1 );
		} else {
			P.DrawRectangle( mPos.x, mPos.y, 32 ,32, 0, 1 );
		}
	}
}

void cGameObjectVirtual::Update() {
}

eeVector2f cGameObjectVirtual::Pos() const {
	return mPos;
}

void cGameObjectVirtual::Pos( eeVector2f pos ) {
	mPos = pos;
}

Uint32 cGameObjectVirtual::DataId() {
	return mDataId;
}

void cGameObjectVirtual::DataId( Uint32 Id ) {
	mDataId = Id;
}

void cGameObjectVirtual::SetLayer( cLayer * Layer ) {
	mLayer = Layer;
}

}}