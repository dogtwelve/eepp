#include "cmap.hpp"
#include "cgameobjectvirtual.hpp"
#include "cgameobjectshape.hpp"
#include "cgameobjectshapeex.hpp"
#include "cgameobjectsprite.hpp"
#include "ctilelayer.hpp"
#include "cobjectlayer.hpp"

#include "../system/cpackmanager.hpp"

#include "../graphics/cprimitives.hpp"
#include "../graphics/cshapegroupmanager.hpp"
#include "../graphics/ctexturegrouploader.hpp"
#include "../ui/cuithememanager.hpp"
using namespace EE::Graphics;

namespace EE { namespace Gaming {

cMap::cMap() :
	mWindow( cEngine::instance()->GetCurrentWindow() ),
	mLayers( NULL ),
	mFlags( 0 ),
	mMaxLayers( 0 ),
	mLayerCount( 0 ),
	mViewSize( 800, 600 ),
	mBaseColor( 255, 255, 255, 255 ),
	mTileTex( NULL ),
	mLightManager( NULL )
{
	ViewSize( mViewSize );
}

cMap::~cMap() {
	DeleteLayers();
}

void cMap::Reset() {
	DeleteLayers();

	mWindow = NULL;
	mLayers = NULL;
	mFlags	= 0;
	mMaxLayers	= 0;
	mViewSize = eeSize( 800, 600 );
	mBaseColor = eeColorA( 255, 255, 255, 255 );
}

void cMap::DeleteLayers() {
	eeSAFE_DELETE( mLightManager );

	for ( Uint32 i = 0; i < mLayerCount; i++ )
		eeSAFE_DELETE( mLayers[i] );

	eeSAFE_DELETE( mLayers );

	mLayerCount = 0;
}

void cMap::Create( eeSize Size, Uint32 MaxLayers, eeSize TileSize, Uint32 Flags, eeSize viewSize, Window::cWindow * Window ) {
	Reset();

	mWindow		= Window;

	if ( NULL == mWindow )
		mWindow	= cEngine::instance()->GetCurrentWindow();

	mFlags		= Flags;
	mMaxLayers	= MaxLayers;
	mSize		= Size;
	mTileSize	= TileSize;
	mPixelSize	= Size * TileSize;
	mLayers		= eeNewArray( cLayer*, mMaxLayers );

	if ( LightsEnabled() )
		CreateLightManager();

	for ( Uint32 i = 0; i < mMaxLayers; i++ )
		mLayers[i] = NULL;

	ViewSize( viewSize );

	CreateEmptyTile();
}

void cMap::CreateLightManager() {
	eeSAFE_DELETE( mLightManager );
	mLightManager = eeNew( cLightManager, ( this, ( mFlags & MAP_FLAG_LIGHTS_BYVERTEX ) ? true : false ) );
}

void cMap::CreateEmptyTile() {
	//! I create a texture representing an empty tile to render instead of rendering with primitives because is a lot faster, at least with NVIDIA GPUs.
	cTextureFactory * TF = cTextureFactory::instance();
	std::string tileName( "maptile-" + toStr( mTileSize.Width() ) + "x" + toStr( mTileSize.Height() ) );

	cTexture * Tex = TF->GetByName( tileName );

	if ( NULL == Tex ) {
		Uint32 x, y;
		eeColorA Col( 255, 255, 255, 255 );

		cImage Img( mTileSize.Width(), mTileSize.Height(), 4 );

		Img.FillWithColor( eeColorA( 0, 0, 0, 0 ) );

		for ( x = 0; x < Img.Width(); x++ ) {
			Img.SetPixel( x, 0, Col );
			Img.SetPixel( x, mTileSize.y - 1, Col );
		}

		for ( y = 0; y < Img.Height(); y++ ) {
			Img.SetPixel( 0, y, Col );
			Img.SetPixel( mTileSize.x - 1, y, Col );
		}

		Uint32 TileTexId = TF->LoadFromPixels(
			Img.GetPixelsPtr(),
			Img.Width(),
			Img.Height(),
			Img.Channels(),
			false,
			EE_CLAMP_TO_EDGE,
			false,
			false,
			tileName
		);

		mTileTex = TF->GetTexture( TileTexId );
	} else {
		mTileTex = Tex;
	}
}

cLayer * cMap::AddLayer( Uint32 Type, Uint32 flags, std::string name ) {
	eeASSERT( NULL != mLayers );

	if ( mLayerCount >= mMaxLayers )
		return NULL;

	switch ( Type ) {
		case MAP_LAYER_TILED:
			mLayers[ mLayerCount ] = eeNew( cTileLayer, ( this, mSize, flags, name ) );
			break;
		case MAP_LAYER_OBJECT:
			mLayers[ mLayerCount ] = eeNew( cObjectLayer, ( this, flags, name ) );
			break;
		default:
			return NULL;
	}

	mLayerCount++;

	return mLayers[ mLayerCount - 1 ];
}

cLayer* cMap::GetLayer( Uint32 index ) {
	eeASSERT( index < mLayerCount );
	return mLayers[ index ];
}

cLayer* cMap::GetLayerByHash( Uint32 hash ) {
	for ( Uint32 i = 0; i < mLayerCount; i++ ) {
		if ( mLayers[i]->Id() == hash )
			return mLayers[i];
	}

	return NULL;
}

Uint32 cMap::GetLayerIndex( cLayer * Layer ) {
	if ( NULL != Layer ) {
		for ( Uint32 i = 0; i < mLayerCount; i++ ) {
			if ( mLayers[i] == Layer )
				return i;
		}
	}

	return MAP_LAYER_UNKNOWN;
}

cLayer* cMap::GetLayer( const std::string& name ) {
	return GetLayerByHash( MakeHash( name ) );
}

void cMap::Draw() {
	cGlobalBatchRenderer::instance()->Draw();

	if ( ClipedArea() ) {
		mWindow->ClipEnable( mScreenPos.x, mScreenPos.y, mViewSize.x, mViewSize.y );
	}

	GridDraw();

	for ( Uint32 i = 0; i < mLayerCount; i++ ) {
		if ( mLayers[i]->Visible() )
			mLayers[i]->Draw( mOffsetFixed );
	}

	MouseOverDraw();

	if ( mDrawCb.IsSet() )
		mDrawCb();

	if ( ClipedArea() ) {
		mWindow->ClipDisable();
	}
}

void cMap::MouseOverDraw() {
	if ( !DrawTileOver() )
		return;

	cPrimitives P;
	P.SetColor( eeColorA( 255, 0, 0, 255 ) );

	P.DrawRectangle( mOffsetFixed.x + mMouseOverTileFinal.x * mTileSize.x, mOffsetFixed.y + mMouseOverTileFinal.y * mTileSize.y, mTileSize.x, mTileSize.y, 0.f, 1.f, EE_DRAW_LINE );
}

void cMap::GridDraw() {
	if ( DrawBackground() ) {
		cPrimitives P;

		P.SetColor( eeColorA( 0, 0, 0, 50 ) );
		P.DrawRectangle( mScreenPos.x, mScreenPos.y, mViewSize.x, mViewSize.y, 0.f, 1.f );
		P.SetColor( eeColorA( 255, 255, 255, 255 ) );
	}

	if ( !DrawGrid() )
		return;

	if ( 0 == mSize.x || 0 == mSize.y )
		return;

	cGlobalBatchRenderer::instance()->Draw();

	GLi->LoadIdentity();
	GLi->PushMatrix();
	GLi->Translatef( mOffsetFixed.x, mOffsetFixed.y, 0.0f );

	eeVector2i start = StartTile();
	eeVector2i end = EndTile();

	eeFloat tx, ty;

	for ( Int32 x = start.x; x < end.x; x++ ) {
		for ( Int32 y = start.y; y < end.y; y++ ) {
			tx = x * mTileSize.x;

			ty = y * mTileSize.y;

			mTileTex->Draw( tx, ty );
		}
	}

	cGlobalBatchRenderer::instance()->Draw();

	GLi->PopMatrix();
}

void cMap::GetMouseOverTile() {
	eeVector2i mouse = mWindow->GetInput()->GetMousePos();

	eeVector2i MapPos( mouse.x - mScreenPos.x - mOffset.x, mouse.y - mScreenPos.y - mOffset.y );

	MapPos.x = eemax( MapPos.x, 0 );
	MapPos.y = eemax( MapPos.y, 0 );
	MapPos.x = eemin( MapPos.x, mPixelSize.x );
	MapPos.y = eemin( MapPos.y, mPixelSize.y );

	mMouseOverTile.x = MapPos.x / mTileSize.Width();
	mMouseOverTile.y = MapPos.y / mTileSize.Height();

	// Clamped pos
	mMouseOverTileFinal.x = eemin( mMouseOverTile.x, mSize.Width()	- 1 );
	mMouseOverTileFinal.y = eemin( mMouseOverTile.y, mSize.Height()	- 1 );
	mMouseOverTileFinal.x = eemax( mMouseOverTileFinal.x, 0 );
	mMouseOverTileFinal.y = eemax( mMouseOverTileFinal.y, 0 );

	mMouseMapPos = MapPos;
}

void cMap::UpdateScreenAABB() {
	mScreenAABB = eeAABB( -mOffset.x, -mOffset.y, -mOffset.x + mViewSize.Width(), -mOffset.y + mViewSize.Height() );
}

const eeAABB& cMap::GetViewAreaAABB() const {
	return mScreenAABB;
}

void cMap::Update() {
	mOffsetFixed = eeVector2f( (eeFloat)mScreenPos.x, (eeFloat)mScreenPos.y ) + FixOffset();

	GetMouseOverTile();

	UpdateScreenAABB();

	if ( NULL != mLightManager )
		mLightManager->Update();

	for ( Uint32 i = 0; i < mLayerCount; i++ )
		mLayers[i]->Update();

	if ( mUpdateCb.IsSet() )
		mUpdateCb();
}

const eeSize& cMap::ViewSize() const {
	return mViewSize;
}

const eeVector2i& cMap::GetMouseTilePos() const {
	return mMouseOverTileFinal;
}

const eeVector2i& cMap::GetRealMouseTilePos() const {
	return mMouseOverTile;
}

const eeVector2i& cMap::GetMouseMapPos() const {
	return mMouseMapPos;
}

eeVector2f cMap::GetMouseMapPosf() const {
	return eeVector2f( (eeFloat)mMouseMapPos.x, (eeFloat)mMouseMapPos.y );
}

void cMap::ViewSize( const eeSize& viewSize ) {
	mViewSize = viewSize;

	Clamp();

	CalcTilesClip();
}

const eeVector2i& cMap::Position() const {
	return mScreenPos;
}

void cMap::Position( const eeVector2i& position ) {
	mScreenPos = position;
}

const eeVector2f& cMap::Offset() const {
	return mOffset;
}

const eeVector2f& cMap::OffsetFixed() const {
	return mOffsetFixed;
}

const eeVector2i& cMap::StartTile() const {
	return mStartTile;
}

const eeVector2i& cMap::EndTile() const {
	return mEndTile;
}

void cMap::ExtraTiles( const eeVector2i& extra ) {
	mExtraTiles = extra;
}

const eeVector2i& cMap::ExtraTiles() const {
	return mExtraTiles;
}

void cMap::Offset( const eeVector2f& offset ) {
	mOffset = offset;

	Clamp();

	CalcTilesClip();
}

void cMap::CalcTilesClip() {
	if ( mTileSize.x > 0 && mTileSize.y > 0 ) {
		eeVector2f ffoff( FixOffset() );
		eeVector2i foff( (Int32)ffoff.x, (Int32)ffoff.y );

		mStartTile.x	= -foff.x / mTileSize.x - mExtraTiles.x;
		mStartTile.y	= -foff.y / mTileSize.y - mExtraTiles.y;
		mEndTile.x		= mStartTile.x + eeRound( (eeFloat)mViewSize.x / (eeFloat)mTileSize.x ) + 1 + mExtraTiles.x;
		mEndTile.y		= mStartTile.y + eeRound( (eeFloat)mViewSize.y / (eeFloat)mTileSize.y ) + 1 + mExtraTiles.y;

		if ( mStartTile.x < 0 )
			mStartTile.x = 0;

		if ( mStartTile.y < 0 )
			mStartTile.y = 0;

		if ( mEndTile.x > mSize.x )
			mEndTile.x = mSize.x;

		if ( mEndTile.y > mSize.y )
			mEndTile.y = mSize.y;
	}
}

void cMap::Clamp() {
	if ( !ClampBorders() )
		return;

	if ( mOffset.x > 0 )
		mOffset.x = 0;

	if ( mOffset.y > 0 )
		mOffset.y = 0;

	eeSize totSize( mTileSize * mSize );

	if ( -mOffset.x + mViewSize.x > totSize.x )
		mOffset.x = -( totSize.x - mViewSize.x );

	if ( -mOffset.y + mViewSize.y > totSize.y )
		mOffset.y = -( totSize.y - mViewSize.y );

	if ( totSize.x < mViewSize.x )
		mOffset.x = 0;

	if ( totSize.y < mViewSize.y )
		mOffset.y = 0;
}

void cMap::BaseColor( const eeColorA& color ) {
	mBaseColor = color;
}

const eeColorA& cMap::BaseColor() const {
	return mBaseColor;
}

void cMap::DrawGrid( const bool& draw ) {
	SetFlagValue( &mFlags, MAP_FLAG_DRAW_GRID, draw ? 1 : 0 );
}

Uint32 cMap::DrawGrid() const {
	return mFlags & MAP_FLAG_DRAW_GRID;
}

void cMap::DrawBackground( const bool& draw ) {
	SetFlagValue( &mFlags, MAP_FLAG_DRAW_BACKGROUND, draw ? 1 : 0 );
}

Uint32 cMap::DrawBackground() const {
	return mFlags & MAP_FLAG_DRAW_BACKGROUND;
}

Uint32 cMap::ClipedArea() const {
	return mFlags & MAP_FLAG_CLIP_AREA;
}

Uint32 cMap::ClampBorders() const {
	return mFlags & MAP_FLAG_CLAMP_BORDERS;
}

Uint32 cMap::DrawTileOver() const {
	return mFlags & MAP_FLAG_DRAW_TILE_OVER;
}

void cMap::DrawTileOver( const bool& draw ) {
	SetFlagValue( &mFlags, MAP_FLAG_DRAW_TILE_OVER, draw ? 1 : 0 );
}

Uint32 cMap::LightsEnabled() {
	return mFlags & MAP_FLAG_LIGHTS_ENABLED;
}

void cMap::LightsEnabled( const bool& enabled ) {
	SetFlagValue( &mFlags, MAP_FLAG_LIGHTS_ENABLED, enabled ? 1 : 0 );
}

eeVector2f cMap::FixOffset() {
	return eeVector2f( (eeFloat)static_cast<Int32>( mOffset.x ), (eeFloat)static_cast<Int32>( mOffset.y ) );
}

void cMap::Move( const eeVector2f& offset )  {
	Move( offset.x, offset.y );
}

void cMap::Move( const eeFloat& offsetx, const eeFloat& offsety ) {
	Offset( eeVector2f( mOffset.x + offsetx, mOffset.y + offsety ) );
}

cGameObject * cMap::CreateGameObject( const Uint32& Type, const Uint32& Flags, cLayer * Layer, const Uint32& DataId ) {
	switch ( Type ) {
		case GAMEOBJECT_TYPE_SHAPE:
		{
			cGameObjectShape * tShape = eeNew( cGameObjectShape, ( Flags, Layer ) );

			tShape->DataId( DataId );

			return tShape;
		}
		case GAMEOBJECT_TYPE_SHAPEEX:
		{
			cGameObjectShapeEx * tShapeEx = eeNew( cGameObjectShapeEx, ( Flags, Layer ) );

			tShapeEx->DataId( DataId );

			return tShapeEx;
		}
		case GAMEOBJECT_TYPE_SPRITE:
		{
			cGameObjectSprite * tSprite = eeNew( cGameObjectSprite, ( Flags, Layer ) );

			tSprite->DataId( DataId );

			return tSprite;
		}
		default:
		{
			if ( mCreateGOCb.IsSet() ) {
				return mCreateGOCb( Type, Flags, Layer, DataId );
			} else {
				cGameObjectVirtual * tVirtual;
				cShape * tIsShape = cShapeGroupManager::instance()->GetShapeById( DataId );

				if ( NULL != tIsShape ) {
					tVirtual = eeNew( cGameObjectVirtual, ( tIsShape, Layer, Flags, Type ) );
				} else {
					tVirtual = eeNew( cGameObjectVirtual, ( DataId, Layer, Flags, Type ) );
				}

				return tVirtual;
			}
		}
	}

	return NULL;
}

cLightManager * cMap::GetLightManager() const {
	return mLightManager;
}

const eeSize& cMap::TotalSize() const {
	return mPixelSize;
}

const eeSize& cMap::TileSize() const {
	return mTileSize;
}

const eeSize& cMap::Size() const {
	return mSize;
}

const Uint32& cMap::LayerCount() const {
	return mLayerCount;
}

const Uint32& cMap::MaxLayers() const {
	return mMaxLayers;
}

bool cMap::MoveLayerUp( cLayer * Layer ) {
	Uint32 Lindex = GetLayerIndex( Layer );

	if ( Lindex != MAP_LAYER_UNKNOWN && mLayerCount > 1 && ( Lindex < mLayerCount - 1 ) && ( Lindex + 1 < mLayerCount ) ) {
		cLayer * tLayer = mLayers[ Lindex + 1 ];

		mLayers[ Lindex ]		= tLayer;
		mLayers[ Lindex + 1 ]	= Layer;

		return true;
	}

	return false;
}

bool cMap::MoveLayerDown( cLayer * Layer ) {
	Uint32 Lindex = GetLayerIndex( Layer );

	if ( Lindex != MAP_LAYER_UNKNOWN && mLayerCount > 1 && Lindex >= 1 ) {
		cLayer * tLayer = mLayers[ Lindex - 1 ];

		mLayers[ Lindex ]		= tLayer;
		mLayers[ Lindex - 1 ]	= Layer;

		return true;
	}

	return false;
}

bool cMap::RemoveLayer( cLayer * Layer ) {
	Uint32 Lindex = GetLayerIndex( Layer );

	if ( Lindex != MAP_LAYER_UNKNOWN ) {
		eeSAFE_DELETE( mLayers[ Lindex ] );

		cLayer * LastLayer = NULL;

		// Reorder layers, to clean empty layers in between layers.
		for ( Uint32 i = 0; i < mLayerCount; i++ ) {
			if ( i > 0 && NULL != mLayers[i] && NULL == LastLayer ) {
				mLayers[ i - 1 ]	= mLayers[ i ];
				mLayers[ i ]		= NULL;
			}

			LastLayer = mLayers[i];
		}

		mLayerCount--;

		return true;
	}

	return false;
}

void cMap::ClearProperties() {
	mProperties.clear();
}

void cMap::AddProperty( std::string Text, std::string Value ) {
	mProperties[ Text ] = Value;
}

void cMap::EditProperty( std::string Text, std::string Value ) {
	mProperties[ Text ] = Value;
}

void cMap::RemoveProperty( std::string Text ) {
	mProperties.erase( Text );
}

cMap::PropertiesMap& cMap::GetProperties() {
	return mProperties;
}

void cMap::AddVirtualObjectType( const std::string& name ) {
	mObjTypes.push_back( name );
	mObjTypes.unique();
}

void cMap::RemoveVirtualObjectType( const std::string& name ) {
	mObjTypes.remove( name );
}

void cMap::ClearVirtualObjectTypes() {
	mObjTypes.clear();
}

cMap::GOTypesList& cMap::GetVirtualObjectTypes() {
	return mObjTypes;
}

void cMap::SetCreateGameObjectCallback( const CreateGOCb& Cb ) {
	mCreateGOCb = Cb;
}

bool cMap::LoadFromStream( cIOStream& IOS ) {
	sMapHdr MapHdr;
	Uint32 i, z;

	if ( IOS.IsOpen() ) {
		IOS.Read( (char*)&MapHdr, sizeof(sMapHdr) );

		if ( MapHdr.Magic == ( ( 'E' << 0 ) | ( 'E' << 8 ) | ( 'M' << 16 ) | ( 'P' << 24 ) ) ) {
			Create( eeSize( MapHdr.SizeX, MapHdr.SizeY ), MapHdr.MaxLayers, eeSize( MapHdr.TileSizeX, MapHdr.TileSizeY ), MapHdr.Flags );

			BaseColor( eeColorA( MapHdr.BaseColor ) );

			//! Load Properties
			if ( MapHdr.PropertyCount ) {
				sPropertyHdr tProp[ MapHdr.PropertyCount ];

				IOS.Read( (char*)&tProp[0], sizeof(sPropertyHdr) * MapHdr.PropertyCount );

				for ( i = 0; i < MapHdr.PropertyCount; i++ ) {
					AddProperty( std::string( tProp[i].Name ), std::string( tProp[i].Value ) );
				}
			}

			//! Load Shape Groups
			if ( MapHdr.ShapeGroupCount ) {
				sMapShapeGroup tSG[ MapHdr.ShapeGroupCount ];

				IOS.Read( (char*)&tSG[0], sizeof(sMapShapeGroup) * MapHdr.ShapeGroupCount );

				std::vector<std::string> ShapeGroups;

				for ( i = 0; i < MapHdr.ShapeGroupCount; i++ ) {
					ShapeGroups.push_back( std::string( tSG[i].Path ) );
				}

				//! Load the Texture groups if needed
				for ( i = 0; i < ShapeGroups.size(); i++ ) {
					std::string sgname = FileRemoveExtension( FileNameFromPath( ShapeGroups[i] ) );

					if ( NULL == cShapeGroupManager::instance()->GetByName( sgname ) ) {
						cTextureGroupLoader * tgl = eeNew( cTextureGroupLoader, () );

						tgl->Load( tgl->AppPath() + ShapeGroups[i] );

						eeSAFE_DELETE( tgl );
					}
				}
			}

			//! Load Virtual Object Types
			if ( MapHdr.VirtualObjectTypesCount ) {
				sVirtualObj tVObj[ MapHdr.VirtualObjectTypesCount ];

				IOS.Read( (char*)&tVObj[0], sizeof(sVirtualObj) * MapHdr.VirtualObjectTypesCount );

				for ( i = 0; i < MapHdr.VirtualObjectTypesCount; i++ ) {
					AddVirtualObjectType( std::string( tVObj[i].Name ) );
				}
			}

			//! Load Layers
			if ( MapHdr.LayerCount ) {
				sLayerHdr tLayersHdr[ MapHdr.LayerCount ];
				sLayerHdr * tLayerHdr;

				for ( i = 0; i < MapHdr.LayerCount; i++ ) {
					IOS.Read( (char*)&tLayersHdr[i], sizeof(sLayerHdr) );

					tLayerHdr = &(tLayersHdr[i]);

					cLayer * tLayer = AddLayer( tLayerHdr->Type, tLayerHdr->Flags, std::string( tLayerHdr->Name ) );

					tLayer->Offset( eeVector2f( (eeFloat)tLayerHdr->OffsetX, (eeFloat)tLayerHdr->OffsetY ) );

					sPropertyHdr tProps[ tLayerHdr->PropertyCount ];

					IOS.Read( (char*)&tProps[0], sizeof(sPropertyHdr) * tLayerHdr->PropertyCount );

					for ( z = 0; z < tLayerHdr->PropertyCount; z++ ) {
						tLayer->AddProperty( std::string( tProps[z].Name ), std::string( tProps[z].Value ) );
					}
				}

				bool ThereIsTiled = false;

				for ( i = 0; i < mLayerCount; i++ ) {
					if ( NULL != mLayers[i] && mLayers[i]->Type() == MAP_LAYER_TILED ) {
						ThereIsTiled = true;
					}
				}

				Int32 x, y;
				Uint32 tReadFlag = 0;
				cTileLayer * tTLayer;
				cGameObject * tGO;

				if ( ThereIsTiled ) {
					//! First we read the tiled layers.
					for ( y = 0; y < mSize.y; y++ ) {
						for ( x = 0; x < mSize.x; x++ ) {

							//! Read the current tile flags
							IOS.Read( (char*)&tReadFlag, sizeof(Uint32) );

							//! Read every game object header corresponding to this tile
							for ( i = 0; i < mLayerCount; i++ ) {
								if ( tReadFlag & ( 1 << i ) ) {
									tTLayer = reinterpret_cast<cTileLayer*> ( mLayers[i] );

									sMapTileGOHdr tTGOHdr;

									IOS.Read( (char*)&tTGOHdr, sizeof(sMapTileGOHdr) );

									tGO = CreateGameObject( tTGOHdr.Type, tTGOHdr.Flags, mLayers[i], tTGOHdr.Id );

									tTLayer->AddGameObject( tGO, eeVector2i( x, y ) );
								}
							}
						}
					}
				}

				//! Load the game objects from the object layers
				cObjectLayer * tOLayer;

				for ( i = 0; i < mLayerCount; i++ ) {
					if ( NULL != mLayers[i] && mLayers[i]->Type() == MAP_LAYER_OBJECT ) {
						tLayerHdr	= &( tLayersHdr[i] );
						tOLayer		= reinterpret_cast<cObjectLayer*> ( mLayers[i] );

						sMapObjGOHdr tOGOsHdr[ tLayerHdr->ObjectCount ];
						sMapObjGOHdr * tOGOHdr;

						IOS.Read( (char*)&tOGOsHdr, sizeof(sMapObjGOHdr) * tLayerHdr->ObjectCount );

						for ( z = 0; z < tLayerHdr->ObjectCount; z++ ) {
							tOGOHdr = &( tOGOsHdr[z] );

							tGO = CreateGameObject( tOGOHdr->Type, tOGOHdr->Flags, mLayers[i], tOGOHdr->Id );

							tGO->Pos( eeVector2f( tOGOHdr->PosX, tOGOHdr->PosY ) );

							tOLayer->AddGameObject( tGO );
						}
					}
				}

				if ( MapHdr.LightsCount ) {
					CreateLightManager();

					sMapLightHdr tLighsHdr[ MapHdr.LightsCount ];
					sMapLightHdr * tLightHdr;

					IOS.Read( (char*)&tLighsHdr, sizeof(sMapLightHdr) * MapHdr.LightsCount );

					for ( i = 0; i < MapHdr.LightsCount; i++ ) {
						tLightHdr = &( tLighsHdr[ i ] );

						eeColorA tLightColA( tLightHdr->Color );
						eeColor tLightCol( tLightColA.R(), tLightColA.G(), tLightColA.B() );

						mLightManager->AddLight(
							eeNew( cLight, ( tLightHdr->Radius, tLightHdr->PosX, tLightHdr->PosY, tLightCol, (LIGHT_TYPE)tLightHdr->Type ) )
						);
					}
				}
			}

			return true;
		}
	}

	return false;
}

const std::string& cMap::Path() const {
	return mPath;
}

bool cMap::Load( const std::string& path ) {
	if ( FileExists( path ) ) {
		mPath = path;

		cIOStreamFile IOS( mPath, std::ios::in | std::ios::binary );

		return LoadFromStream( IOS );
	}

	return false;
}

bool cMap::LoadFromPack( cPack * Pack, const std::string& FilePackPath ) {
	if ( NULL != Pack && Pack->IsOpen() && -1 != Pack->Exists( FilePackPath ) ) {
		SafeDataPointer PData;

		Pack->ExtractFileToMemory( FilePackPath, PData );

		return LoadFromMemory( reinterpret_cast<const char*> ( PData.Data ), PData.DataSize );
	}

	return false;
}

bool cMap::LoadFromMemory( const char * Data, const Uint32& DataSize ) {
	cIOStreamMemory IOS( Data, DataSize );

	return LoadFromStream( IOS );
}

void cMap::SaveToStream( cIOStream& IOS ) {
	Uint32 i;
	sMapHdr MapHdr;
	cLayer * tLayer;

	std::vector<std::string> ShapeGroups = GetShapeGroups();

	MapHdr.Magic					= ( ( 'E' << 0 ) | ( 'E' << 8 ) | ( 'M' << 16 ) | ( 'P' << 24 ) );
	MapHdr.Flags					= mFlags;
	MapHdr.MaxLayers				= mMaxLayers;
	MapHdr.SizeX					= mSize.Width();
	MapHdr.SizeY					= mSize.Height();
	MapHdr.TileSizeX				= mTileSize.Width();
	MapHdr.TileSizeY				= mTileSize.Height();
	MapHdr.LayerCount				= mLayerCount;
	MapHdr.PropertyCount			= mProperties.size();
	MapHdr.ShapeGroupCount			= ShapeGroups.size();
	MapHdr.VirtualObjectTypesCount	= mObjTypes.size();	//! This is only usefull for the Map Editor, to auto add on the load the virtual object types that where used to create the map.
	MapHdr.BaseColor				= mBaseColor.GetUint32();

	if ( LightsEnabled() && NULL != mLightManager )
		MapHdr.LightsCount = mLightManager->Count();
	else
		MapHdr.LightsCount = 0;

	if ( IOS.IsOpen() ) {
		//! Writes the map header
		IOS.Write( (const char*)&MapHdr, sizeof(sMapHdr) );

		//! Writes the properties of the map
		for ( cMap::PropertiesMap::iterator it = mProperties.begin(); it != mProperties.end(); it++ ) {
			sPropertyHdr tProp;

			memset( tProp.Name, 0, MAP_PROPERTY_SIZE );
			memset( tProp.Value, 0, MAP_PROPERTY_SIZE );

			StrCopy( tProp.Name, (*it).first.c_str(), MAP_PROPERTY_SIZE );
			StrCopy( tProp.Value, (*it).second.c_str(), MAP_PROPERTY_SIZE );

			IOS.Write( (const char*)&tProp, sizeof(sPropertyHdr) );
		}

		//! Writes the shape groups that the map will need and load
		for ( i = 0; i < ShapeGroups.size(); i++ ) {
			sMapShapeGroup tSG;

			memset( tSG.Path, 0, MAP_SHAPEGROUP_PATH_SIZE );

			StrCopy( tSG.Path, ShapeGroups[i].c_str(), MAP_SHAPEGROUP_PATH_SIZE );

			IOS.Write( (const char*)&tSG, sizeof(sMapShapeGroup) );
		}

		//! Writes the names of the virtual object types created in the map editor
		for ( GOTypesList::iterator votit = mObjTypes.begin(); votit != mObjTypes.end(); votit++ ) {
			sVirtualObj tVObjH;

			memset( tVObjH.Name, 0, MAP_PROPERTY_SIZE );

			StrCopy( tVObjH.Name, (*votit).c_str(), MAP_PROPERTY_SIZE );

			IOS.Write( (const char*)&tVObjH, sizeof(sVirtualObj) );
		}

		//! Writes every layer header
		for ( i = 0; i < mLayerCount; i++ ) {
			tLayer = mLayers[i];
			sLayerHdr tLayerH;

			memset( tLayerH.Name, 0, LAYER_NAME_SIZE );

			StrCopy( tLayerH.Name, tLayer->Name().c_str(), LAYER_NAME_SIZE );

			tLayerH.Type			= tLayer->Type();
			tLayerH.Flags			= tLayer->Flags();
			tLayerH.OffsetX			= tLayer->Offset().x;
			tLayerH.OffsetY			= tLayer->Offset().y;

			if ( MAP_LAYER_OBJECT == tLayerH.Type )
				tLayerH.ObjectCount = reinterpret_cast<cObjectLayer*> ( tLayer )->GetObjectCount();
			else
				tLayerH.ObjectCount		= 0;

			cLayer::PropertiesMap& tLayerProp = tLayer->GetProperties();

			tLayerH.PropertyCount	= tLayerProp.size();

			//! Writes the layer header
			IOS.Write( (const char*)&tLayerH, sizeof(sLayerHdr) );

			//! Writes the properties of the current layer
			for ( cLayer::PropertiesMap::iterator lit = tLayerProp.begin(); lit != tLayerProp.end(); lit++ ) {
				sPropertyHdr tProp;

				memset( tProp.Name, 0, MAP_PROPERTY_SIZE );
				memset( tProp.Value, 0, MAP_PROPERTY_SIZE );

				StrCopy( tProp.Name, (*lit).first.c_str(), MAP_PROPERTY_SIZE );
				StrCopy( tProp.Value, (*lit).second.c_str(), MAP_PROPERTY_SIZE );

				IOS.Write( (const char*)&tProp, sizeof(sPropertyHdr) );
			}
		}

		bool ThereIsTiled = false;

		for ( i = 0; i < mLayerCount; i++ ) {
			if ( NULL != mLayers[i] && mLayers[i]->Type() == MAP_LAYER_TILED ) {
				ThereIsTiled = true;
			}
		}

		//! This method is slow, but allows to save big maps with little space needed, i'll add an alternative save method ( just plain layer -> tile object saving )
		Int32 x, y;
		Uint32 tReadFlag = 0, z;
		cTileLayer * tTLayer;
		cGameObject * tObj;

		cGameObject * tObjects[ mLayerCount ];

		if ( ThereIsTiled ) {
			//! First we save the tiled layers.
			for ( y = 0; y < mSize.y; y++ ) {
				for ( x = 0; x < mSize.x; x++ ) {
					//! Reset Layer Read Flags and temporal objects
					tReadFlag		= 0;

					for ( z = 0; z < mLayerCount; z++ )
						tObjects[z] = NULL;

					//! Look at every layer if it's some data on the current tile, in that case it will write a bit flag to
					//! inform that it's an object on the current tile layer, and it will store a temporal reference to the
					//! object to write layer the object header information
					for ( i = 0; i < mLayerCount; i++ ) {
						tLayer = mLayers[i];

						if ( NULL != tLayer && tLayer->Type() == MAP_LAYER_TILED ) {
							tTLayer = reinterpret_cast<cTileLayer*> ( tLayer );

							tObj = tTLayer->GetGameObject( eeVector2i( x, y ) );

							if ( NULL != tObj ) {
								tReadFlag |= 1 << i;

								tObjects[i] = tObj;
							}
						}
					}

					//! Writes the current tile flags
					IOS.Write( (const char*)&tReadFlag, sizeof(Uint32) );

					//! Writes every game object header corresponding to this tile
					for ( i = 0; i < mLayerCount; i++ ) {
						if ( tReadFlag & ( 1 << i ) ) {
							tObj = tObjects[i];

							sMapTileGOHdr tTGOHdr;

							//! The DataId should be the Shape hash name ( at least in the cases of type Shape, ShapeEx and Sprite.
							tTGOHdr.Id		= tObj->DataId();

							//! If the object type is virtual, means that the real type is stored elsewhere.
							if ( tObj->Type() != GAMEOBJECT_TYPE_VIRTUAL ) {
								tTGOHdr.Type	= tObj->Type();
							} else {
								cGameObjectVirtual * tObjV = reinterpret_cast<cGameObjectVirtual*> ( tObj );

								tTGOHdr.Type	= tObjV->RealType();
							}

							tTGOHdr.Flags	= tObj->Flags();

							IOS.Write( (const char*)&tTGOHdr, sizeof(sMapTileGOHdr) );
						}
					}
				}
			}
		}

		//! Then we save the Object layers.
		cObjectLayer * tOLayer;

		for ( i = 0; i < mLayerCount; i++ ) {
			tLayer = mLayers[i];

			if ( NULL != tLayer && tLayer->Type() == MAP_LAYER_OBJECT ) {
				tOLayer = reinterpret_cast<cObjectLayer*> ( tLayer );

				cObjectLayer::ObjList ObjList = tOLayer->GetObjectList();

				for ( cObjectLayer::ObjList::iterator MapObjIt = ObjList.begin(); MapObjIt != ObjList.end(); MapObjIt++ ) {
					tObj = (*MapObjIt);

					sMapObjGOHdr tOGOHdr;

					//! The DataId should be the Shape hash name ( at least in the cases of type Shape, ShapeEx and Sprite.
					tOGOHdr.Id		= tObj->DataId();

					//! If the object type is virtual, means that the real type is stored elsewhere.
					if ( tObj->Type() != GAMEOBJECT_TYPE_VIRTUAL ) {
						tOGOHdr.Type	= tObj->Type();
					} else {
						cGameObjectVirtual * tObjV = reinterpret_cast<cGameObjectVirtual*> ( tObj );

						tOGOHdr.Type	= tObjV->RealType();
					}

					tOGOHdr.Flags	= tObj->Flags();

					tOGOHdr.PosX	= (Int32)tObj->Pos().x;

					tOGOHdr.PosY	= (Int32)tObj->Pos().y;

					IOS.Write( (const char*)&tOGOHdr, sizeof(sMapObjGOHdr) );
				}
			}
		}

		if ( MapHdr.LightsCount && NULL != mLightManager ) {
			cLightManager::LightsList& Lights = mLightManager->GetLights();

			for ( cLightManager::LightsList::iterator LightsIt = Lights.begin(); LightsIt != Lights.end(); LightsIt++ ) {
				cLight * Light = (*LightsIt);

				sMapLightHdr tLightHdr;

				tLightHdr.Radius	= Light->Radius();
				tLightHdr.PosX		= (Int32)Light->Position().x;
				tLightHdr.PosY		= (Int32)Light->Position().y;
				tLightHdr.Color		= eeColorA( Light->Color() ).GetUint32();
				tLightHdr.Type		= Light->Type();

				IOS.Write( (const char*)&tLightHdr, sizeof(sMapLightHdr) );
			}
		}
	}
}

void cMap::Save( const std::string& path ) {
	if ( !IsDirectory( path ) ) {
		cIOStreamFile IOS( path, std::ios::out | std::ios::binary );

		SaveToStream( IOS );

		mPath = path;
	}
}

std::vector<std::string> cMap::GetShapeGroups() {
	cShapeGroupManager * SGM = cShapeGroupManager::instance();
	std::list<cShapeGroup*>& Res = SGM->GetResources();

	std::vector<std::string> items;

	//! Ugly ugly ugly, but i don't see another way
	Uint32 Restricted1 = MakeHash( std::string( "global" ) );
	Uint32 Restricted2 = MakeHash( UI::cUIThemeManager::instance()->DefaultTheme()->ShapeGroup()->Name() );

	for ( std::list<cShapeGroup*>::iterator it = Res.begin(); it != Res.end(); it++ ) {
		if ( (*it)->Id() != Restricted1 && (*it)->Id() != Restricted2 )
			items.push_back( (*it)->Path() );
	}

	return items;
}

void cMap::SetDrawCallback( MapDrawCb Cb ) {
	mDrawCb = Cb;
}

void cMap::SetUpdateCallback( MapUpdateCb Cb ) {
	mUpdateCb = Cb;
}

}}
