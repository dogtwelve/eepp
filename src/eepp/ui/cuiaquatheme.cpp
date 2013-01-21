#include <eepp/ui/cuiaquatheme.hpp>
#include <eepp/ui/cuiwindow.hpp>
#include <eepp/ui/cuiwinmenu.hpp>
#include <eepp/ui/cuipopupmenu.hpp>
#include <eepp/ui/cuiprogressbar.hpp>
#include <eepp/ui/cuicommondialog.hpp>
#include <eepp/ui/cuimessagebox.hpp>
#include <eepp/ui/cuitabwidget.hpp>

namespace EE { namespace UI {

cUIAquaTheme::cUIAquaTheme( const std::string& Name, const std::string& Abbr, cFont * DefaultFont ) :
	cUITheme( Name, Abbr, DefaultFont )
{
	FontSelectedColor( eeColorA( 255, 255, 255, 255 ) );
	FontShadowColor( eeColorA( 255, 255, 255, 150 ) );
}

cUIPopUpMenu * cUIAquaTheme::CreatePopUpMenu( cUIControl * Parent, const eeSize& Size, const eeVector2i& Pos, const Uint32& Flags, Uint32 RowHeight, eeRecti PaddingContainer, Uint32 MinWidth, Uint32 MinSpaceForIcons, Uint32 MinRightMargin ) {
	cUIPopUpMenu::CreateParams MenuParams;
	MenuParams.Parent( Parent );
	MenuParams.PosSet( Pos );
	MenuParams.SizeSet( Size );
	MenuParams.Flags = Flags;
	MenuParams.RowHeight = RowHeight;
	MenuParams.PaddingContainer = PaddingContainer;
	MenuParams.MinWidth = MinWidth;
	MenuParams.MinSpaceForIcons = MinSpaceForIcons;
	MenuParams.MinRightMargin = MinRightMargin;

	if ( UseDefaultThemeValues() ) {
		MenuParams.MinWidth = 100;
		MenuParams.MinSpaceForIcons = 24;
		MenuParams.MinRightMargin = 8;
	}

	return eeNew( cUIPopUpMenu, ( MenuParams ) );
}

cUIProgressBar * cUIAquaTheme::CreateProgressBar( cUIControl * Parent, const eeSize& Size, const eeVector2i& Pos, const Uint32& Flags, bool DisplayPercent, bool VerticalExpand, eeVector2f MovementSpeed, eeRectf FillerMargin ) {
	cUIProgressBar::CreateParams PBParams;
	PBParams.Parent( Parent );
	PBParams.PosSet( Pos );
	PBParams.SizeSet( Size );
	PBParams.Flags = Flags;
	PBParams.DisplayPercent = DisplayPercent;
	PBParams.VerticalExpand = VerticalExpand;
	PBParams.MovementSpeed = MovementSpeed;
	PBParams.FillerMargin = FillerMargin;

	if ( UseDefaultThemeValues() ) {
		PBParams.DisplayPercent = true;
	}

	cUIProgressBar * Ctrl = eeNew( cUIProgressBar, ( PBParams ) );
	Ctrl->Visible( true );
	Ctrl->Enabled( true );
	return Ctrl;
}
cUIWinMenu * cUIAquaTheme::CreateWinMenu( cUIControl * Parent, const eeSize& Size, const eeVector2i& Pos, const Uint32& Flags, Uint32 MarginBetweenButtons, Uint32 ButtonMargin, Uint32 MenuHeight, Uint32 FirstButtonMargin ) {
	cUIWinMenu::CreateParams WinMenuParams;
	WinMenuParams.Parent( Parent );
	WinMenuParams.PosSet( Pos );
	WinMenuParams.SizeSet( Size );
	WinMenuParams.Flags = Flags;
	WinMenuParams.MarginBetweenButtons = MarginBetweenButtons;
	WinMenuParams.ButtonMargin = ButtonMargin;
	WinMenuParams.MenuHeight = MenuHeight;
	WinMenuParams.FirstButtonMargin = FirstButtonMargin;

	if ( UseDefaultThemeValues() ) {
		WinMenuParams.ButtonMargin = 12;
	}

	cUIWinMenu * Ctrl = eeNew( cUIWinMenu, ( WinMenuParams ) );
	Ctrl->Visible( true );
	Ctrl->Enabled( true );
	return Ctrl;
}

cUIWindow * cUIAquaTheme::CreateWindow( cUIControl * Parent, const eeSize& Size, const eeVector2i& Pos, const Uint32& Flags, Uint32 WinFlags, eeSize MinWindowSize, Uint8 BaseAlpha ) {
	cUIWindow::CreateParams WinParams;
	WinParams.Parent( Parent );
	WinParams.PosSet( Pos );
	WinParams.SizeSet( Size );
	WinParams.Flags = Flags;
	WinParams.WinFlags = WinFlags;
	WinParams.MinWindowSize = MinWindowSize;
	WinParams.BaseAlpha = BaseAlpha;

	if ( UseDefaultThemeValues() ) {
		WinParams.Flags |= UI_DRAW_SHADOW;
		WinParams.WinFlags |= UI_WIN_DRAW_SHADOW;
		WinParams.ButtonsPositionFixer.x = -2;
		WinParams.TitleFontColor = eeColorA( 0, 0, 0, 255 );
	}

	return eeNew( cUIWindow, ( WinParams ) );
}

cUICommonDialog * cUIAquaTheme::CreateCommonDialog( cUIControl * Parent, const eeSize& Size, const eeVector2i& Pos, const Uint32& Flags, Uint32 WinFlags, eeSize MinWindowSize, Uint8 BaseAlpha, Uint32 CDLFlags, std::string DefaultFilePattern, std::string DefaultDirectory ) {
	cUICommonDialog::CreateParams DLGParams;
	DLGParams.Parent( Parent );
	DLGParams.PosSet( Pos );
	DLGParams.SizeSet( Size );
	DLGParams.Flags = Flags;
	DLGParams.WinFlags = WinFlags;
	DLGParams.MinWindowSize = MinWindowSize;
	DLGParams.BaseAlpha = BaseAlpha;
	DLGParams.DefaultDirectory = DefaultDirectory;
	DLGParams.DefaultFilePattern = DefaultFilePattern;
	DLGParams.CDLFlags = CDLFlags;

	if ( UseDefaultThemeValues() ) {
		DLGParams.Flags |= UI_DRAW_SHADOW;
		DLGParams.WinFlags |= UI_WIN_DRAW_SHADOW;
		DLGParams.ButtonsPositionFixer.x = -2;
		DLGParams.TitleFontColor = eeColorA( 0, 0, 0, 255 );
	}

	return eeNew( cUICommonDialog, ( DLGParams ) );
}

cUIMessageBox * cUIAquaTheme::CreateMessageBox( UI_MSGBOX_TYPE Type, const String& Message, Uint32 WinFlags, cUIControl * Parent, const eeSize& Size, const eeVector2i& Pos, const Uint32& Flags, eeSize MinWindowSize, Uint8 BaseAlpha ) {
	cUIMessageBox::CreateParams MsgBoxParams;
	MsgBoxParams.Parent( Parent );
	MsgBoxParams.PosSet( Pos );
	MsgBoxParams.SizeSet( Size );
	MsgBoxParams.Flags = Flags;
	MsgBoxParams.WinFlags = WinFlags;
	MsgBoxParams.MinWindowSize = MinWindowSize;
	MsgBoxParams.BaseAlpha = BaseAlpha;
	MsgBoxParams.Type = Type;
	MsgBoxParams.Message = Message;

	if ( UseDefaultThemeValues() ) {
		MsgBoxParams.Flags |= UI_DRAW_SHADOW;
		MsgBoxParams.WinFlags |= UI_WIN_DRAW_SHADOW;
		MsgBoxParams.ButtonsPositionFixer.x = -2;
		MsgBoxParams.TitleFontColor = eeColorA( 0, 0, 0, 255 );
	}

	return eeNew( cUIMessageBox, ( MsgBoxParams ) );
}

cUIComboBox * cUIAquaTheme::CreateComboBox( cUIControl * Parent, const eeSize& Size, const eeVector2i& Pos, const Uint32& Flags, Uint32 MinNumVisibleItems, bool PopUpToMainControl, cUIListBox * ListBox ) {
	cUIComboBox::CreateParams ComboParams;
	ComboParams.Parent( Parent );
	ComboParams.PosSet( Pos );
	ComboParams.SizeSet( Size );
	ComboParams.Flags = Flags;
	ComboParams.MinNumVisibleItems = MinNumVisibleItems;
	ComboParams.PopUpToMainControl = PopUpToMainControl;
	ComboParams.ListBox = ListBox;

	if ( UseDefaultThemeValues() ) {
		ComboParams.Flags |= UI_AUTO_SIZE;
	}

	cUIComboBox * Ctrl = eeNew( cUIComboBox, ( ComboParams ) );
	Ctrl->Visible( true );
	Ctrl->Enabled( true );
	return Ctrl;
}

cUIDropDownList * cUIAquaTheme::CreateDropDownList( cUIControl * Parent, const eeSize& Size, const eeVector2i& Pos, const Uint32& Flags, Uint32 MinNumVisibleItems, bool PopUpToMainControl, cUIListBox * ListBox ) {
	cUIDropDownList::CreateParams DDLParams;
	DDLParams.Parent( Parent );
	DDLParams.PosSet( Pos );
	DDLParams.SizeSet( Size );
	DDLParams.Flags = Flags;
	DDLParams.MinNumVisibleItems = MinNumVisibleItems;
	DDLParams.PopUpToMainControl = PopUpToMainControl;
	DDLParams.ListBox = ListBox;

	if ( UseDefaultThemeValues() ) {
		DDLParams.Flags |= UI_AUTO_SIZE;
	}

	cUIDropDownList * Ctrl = eeNew( cUIDropDownList, ( DDLParams ) );
	Ctrl->Visible( true );
	Ctrl->Enabled( true );
	return Ctrl;
}


cUITabWidget * cUIAquaTheme::CreateTabWidget( cUIControl *Parent, const eeSize &Size, const eeVector2i &Pos, const Uint32 &Flags, const bool &TabsClosable, const bool &SpecialBorderTabs, const Int32 &TabSeparation, const Uint32 &MaxTextLength, const Uint32 &TabWidgetHeight, const Uint32 &TabTextAlign, const Uint32 &MinTabWidth, const Uint32 &MaxTabWidth ) {
	cUITabWidget::CreateParams TabWidgetParams;
	TabWidgetParams.Parent( Parent );
	TabWidgetParams.PosSet( Pos );
	TabWidgetParams.SizeSet( Size );
	TabWidgetParams.Flags = Flags;
	TabWidgetParams.TabsClosable = TabsClosable;
	TabWidgetParams.SpecialBorderTabs = SpecialBorderTabs;
	TabWidgetParams.TabSeparation = TabSeparation;
	TabWidgetParams.MaxTextLength = MaxTextLength;
	TabWidgetParams.TabWidgetHeight = TabWidgetHeight;
	TabWidgetParams.TabTextAlign = TabTextAlign;
	TabWidgetParams.MinTabWidth = MinTabWidth;
	TabWidgetParams.MaxTabWidth = MaxTabWidth;

	if ( UseDefaultThemeValues() ) {
		TabWidgetParams.TabSeparation = -15;
		TabWidgetParams.FontSelectedColor = eeColorA( 0, 0, 0, 255 );
	}

	cUITabWidget * Ctrl = eeNew( cUITabWidget, ( TabWidgetParams ) );
	Ctrl->Visible( true );
	Ctrl->Enabled( true );
	return Ctrl;
}

}}
