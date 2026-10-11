// Advanced Combat Training: the pieces of the Supply Drops menus (sd_*.menu, popup_sd_variant.menu).
// The game fills the sd_* dvars (SupplyDrops.cpp); a weapon card reads the dvars of its prefix ("sd_rv0", "sd_f"...).

// --- Rarity colours: -1 base weapon (grey), 0 Enlisted (green), 1 Professional (blue), 2 Elite (gold) ----------------
#define SD_RARITY_R(v)		( ( ( v ) == 0 ) * 0.47 + ( ( v ) == 1 ) * 0.32 + ( ( v ) == 2 ) * 1.0 + ( ( v ) < 0 ) * 0.62 )
#define SD_RARITY_G(v)		( ( ( v ) == 0 ) * 0.86 + ( ( v ) == 1 ) * 0.64 + ( ( v ) == 2 ) * 0.74 + ( ( v ) < 0 ) * 0.62 )
#define SD_RARITY_B(v)		( ( ( v ) == 0 ) * 0.38 + ( ( v ) == 1 ) * 1.0 + ( ( v ) == 2 ) * 0.2 + ( ( v ) < 0 ) * 0.62 )

// The colour of a card's rarity, set by the game as percentages (<prefix>_cr, _cg, _cb)
#define SD_COLOR_R(P)		( dvarint( P + "_cr" ) / 100.0 )
#define SD_COLOR_G(P)		( dvarint( P + "_cg" ) / 100.0 )
#define SD_COLOR_B(P)		( dvarint( P + "_cb" ) / 100.0 )

// 0..1 without max(): x when 0 < x < 1
#define SD_CLAMP01(x)		min( 1 , ( x ) * ( ( x ) > 0 ) )


// A command that does nothing, for an empty macro argument
#define SD_NOP				setLocalVarBool ui_sd_nop 1;

// --- A plain rectangle --------------------------------------------------------------------------------------------
#define SD_RECT(x, y, w, h, ah, av, r, g, b, a, visArg) \
	itemDef \
	{ \
		rect				x y w h ah av \
		style				WINDOW_STYLE_SHADER \
		background			"white" \
		forecolor			r g b a \
		decoration \
		visible				visArg \
	}

// --- A text --------------------------------------------------------------------------------------------------------
#define SD_TEXT(x, y, w, h, ah, av, font, scale, align, r, g, b, a, textExpr, visArg) \
	itemDef \
	{ \
		rect				x y w h ah av \
		forecolor			r g b a \
		textfont			font \
		textscale			scale \
		textalign			align \
		decoration \
		visible				visArg \
		exp					text ( textExpr ) \
	}

// --- A button of the left column, like the stock ones, with a text from an expression -----------------------------
#define SD_BUTTON(itemIndex, buttonName, textExpr, actionArg, focusArg, hintArg, visArg) \
	itemDef \
	{ \
		name				buttonName \
		rect				CHOICE_RECT(itemIndex) \
		forecolor			CHOICE_TEXTCOLOR \
		disablecolor		1 1 1 0.35 \
		style				WINDOW_STYLE_FILLED \
		background			HIGHLIGHT_SHADER_ART \
		group				"mw2_button" \
		type				ITEM_TYPE_BUTTON \
		textAlign			ITEM_ALIGN_TOP_RIGHT \
		textAlignX			-60 \
		textScale			CHOICE_TEXTSIZE \
		textFont			CHOICE_TEXTFONT \
		visible				visArg \
		exp					text ( textExpr ) \
		mouseEnter \
		{ \
			play			CHOICE_FOCUS_SOUND; \
		} \
		onFocus \
		{ \
			setItemColor	self backColor 0 0 0 1; \
			setLocalVarBool	ui_menuAButton 1; \
			setLocalVarFloat ui_popupYPos 0; \
			setLocalVarString ui_hint_text hintArg; \
			focusArg \
		} \
		leaveFocus \
		{ \
			setItemColor	self backColor 0 0 0 0; \
			setLocalVarString ui_hint_text "@NULL_EMPTY"; \
		} \
		action \
		{ \
			play			CHOICE_CLICK_SOUND; \
			actionArg \
		} \
	}

// --- The hint at the bottom left -----------------------------------------------------------------------------------
#define SD_HINT \
	itemDef \
	{ \
		rect				0 380 215 18 HORIZONTAL_ALIGN_LEFT VERTICAL_ALIGN_TOP \
		forecolor			1 1 1 0.65 \
		textAlign			6 \
		textScale			CHOICE_TEXTSIZE \
		textFont			UI_FONT_SMALL \
		decoration \
		autoWrapped \
		visible				1 \
		exp					text ( localvarstring( "ui_hint_text" ) ) \
	}

// --- A stat bar of a weapon card: the stock value of the weapon (mp/statsTable.csv) and what the variant adds ----
// P card prefix, col the column of the stat, deltaExpr the change (bar points), A alpha, DY y offset
// The stock value of a stat, 0..1 (the game sets <prefix>_s0.._s4: accuracy, damage, range, fire rate, mobility)
#define SD_BAR_FRAC(P, col)			( dvarint( P + col ) / 100.0 )

#define SD_BAR(P, X, Y, BW, ah, av, labelArg, col, deltaExpr, A, DY, visArg) \
	itemDef \
	{ \
		rect				X Y 80 12 ah av \
		forecolor			1 1 1 0.75 \
		textfont			UI_FONT_SMALL \
		textscale			0.28 \
		textalign			ITEM_ALIGN_TOP_LEFT \
		decoration \
		text				labelArg \
		visible				visArg \
		exp					rect y ( Y + DY ) \
		exp					forecolor a ( 0.75 * ( A ) ) \
	} \
	itemDef \
	{ \
		rect				(X + 78) (Y + 4) BW 4 ah av \
		style				WINDOW_STYLE_SHADER \
		background			"white" \
		forecolor			1 1 1 0.12 \
		decoration \
		visible				visArg \
		exp					rect y ( Y + 4 + DY ) \
		exp					forecolor a ( 0.12 * ( A ) ) \
	} \
	itemDef \
	{ \
		rect				(X + 78) (Y + 4) BW 4 ah av \
		style				WINDOW_STYLE_SHADER \
		background			"white" \
		forecolor			0.85 0.85 0.85 0.9 \
		decoration \
		visible				visArg \
		exp					rect w ( SD_BAR_FRAC(P, col) * BW ) \
		exp					rect y ( Y + 4 + DY ) \
		exp					forecolor a ( 0.9 * ( A ) ) \
	} \
	itemDef \
	{ \
		rect				(X + 78) (Y + 4) BW 4 ah av \
		style				WINDOW_STYLE_SHADER \
		background			"white" \
		forecolor			0.3 0.85 0.3 1 \
		decoration \
		visible				visArg \
		exp					rect x ( X + 78 + SD_BAR_FRAC(P, col) * BW + ( ( deltaExpr ) < 0 ) * BW * ( deltaExpr ) / 100 ) \
		exp					rect w ( ( ( deltaExpr ) > 0 ) * min( BW * ( deltaExpr ) / 100 , ( 1 - SD_BAR_FRAC(P, col) ) * BW ) + ( ( deltaExpr ) < 0 ) * BW * ( 0 - ( deltaExpr ) ) / 100 ) \
		exp					rect y ( Y + 4 + DY ) \
		exp					forecolor r ( 0.25 + ( ( deltaExpr ) < 0 ) * 0.7 ) \
		exp					forecolor g ( 0.25 + ( ( deltaExpr ) > 0 ) * 0.65 ) \
		exp					forecolor b ( 0.25 ) \
		exp					forecolor a ( A ) \
	}

// --- A weapon card, as the variants are shown in Advanced Warfare --------------------------------------------------
// P prefix of its dvars, X/Y top left, W width (the height is (W - 20) / 2 + 222), A alpha, DY y offset (animations)
#define SD_CARD_RARITY(P)	dvarint( P + "_rarity" )
#define SD_CARD_IH(W)		((W - 20) / 2)
#define SD_CARD_H(W)		(SD_CARD_IH(W) + 222)

#define SD_CARD(P, X, Y, W, ah, av, A, DY, visArg) \
	itemDef \
	{ \
		rect				X Y W SD_CARD_H(W) ah av \
		style				WINDOW_STYLE_SHADER \
		background			"white" \
		forecolor			0.04 0.045 0.05 0.85 \
		decoration \
		visible				visArg \
		exp					rect y ( Y + DY ) \
		exp					forecolor a ( 0.85 * ( A ) ) \
	} \
	itemDef \
	{ \
		rect				X Y W 70 ah av \
		style				WINDOW_STYLE_SHADER \
		background			"white" \
		forecolor			1 1 1 0.16 \
		decoration \
		visible				visArg \
		exp					rect y ( Y + DY ) \
		exp					forecolor r ( SD_COLOR_R(P) ) \
		exp					forecolor g ( SD_COLOR_G(P) ) \
		exp					forecolor b ( SD_COLOR_B(P) ) \
		exp					forecolor a ( 0.16 * ( A ) ) \
	} \
	itemDef \
	{ \
		rect				X Y W 3 ah av \
		style				WINDOW_STYLE_SHADER \
		background			"white" \
		forecolor			1 1 1 1 \
		decoration \
		visible				visArg \
		exp					rect y ( Y + DY ) \
		exp					forecolor r ( SD_COLOR_R(P) ) \
		exp					forecolor g ( SD_COLOR_G(P) ) \
		exp					forecolor b ( SD_COLOR_B(P) ) \
		exp					forecolor a ( A ) \
	} \
	itemDef \
	{ \
		rect				X (Y + SD_CARD_H(W) - 2) W 2 ah av \
		style				WINDOW_STYLE_SHADER \
		background			"white" \
		forecolor			1 1 1 1 \
		decoration \
		visible				visArg \
		exp					rect y ( Y + SD_CARD_H(W) - 2 + DY ) \
		exp					forecolor r ( SD_COLOR_R(P) ) \
		exp					forecolor g ( SD_COLOR_G(P) ) \
		exp					forecolor b ( SD_COLOR_B(P) ) \
		exp					forecolor a ( 0.6 * ( A ) ) \
	} \
	itemDef \
	{ \
		rect				(X + 10) (Y + 9) (W - 20) 14 ah av \
		forecolor			1 1 1 1 \
		textfont			UI_FONT_SMALL \
		textscale			0.3 \
		textalign			ITEM_ALIGN_TOP_LEFT \
		decoration \
		visible				visArg \
		exp					text ( dvarstring( P + "_rname" ) ) \
		exp					rect y ( Y + 9 + DY ) \
		exp					forecolor r ( SD_COLOR_R(P) ) \
		exp					forecolor g ( SD_COLOR_G(P) ) \
		exp					forecolor b ( SD_COLOR_B(P) ) \
		exp					forecolor a ( A ) \
	} \
	itemDef \
	{ \
		rect				(X + 10) (Y + 9) (W - 20) 14 ah av \
		forecolor			0.45 1 0.45 1 \
		textfont			UI_FONT_SMALL \
		textscale			0.3 \
		textalign			ITEM_ALIGN_TOP_RIGHT \
		decoration \
		visible				visArg \
		exp					text ( dvarstring( P + "_equipped" ) ) \
		exp					rect y ( Y + 9 + DY ) \
		exp					forecolor a ( A ) \
	} \
	itemDef \
	{ \
		rect				(X + 10) (Y + 26) (W - 20) SD_CARD_IH(W) ah av \
		style				WINDOW_STYLE_SHADER \
		forecolor			1 1 1 1 \
		decoration \
		visible				visArg \
		exp					material ( dvarstring( P + "_img" ) ) \
		exp					rect y ( Y + 26 + DY ) \
		exp					forecolor a ( A ) \
	} \
	itemDef \
	{ \
		rect				(X + 10) (Y + 30 + SD_CARD_IH(W)) (W - 20) 22 ah av \
		forecolor			1 1 1 1 \
		textfont			9 \
		textscale			0.4 \
		textalign			ITEM_ALIGN_TOP_LEFT \
		decoration \
		visible				visArg \
		exp					text ( dvarstring( P + "_name" ) ) \
		exp					rect y ( Y + 30 + SD_CARD_IH(W) + DY ) \
		exp					forecolor r ( SD_COLOR_R(P) ) \
		exp					forecolor g ( SD_COLOR_G(P) ) \
		exp					forecolor b ( SD_COLOR_B(P) ) \
		exp					forecolor a ( A ) \
	} \
	itemDef \
	{ \
		rect				(X + 10) (Y + 52 + SD_CARD_IH(W)) (W - 20) 14 ah av \
		forecolor			1 1 1 0.7 \
		textfont			UI_FONT_SMALL \
		textscale			0.3 \
		textalign			ITEM_ALIGN_TOP_LEFT \
		decoration \
		visible				visArg \
		exp					text ( dvarstring( P + "_weapon" ) ) \
		exp					rect y ( Y + 52 + SD_CARD_IH(W) + DY ) \
		exp					forecolor a ( 0.7 * ( A ) ) \
	} \
	SD_RECT((X + 10), (Y + 70 + SD_CARD_IH(W)), (W - 20), 1, ah, av, 1, 1, 1, 0.15, visArg) \
	SD_BAR(P, (X + 10), (Y + 76 + SD_CARD_IH(W)), (W - 98), ah, av, "ACCURACY", "_s0", 0, A, DY, visArg) \
	SD_BAR(P, (X + 10), (Y + 92 + SD_CARD_IH(W)), (W - 98), ah, av, "DAMAGE", "_s1", dvarint( P + "_ddmg" ), A, DY, visArg) \
	SD_BAR(P, (X + 10), (Y + 108 + SD_CARD_IH(W)), (W - 98), ah, av, "RANGE", "_s2", dvarint( P + "_drng" ), A, DY, visArg) \
	SD_BAR(P, (X + 10), (Y + 124 + SD_CARD_IH(W)), (W - 98), ah, av, "FIRE RATE", "_s3", 0, A, DY, visArg) \
	SD_BAR(P, (X + 10), (Y + 140 + SD_CARD_IH(W)), (W - 98), ah, av, "MOBILITY", "_s4", dvarint( P + "_dmob" ), A, DY, visArg) \
	SD_RECT((X + 10), (Y + 158 + SD_CARD_IH(W)), (W - 20), 1, ah, av, 1, 1, 1, 0.15, visArg) \
	SD_CARD_MOD(P, "_mod0", X, (Y + 162 + SD_CARD_IH(W)), W, ah, av, A, DY, visArg) \
	SD_CARD_MOD(P, "_mod1", X, (Y + 176 + SD_CARD_IH(W)), W, ah, av, A, DY, visArg) \
	SD_CARD_MOD(P, "_mod2", X, (Y + 190 + SD_CARD_IH(W)), W, ah, av, A, DY, visArg) \
	SD_CARD_MOD(P, "_mod3", X, (Y + 204 + SD_CARD_IH(W)), W, ah, av, A, DY, visArg)

#define SD_CARD_MOD(P, suffix, X, Y, W, ah, av, A, DY, visArg) \
	itemDef \
	{ \
		rect				(X + 10) Y (W - 20) 14 ah av \
		forecolor			1 1 1 1 \
		textfont			UI_FONT_SMALL \
		textscale			0.3 \
		textalign			ITEM_ALIGN_TOP_LEFT \
		decoration \
		visible				visArg \
		exp					text ( dvarstring( P + suffix ) ) \
		exp					rect y ( Y + DY ) \
		exp					forecolor a ( A ) \
	}

// --- A Supply Drop crate, drawn: cx/cy its centre, S its scale, gold trim when advanced -----------------------------
#define SD_CRATE(cx, cy, S, ah, av, advancedExpr, A, DX, DY, LID, visArg) \
	itemDef \
	{ \
		rect				(cx - 50 * S) (cy - 26 * S) (100 * S) (60 * S) ah av \
		style				WINDOW_STYLE_SHADER \
		background			"white" \
		forecolor			0.22 0.24 0.19 1 \
		decoration \
		visible				visArg \
		exp					rect x ( cx - 50 * S + DX ) \
		exp					rect y ( cy - 26 * S + DY ) \
		exp					forecolor a ( A ) \
	} \
	itemDef \
	{ \
		rect				(cx - 50 * S) (cy - 6 * S) (100 * S) (6 * S) ah av \
		style				WINDOW_STYLE_SHADER \
		background			"white" \
		forecolor			0.14 0.15 0.12 1 \
		decoration \
		visible				visArg \
		exp					rect x ( cx - 50 * S + DX ) \
		exp					rect y ( cy - 6 * S + DY ) \
		exp					forecolor a ( A ) \
	} \
	itemDef \
	{ \
		rect				(cx - 10 * S) (cy - 26 * S) (20 * S) (60 * S) ah av \
		style				WINDOW_STYLE_SHADER \
		background			"white" \
		forecolor			0.85 0.85 0.8 1 \
		decoration \
		visible				visArg \
		exp					rect x ( cx - 10 * S + DX ) \
		exp					rect y ( cy - 26 * S + DY ) \
		exp					forecolor r ( 0.75 + ( advancedExpr ) * 0.25 ) \
		exp					forecolor g ( 0.75 - ( advancedExpr ) * 0.1 ) \
		exp					forecolor b ( 0.7 - ( advancedExpr ) * 0.5 ) \
		exp					forecolor a ( A ) \
	} \
	itemDef \
	{ \
		rect				(cx - 54 * S) (cy - 40 * S) (108 * S) (16 * S) ah av \
		style				WINDOW_STYLE_SHADER \
		background			"white" \
		forecolor			0.3 0.32 0.26 1 \
		decoration \
		visible				visArg \
		exp					rect x ( cx - 54 * S + DX ) \
		exp					rect y ( cy - 40 * S + DY - ( LID ) ) \
		exp					forecolor a ( A ) \
	} \
	itemDef \
	{ \
		rect				(cx - 54 * S) (cy - 26 * S) (108 * S) (2 * S) ah av \
		style				WINDOW_STYLE_SHADER \
		background			"white" \
		forecolor			1 0.75 0.2 1 \
		decoration \
		visible				visArg \
		exp					rect x ( cx - 54 * S + DX ) \
		exp					rect y ( cy - 26 * S + DY - ( LID ) ) \
		exp					forecolor r ( 0.85 + ( advancedExpr ) * 0.15 ) \
		exp					forecolor g ( 0.85 - ( advancedExpr ) * 0.1 ) \
		exp					forecolor b ( 0.8 - ( advancedExpr ) * 0.6 ) \
		exp					forecolor a ( A ) \
	}

// --- A button in the middle of the screen (popups, the reveal) -----------------------------------------------
#define SD_CENTER_BUTTON(buttonName, y, textArg, actionArg, visArg) \
	itemDef \
	{ \
		name				buttonName \
		rect				(0 - 110) y 220 22 HORIZONTAL_ALIGN_CENTER VERTICAL_ALIGN_CENTER \
		forecolor			1 1 1 1 \
		style				WINDOW_STYLE_FILLED \
		background			"menu_button_selection_bar" \
		group				"sd_center_buttons" \
		type				ITEM_TYPE_BUTTON \
		textAlign			ITEM_ALIGN_MIDDLE_CENTER \
		textScale			0.375 \
		textFont			UI_FONT_NORMAL \
		exp					text ( textArg ) \
		visible				visArg \
		mouseEnter \
		{ \
			play			"mouse_over"; \
		} \
		onFocus \
		{ \
			setItemColor	self backColor 0 0 0 1; \
		} \
		leaveFocus \
		{ \
			setItemColor	self backColor 0 0 0 0; \
		} \
		action \
		{ \
			play			"mouse_click"; \
			actionArg \
		} \
	}

