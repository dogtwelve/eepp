#include "cuilistbox.hpp"
#include "cuimanager.hpp"

namespace EE { namespace UI {

cUIListBox::cUIListBox( cUIListBox::CreateParams& Params ) :
	cUIControlAnim( Params ),
	mRowHeight( Params.RowHeight ),
	mScrollAlwaysVisible( Params.ScrollAlwaysVisible ),
	mSmoothScroll( Params.SmoothScroll ),
	mPaddingContainer( Params.PaddingContainer ),
	mContainer( NULL ),
	mScrollBar( NULL ),
	mHScrollBar( NULL ),
	mFont( Params.Font ),
	mFontColor( Params.FontColor ),
	mFontOverColor( Params.FontOverColor ),
	mFontSelectedColor( Params.FontSelectedColor ),
	mLastPos( 0xFFFFFFFF ),
	mMaxTextWidth(0),
	mAllowHorizontalScroll( Params.AllowHorizontalScroll ),
	mHScrollInit(0),
	mItemsNotVisible(0),
	mLastTickMove(0),
	mVisibleFirst(0),
	mVisibleLast(0)
{
	mType |= UI_TYPE_GET( UI_TYPE_LISTBOX );

	if ( NULL == Params.Font && NULL != cUIThemeManager::instance()->DefaultFont() )
		mFont = cUIThemeManager::instance()->DefaultFont();

	cUIControl::CreateParams CParams;
	CParams.Parent( this );
	CParams.PosSet( mPaddingContainer.Left, mPaddingContainer.Top );
	CParams.Size = eeSize( mSize.Width() - mPaddingContainer.Right - mPaddingContainer.Left, mSize.Height() - mPaddingContainer.Top - mPaddingContainer.Bottom );
	CParams.Flags = Params.Flags;
	mContainer = eeNew( cUIListBoxContainer, ( CParams ) );
	mContainer->Visible( true );
	mContainer->Enabled( true );

	if ( mFlags & UI_CLIP_ENABLE )
		mFlags &= ~UI_CLIP_ENABLE;

	cUIScrollBar::CreateParams ScrollBarP;
	ScrollBarP.Parent( this );
	ScrollBarP.Size = eeSize( 15, mSize.Height() );
	ScrollBarP.PosSet( mSize.Width() - 15, 0 );
	ScrollBarP.Flags = UI_AUTO_SIZE;
	ScrollBarP.VerticalScrollBar = true;
	mScrollBar = eeNew( cUIScrollBar, ( ScrollBarP ) );

	ScrollBarP.Size = eeSize( mSize.Width() - mScrollBar->Size().Width(), 15 );
	ScrollBarP.PosSet( 0, mSize.Height() - 15 );
	ScrollBarP.VerticalScrollBar = false;
	mHScrollBar = eeNew( cUIScrollBar, ( ScrollBarP ) );
	mHScrollBar->Visible( false );
	mHScrollBar->Enabled( false );

	if ( mScrollAlwaysVisible ) {
		mScrollBar->Visible( true );
		mScrollBar->Enabled( true );
	}

	mScrollBar->AddEventListener( cUIEvent::EventOnValueChange, cb::Make1( this, &cUIListBox::OnScrollValueChange ) );
	mHScrollBar->AddEventListener( cUIEvent::EventOnValueChange, cb::Make1( this, &cUIListBox::OnHScrollValueChange ) );

	SetRowHeight();

	ApplyDefaultTheme();
}

cUIListBox::~cUIListBox() {
}

void cUIListBox::SetTheme( cUITheme * Theme ) {
	cUIControl::SetTheme( Theme, "listbox" );

	if ( NULL == mFont && NULL != mSkinState && NULL != mSkinState->GetSkin() && NULL != mSkinState->GetSkin()->Theme() && NULL != mSkinState->GetSkin()->Theme()->Font() )
		mFont = mSkinState->GetSkin()->Theme()->Font();

	AutoPadding();

	OnSizeChange();
}

void cUIListBox::AutoPadding() {
	if ( mFlags & UI_AUTO_PADDING ) {
		mPaddingContainer = MakePadding();
	}
}

cUIScrollBar * cUIListBox::ScrollBar() const {
	return mScrollBar;
}

cUIScrollBar * cUIListBox::HScrollBar() const {
	return mHScrollBar;
}

void cUIListBox::AddListBoxItems( std::vector<std::wstring> Texts ) {
	for ( Uint32 i = 0; i < Texts.size(); i++ ) {
		AddListBoxItem( Texts[i] );
	}

	UpdateScroll();
}

Uint32 cUIListBox::AddListBoxItem( cUIListBoxItem * Item ) {
	mItems.push_back( Item );
	mTexts.push_back( Item->Text() );

	if ( Item->Parent() != mContainer )
		Item->Parent( mContainer );


	UpdateScroll();

	Uint32 tMaxTextWidth = mMaxTextWidth;

	ItemUpdateSize( Item );

	if ( tMaxTextWidth != mMaxTextWidth ) {
		UpdateListBoxItemsSize();
		UpdateScroll();
	}

	return (Uint32)(mItems.size() - 1);
}

Uint32 cUIListBox::AddListBoxItem( const std::string& Text ) {
	return AddListBoxItem( stringTowstring( Text ) );
}

Uint32 cUIListBox::AddListBoxItem( const std::wstring& Text ) {
	mTexts.push_back( Text );
	mItems.push_back( NULL );

	if ( NULL != mFont ) {
		Uint32 twidth = mFont->GetTextWidth( Text );

		if ( twidth > mMaxTextWidth ) {
			mMaxTextWidth = twidth;

			UpdateListBoxItemsSize();
			UpdateScroll();
		}
	}

	return (Uint32)(mItems.size() - 1);
}

cUIListBoxItem * cUIListBox::CreateListBoxItem( const std::wstring& Name ) {
	cUITextBox::CreateParams TextParams;
	TextParams.Parent( mContainer );
	TextParams.Flags 		= UI_VALIGN_CENTER | UI_HALIGN_LEFT;
	TextParams.Font 		= mFont;
	TextParams.FontColor 	= mFontColor;
	cUIListBoxItem * tItem 	= eeNew( cUIListBoxItem, ( TextParams ) );
	tItem->Text( Name );

	return tItem;
}

Uint32 cUIListBox::RemoveListBoxItem( const std::wstring& Text ) {
	return RemoveListBoxItem( GetListBoxItemIndex( Text ) );
}

Uint32 cUIListBox::RemoveListBoxItem( cUIListBoxItem * Item ) {
	return RemoveListBoxItem( GetListBoxItemIndex( Item ) );
}

void cUIListBox::RemoveListBoxItems( std::vector<Uint32> ItemsIndex ) {
	if ( ItemsIndex.size() && 0xFFFFFFFF != ItemsIndex[0] ) {
		std::vector<cUIListBoxItem*> ItemsCpy;
		bool erase;
		mTexts.clear();

		for ( Uint32 i = 0; i < mItems.size(); i++ ) {
			erase = false;

			for ( Uint32 z = 0; z < ItemsIndex.size(); z++ ) {
				if ( ItemsIndex[z] == i ) {
					for ( std::list<Uint32>::iterator it = mSelected.begin(); it != mSelected.end(); it++ ) {
						if ( *it == ItemsIndex[z] ) {
							mSelected.erase( it );

							break;
						}
					}

					ItemsIndex.erase( ItemsIndex.begin() + z );

					erase = true;

					break;
				}
			}

			if ( !erase ) {
				ItemsCpy.push_back( mItems[i] );
				mTexts.push_back( mItems[i]->Text() );
			} else {
				eeSAFE_DELETE( mItems[i] ); // doesn't call to mItems[i]->Close(); because is not checking for close.
			}
		}

		mItems = ItemsCpy;

		UpdateScroll();
		FindMaxWidth();
		UpdateListBoxItemsSize();
	}
}

Uint32 cUIListBox::RemoveListBoxItem( Uint32 ItemIndex ) {
	RemoveListBoxItems( std::vector<Uint32>( 1, ItemIndex ) );

	return ItemIndex;
}

Uint32 cUIListBox::GetListBoxItemIndex( const std::wstring& Name ) {
	Uint32 size = (Uint32)mItems.size();

	for ( Uint32 i = 0; i < size; i++ ) {
		if ( Name == mItems[i]->Text() )
			return i;
	}

	return 0xFFFFFFFF;
}

Uint32 cUIListBox::GetListBoxItemIndex( cUIListBoxItem * Item ) {
	Uint32 size = (Uint32)mItems.size();

	for ( Uint32 i = 0; i < size; i++ ) {
		if ( Item == mItems[i] )
			return i;
	}

	return 0xFFFFFFFF;
}

void cUIListBox::OnScrollValueChange( const cUIEvent * Event ) {
	UpdateScroll( true );
}

void cUIListBox::OnHScrollValueChange( const cUIEvent * Event ) {
	UpdateScroll( true );
}

void cUIListBox::OnSizeChange() {
	mScrollBar->Pos( mSize.Width() - mScrollBar->Size().Width(), 0 );
	mScrollBar->Size( mScrollBar->Size().Width(), mSize.Height() );

	mHScrollBar->Pos( 0, mSize.Height() - mHScrollBar->Size().Height() );
	mHScrollBar->Size( mSize.Width() - mScrollBar->Size().Width(), mHScrollBar->Size().Height() );

	if ( mContainer->IsClipped() && mAllowHorizontalScroll ) {
		if ( (Int32)mMaxTextWidth <= mContainer->Size().Width() ) {
			mHScrollBar->Visible( false );
			mHScrollBar->Enabled( false );
			mHScrollInit = 0;
		}
	}

	ContainerResize();
	UpdateListBoxItemsSize();
	UpdateScroll();
}

void cUIListBox::SetRowHeight() {
	Uint32 tOldRowHeight = mRowHeight;

	if ( 0 == mRowHeight ) {
		Uint32 FontSize = 12;

		if ( NULL != cUIThemeManager::instance()->DefaultFont() )
			FontSize = cUIThemeManager::instance()->DefaultFont()->GetFontSize();

		if ( NULL != mSkinState && NULL != mSkinState->GetSkin() && NULL != mSkinState->GetSkin()->Theme() && NULL != mSkinState->GetSkin()->Theme()->Font() )
			FontSize = mSkinState->GetSkin()->Theme()->Font()->GetFontSize();

		if ( NULL != mFont )
			FontSize = mFont->GetFontSize();

		mRowHeight = (Uint32)( FontSize * 1.5f );
	}

	if ( tOldRowHeight != mRowHeight ) {
		UpdateScroll();
		UpdateListBoxItemsSize();
	}
}

void cUIListBox::FindMaxWidth() {
	Uint32 size = (Uint32)mItems.size();
	Int32 width;

	mMaxTextWidth = 0;

	for ( Uint32 i = 0; i < size; i++ ) {
		if ( NULL != mItems[i] )
			width = (Int32)mItems[i]->GetTextWidth();
		else
			width = mFont->GetTextWidth( mTexts[i] );

		if ( width > (Int32)mMaxTextWidth )
			mMaxTextWidth = (Uint32)width;
	}
}

void cUIListBox::UpdateListBoxItemsSize() {
	Uint32 size = (Uint32)mItems.size();

	for ( Uint32 i = 0; i < size; i++ )
		ItemUpdateSize( mItems[i] );
}

void cUIListBox::ItemUpdateSize( cUIListBoxItem * Item ) {
	if ( NULL != Item ) {
		Int32 width = (Int32)Item->GetTextWidth();

		if ( width > (Int32)mMaxTextWidth )
			mMaxTextWidth = (Uint32)width;

		if ( !mHScrollBar->Visible() ) {
			if ( width < mContainer->Size().Width() )
				width = mContainer->Size().Width();

			if ( ( mItemsNotVisible > 0 || mScrollAlwaysVisible ) )
				width -= mScrollBar->Size().Width();
		} else {
			width = mMaxTextWidth;
		}

		Item->Size( width, mRowHeight );
	}
}

void cUIListBox::ContainerResize() {
	mContainer->Pos( mPaddingContainer.Left, mPaddingContainer.Top );

	if( mHScrollBar->Visible() )
		mContainer->Size( mSize.Width() - mPaddingContainer.Right - mPaddingContainer.Left, mSize.Height() - mPaddingContainer.Top - mHScrollBar->Size().Height() );
	else
		mContainer->Size( mSize.Width() - mPaddingContainer.Right - mPaddingContainer.Left, mSize.Height() - mPaddingContainer.Bottom - mPaddingContainer.Top );
}

void cUIListBox::CreateItemIndex( const Uint32& i ) {
	if ( NULL == mItems[i] ) {
		mItems[i] = CreateListBoxItem( mTexts[i] );

		ItemUpdateSize( mItems[i] );

		for ( std::list<Uint32>::iterator it = mSelected.begin(); it != mSelected.end(); it++ ) {
			if ( *it == i ) {
				mItems[i]->Select();

				break;
			}
		}
	}
}

void cUIListBox::UpdateScroll( bool FromScrollChange ) {
	if ( !mItems.size() )
		return;

	cUIListBoxItem * Item;
	Uint32 i, RelPos = 0, RelPosMax;
	Int32 ItemPos, ItemPosMax;
	Int32 tHLastScroll 		= mHScrollInit;

	Uint32 VisibleItems 	= mContainer->Size().Height() / mRowHeight;
	mItemsNotVisible 		= (Int32)mItems.size() - VisibleItems;

	bool wasScrollVisible 	= mScrollBar->Visible();
	bool wasHScrollVisible 	= mHScrollBar->Visible();

	bool Clipped 			= 0 != mContainer->IsClipped();

	if ( mItemsNotVisible <= 0 ) {
		if ( mScrollAlwaysVisible ) {
			mScrollBar->Visible( true );
			mScrollBar->Enabled( true );
		} else {
			mScrollBar->Visible( false );
			mScrollBar->Enabled( false );
		}
	} else {
		mScrollBar->Visible( true );
		mScrollBar->Enabled( true );
	}

	if ( Clipped && mAllowHorizontalScroll ) {
		if ( ( mScrollBar->Visible() && mContainer->Size().Width() - mScrollBar->Size().Width() < (Int32)mMaxTextWidth ) ||
			( !mScrollBar->Visible() && mContainer->Size().Width() < (Int32)mMaxTextWidth ) ) {
				mHScrollBar->Visible( true );
				mHScrollBar->Enabled( true );

				ContainerResize();

				Int32 ScrollH;

				if ( mScrollBar->Visible() )
					ScrollH = mMaxTextWidth - mContainer->Size().Width() + mScrollBar->Size().Width();
				else
					ScrollH = mMaxTextWidth - mContainer->Size().Width();

				Int32 HScrolleable = (Uint32)( mHScrollBar->Value() * ScrollH );

				mHScrollInit = -HScrolleable;
		} else {
			mHScrollBar->Visible( false );
			mHScrollBar->Enabled( false );

			mHScrollInit = 0;

			ContainerResize();
		}
	}

	VisibleItems 			= mContainer->Size().Height() / mRowHeight;
	mItemsNotVisible 		= (Uint32)mItems.size() - VisibleItems;
	Uint32 Scrolleable 		= (Uint32)mItems.size() * mRowHeight - mContainer->Size().Height();
	bool isScrollVisible 	= mScrollBar->Visible();
	bool isHScrollVisible 	= mHScrollBar->Visible();
	bool FirstVisible 		= false;

	if ( Clipped && mSmoothScroll ) {
		RelPos 		= (Uint32)( mScrollBar->Value() * Scrolleable );
		RelPosMax 	= RelPos + mContainer->Size().Height() + mRowHeight;

		if ( ( FromScrollChange && 0xFFFFFFFF != mLastPos && mLastPos == RelPos ) && ( tHLastScroll == mHScrollInit ) )
			return;

		mLastPos = RelPos;

		for ( i = 0; i < mItems.size(); i++ ) {
			Item = mItems[i];
			ItemPos = mRowHeight * i;
			ItemPosMax = ItemPos + mRowHeight;

			if ( ( ItemPos >= (Int32)RelPos || ItemPosMax >= (Int32)RelPos ) && ( ItemPos <= (Int32)RelPosMax ) ) {
				if ( NULL == Item ) {
					CreateItemIndex( i );
					Item = mItems[i];
				}

				Item->Pos( mHScrollInit, ItemPos - RelPos );
				Item->Enabled( true );
				Item->Visible( true );

				if ( !FirstVisible ) {
					mVisibleFirst = i;
					FirstVisible = true;
				}

				mVisibleLast = i;
			} else {
				eeSAFE_DELETE( mItems[i] );
				Item = NULL;
			}

			if ( NULL != Item ) {
				if ( ( !wasScrollVisible && isScrollVisible ) || ( wasScrollVisible && !isScrollVisible ) ||( !wasHScrollVisible && isHScrollVisible ) || ( wasHScrollVisible && !isHScrollVisible ) )
					ItemUpdateSize( Item );
			}
		}
	} else {
		RelPosMax		= (Uint32)mItems.size();

		if ( mItemsNotVisible > 0 ) {
			RelPos 				= (Uint32)( mScrollBar->Value() * mItemsNotVisible );
			RelPosMax			= RelPos + VisibleItems;
		}

		if ( ( FromScrollChange && 0xFFFFFFFF != mLastPos && mLastPos == RelPos )  && ( !Clipped || tHLastScroll == mHScrollInit ) )
			return;

		mLastPos = RelPos;

		for ( i = 0; i < mItems.size(); i++ ) {
			Item = mItems[i];
			ItemPos = mRowHeight * ( i - RelPos );

			if ( i >= RelPos && i < RelPosMax ) {
				if ( NULL == Item ) {
					CreateItemIndex( i );
					Item = mItems[i];
				}

				if ( Clipped )
					Item->Pos( mHScrollInit, ItemPos );
				else
					Item->Pos( 0, ItemPos );

				Item->Enabled( true );
				Item->Visible( true );

				if ( !FirstVisible ) {
					mVisibleFirst = i;
					FirstVisible = true;
				}

				mVisibleLast = i;
			} else {
				eeSAFE_DELETE( mItems[i] );
				Item = NULL;
			}

			if ( NULL != Item ) {
				if ( ( !wasScrollVisible && isScrollVisible ) || ( wasScrollVisible && !isScrollVisible ) ||( !wasHScrollVisible && isHScrollVisible ) || ( wasHScrollVisible && !isHScrollVisible ) )
					ItemUpdateSize( Item );
			}
		}
	}
}

void cUIListBox::ItemKeyEvent( const cUIEventKey &Event ) {
	cUIEventKey ItemEvent( Event.Ctrl(), cUIEvent::EventOnItemKeyDown, Event.KeyCode(), Event.Char() );
	SendEvent( &ItemEvent );
}

void cUIListBox::ItemClicked( cUIListBoxItem * Item ) {
	cUIEvent ItemEvent( Item, cUIEvent::EventOnItemClicked );
	SendEvent( &ItemEvent );

	if ( !( IsMultiSelect() && cUIManager::instance()->GetInput()->IsKeyDown( KEY_LCTRL ) ) )
		ResetItemsStates();
}

Uint32 cUIListBox::OnSelected() {
	SendCommonEvent( cUIEvent::EventOnSelected );

	return 1;
}

void cUIListBox::ResetItemsStates() {
	for ( Uint32 i = 0; i < mItems.size(); i++ ) {
		if ( NULL != mItems[i] )
			mItems[i]->Unselect();
	}
}

bool cUIListBox::IsMultiSelect() const {
	return 0 != ( mFlags & UI_MULTI_SELECT );
}

cUIListBoxItem * cUIListBox::GetItem( const Uint32& Index ) const {
	eeASSERT( Index < mItems.size() )

	return mItems[ Index ];
}

cUIListBoxItem * cUIListBox::GetItemSelected() {
	if ( mSelected.size() ) {
		if ( NULL == mItems[ mSelected.front() ] )
			CreateItemIndex( mSelected.front() );

		return mItems[ mSelected.front() ];
	}

	return NULL;
}

Uint32 cUIListBox::GetItemSelectedIndex() const {
	if ( mSelected.size() )
		return mSelected.front();

	return 0xFFFFFFFF;
}

std::wstring cUIListBox::GetItemSelectedText() const {
	std::wstring tstr;

	if ( mSelected.size() )
		return mTexts[ mSelected.front() ];

	return tstr;
}

std::list<Uint32> cUIListBox::GetItemsSelectedIndex() const {
	return mSelected;
}

std::list<cUIListBoxItem *> cUIListBox::GetItemsSelected() {
	std::list<cUIListBoxItem *> tItems;
	std::list<Uint32>::iterator it;

	for ( it = mSelected.begin(); it != mSelected.end(); it++ ) {
		if ( NULL == mItems[ *it ] )
			CreateItemIndex( *it );

		tItems.push_back( mItems[ *it ] );
	}

	return tItems;
}

Uint32 cUIListBox::GetItemIndex( cUIListBoxItem * Item ) {
	for ( Uint32 i = 0; i < mItems.size(); i++ ) {
		if ( mItems[i] == Item )
			return i;
	}

	return 0xFFFFFFFF;
}

void cUIListBox::FontColor( const eeColorA& Color ) {
	mFontColor = Color;

	for ( Uint32 i = 0; i < mItems.size(); i++ )
		mItems[i]->Color( mFontColor );
}

const eeColorA& cUIListBox::FontColor() const {
	return mFontColor;
}

void cUIListBox::FontOverColor( const eeColorA& Color ) {
	mFontOverColor = Color;
}

const eeColorA& cUIListBox::FontOverColor() const {
	return mFontOverColor;
}

void cUIListBox::FontSelectedColor( const eeColorA& Color ) {
	mFontSelectedColor = Color;
}

const eeColorA& cUIListBox::FontSelectedColor() const {
	return mFontSelectedColor;
}

void cUIListBox::Font( cFont * Font ) {
	mFont = Font;

	for ( Uint32 i = 0; i < mItems.size(); i++ )
		mItems[i]->Font( mFont );

	FindMaxWidth();
	UpdateListBoxItemsSize();
	UpdateScroll();
}

cFont * cUIListBox::Font() const {
	return mFont;
}

void cUIListBox::PaddingContainer( const eeRecti& Padding ) {
	if ( Padding != mPaddingContainer ) {
		mPaddingContainer = Padding;

		ContainerResize();
		UpdateScroll();
	}
}

const eeRecti& cUIListBox::PaddingContainer() const {
	return mPaddingContainer;
}

void cUIListBox::SmoothScroll( const bool& soft ) {
	if ( soft != mSmoothScroll ) {
		mSmoothScroll = soft;

		UpdateScroll();
	}
}

const bool& cUIListBox::SmoothScroll() const {
	return mSmoothScroll;
}

void cUIListBox::ScrollAlwaysVisible( const bool& visible ) {
	if ( visible != mScrollAlwaysVisible ) {
		mScrollAlwaysVisible = visible;

		UpdateScroll();
	}
}

const bool& cUIListBox::ScrollAlwaysVisible() const {
	return mScrollAlwaysVisible;
}

void cUIListBox::RowHeight( const Uint32& height ) {
	if ( mRowHeight != height ) {
		mRowHeight = height;

		UpdateListBoxItemsSize();
		UpdateScroll();
	}
}

const Uint32& cUIListBox::RowHeight() const {
	return mRowHeight;
}


void cUIListBox::AllowHorizontalScroll( const bool& allow ) {
	if ( allow != mAllowHorizontalScroll ) {
		mAllowHorizontalScroll = allow;

		UpdateScroll();
	}
}

const bool& cUIListBox::AllowHorizontalScroll() const {
	return mAllowHorizontalScroll;
}

Uint32 cUIListBox::Count() {
	return (Uint32)mItems.size();
}

void cUIListBox::SetSelected( Uint32 Index ) {
	if ( Index < mItems.size() ) {
		if ( IsMultiSelect() ) {
			for ( std::list<Uint32>::iterator it = mSelected.begin(); it != mSelected.end(); it++ ) {
				if ( *it == Index )
					return;
			}
		} else {
			if ( mSelected.size() )
				mSelected.clear();
		}

		mSelected.push_back( Index );

		if ( NULL != mItems[ Index ] ) {
			mItems[ Index ]->Select();
		} else {
			UpdateScroll();
		}
	}
}

void cUIListBox::SelectPrev() {
	if ( !IsMultiSelect() && mSelected.size() ) {
		Int32 SelIndex = mSelected.front() - 1;

		if ( SelIndex >= 0 ) {
			if ( NULL == mItems[ mSelected.front() ] )
				CreateItemIndex( mSelected.front() );

			if ( NULL == mItems[ SelIndex ] )
				CreateItemIndex( SelIndex );

			mItems[ mSelected.front() 		]->Unselect();

			if ( ScrollBar()->Visible() ) {
				if ( mItems[ SelIndex ]->Pos().y < 0 ) {
					ScrollBar()->Value( (eeFloat)( SelIndex * mRowHeight ) / (eeFloat)( ( mItems.size() - 1 ) * mRowHeight ) );

					mItems[ SelIndex ]->SetFocus();
				}
			}

			mItems[ SelIndex ]->Select();
		}
	}
}

void cUIListBox::SelectNext() {
	if ( !IsMultiSelect() && mSelected.size() ) {
		Int32 SelIndex = mSelected.front() + 1;

		if ( SelIndex < (Int32)mItems.size() ) {
			if ( NULL == mItems[ mSelected.front() ] )
				CreateItemIndex( mSelected.front() );

			if ( NULL == mItems[ SelIndex ] )
				CreateItemIndex( SelIndex );

			mItems[ mSelected.front() 		]->Unselect();

			if ( ScrollBar()->Visible() ) {
				if ( mItems[ SelIndex ]->Pos().y + (Int32)RowHeight() > mContainer->Size().Height() ) {
					ScrollBar()->Value( (eeFloat)( SelIndex * mRowHeight ) / (eeFloat)( ( mItems.size() - 1 ) * mRowHeight ) );

					mItems[ SelIndex ]->SetFocus();
				}
			}

			mItems[ SelIndex 	]->Select();
		}
	}
}

Uint32 cUIListBox::OnKeyDown( const cUIEventKey &Event ) {
	cUIControlAnim::OnKeyDown( Event );

	ManageKeyboard();

	return 1;
}

void cUIListBox::ManageKeyboard() {
	if ( !mSelected.size() || mFlags & UI_MULTI_SELECT )
		return;

	cInput * KM 	= cUIManager::instance()->GetInput();

	if ( eeGetTicks() - mLastTickMove > 100 ) {
		if ( KM->IsKeyDown( KEY_DOWN ) ) {
			mLastTickMove = eeGetTicks();

			SelectNext();
		} else if ( KM->IsKeyDown( KEY_UP ) ) {
			mLastTickMove = eeGetTicks();

			SelectPrev();
		} else if ( KM->IsKeyDown( KEY_PAGEUP ) ) {
			mLastTickMove = eeGetTicks();

			if ( mSelected.front() != 0 ) {
				mItems[ mSelected.front() ]->Unselect();

				ScrollBar()->Value( 0 );

				mItems[ 0 ]->SetFocus();

				mItems[ 0 ]->Select();
			}
		} else if ( KM->IsKeyDown( KEY_PAGEDOWN ) ) {
			mLastTickMove = eeGetTicks();

			if ( mSelected.front() != Count() - 1 ) {
				mItems[ mSelected.front() ]->Unselect();

				ScrollBar()->Value( 1 );

				mItems[ Count() - 1 ]->SetFocus();

				mItems[ Count() - 1 ]->Select();
			}
		}
	}
}

Uint32 cUIListBox::OnMessage( const cUIMessage * Msg ) {
	switch ( Msg->Msg() ) {
		case cUIMessage::MsgFocusLoss:
		{
			cUIControl * FocusCtrl = cUIManager::instance()->FocusControl();

			if ( this != FocusCtrl && !IsParentOf( FocusCtrl ) ) {
				SendCommonEvent( cUIEvent::EventOnComplexControlFocusLoss );
				OnComplexControlFocusLoss();
			}
			
			return 1;
		}
	}

	return 0;
}

void cUIListBox::OnAlphaChange() {
	cUIControlAnim::OnAlphaChange();

	mScrollBar->Alpha( mAlpha );
	mHScrollBar->Alpha( mAlpha );
}

}}
