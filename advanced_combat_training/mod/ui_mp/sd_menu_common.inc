// The common head of the full screen Supply Drops menus (sd_armory, sd_variants)
#undef CHOICE_GROUP
#define CHOICE_GROUP				MENU_NAME

#undef TEXTSIZE_TITLE
#define TEXTSIZE_TITLE				FONTSCALE_BIG

#define CHOICE_Y(i)					(88 + (i * 20))
#define CHOICE_ORIGIN(i)			-64 CHOICE_Y(i)
#define CHOICE_RECT(i)				CHOICE_ORIGIN(i) 336 20 HORIZONTAL_ALIGN_LEFT VERTICAL_ALIGN_TOP

#define HR_ORIGIN(i)				0 (CHOICE_Y(i) - 0.5)
#define HR_RECT(i)					HR_ORIGIN(i) 216 1 HORIZONTAL_ALIGN_LEFT VERTICAL_ALIGN_TOP

// The panel on the right
#define PX							(0 - 300)
#define PW							290

// A small text at the right end of a row of the left column
#define SD_ROW_TAG(i, textExpr, r, g, b, visArg) \
	SD_TEXT(218, (CHOICE_Y(i) + 4), 70, 14, HORIZONTAL_ALIGN_LEFT, VERTICAL_ALIGN_TOP, UI_FONT_SMALL, 0.28, ITEM_ALIGN_TOP_LEFT, r, g, b, 1, textExpr, visArg)

// What just happened, at the bottom of the screen
#define SD_MESSAGE \
	SD_TEXT((0 - 300), 392, 600, 16, HORIZONTAL_ALIGN_CENTER, VERTICAL_ALIGN_TOP, UI_FONT_SMALL, 0.32, ITEM_ALIGN_TOP_CENTER, 1, 1, 1, 1, dvarstring( "sd_msg" ), 1)
