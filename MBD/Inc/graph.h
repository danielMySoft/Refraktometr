// -----------------------------------------------
// definicje rozkazów dla wyœwietlacza
//				OLED
// -----------------------------------------------
// nowe rozkazy dla OLED

// Set column address
#define		OLED_COLUMNS			0x15			// dwa bajty LOW I HIGH

// Set row address
#define		OLED_ROWS				0x75			// dwa bajty LOW I HIGH

// Set contrast
#define		OLED_CONTRAST			0x81			// jeden bajt CONTRAST

// Set current range
#define		OLED_CURRENT_QUARTER	0x84
#define		OLED_CURRENT_HALF		0x85
#define		OLED_CURRENT_FULL		0x86

// Set re-map
#define		OLED_RE_MAP				0xA0			// bajt parametrów

//0-0	disable column adress re-map (POR)
//0-1	enable column adress re-map
//1-0	disable nibble re-map (POR)
//1-1	enable nibble re-map
//2-0	horizontal adsress increment (POR)
//2-1	vertical adsress increment
//3-0	
//3-1	
//4-0	disable COM re-map (POR)
//4-1	enable COM re-map
//5-0	
//5-1	
//6-0	disable COM split odd/even (POR)
//6-1	enable COM split odd/even
//7-0	
//7-1	


// Set display start line
#define		OLED_START_LINE			0xA1			// bajt start line

// Set display offset
#define		OLED_OFFSET				0xA2			// bajt offset

// Set display mode
#define		OLED_MODE_NORMAL		0xA4
#define		OLED_MODE_ED_ON			0xA5			// GS = 16
#define		OLED_MODE_ED_OFF		0xA6			// BLANK
#define		OLED_MODE_INVERSE		0xA7

// Set multiplekser ratio
#define		OLED_MULTIPLEKSER_RATIO	0xA8			// ratio

// Set master configuration
#define		OLED_MASTER_CONF		0xAD			// bajt configuration

//0-0	disable DC-DC converter
//0-1	enable DC-DC converter (POR)
//1-0	disable internal VCOMH
//1-1	enable internal VCOMH (POR)


// Set display OFF
#define		OLED_OFF				0xAE			// (POR)
// Set display ON
#define		OLED_ON					0xAF

// Set pre-charge compensation enable
#define		OLED_PRE_COMP			0xB0			// bajt 0-(POR)
													// 28H enable

// Set pre-charge compensation level
#define		OLED_PRE_COMP_LEVEL		0xB4			// bajt 0-(POR)
													//      3- recomended

// Set segment LOW VOLTAGE
#define		OLED_SEG_LOW_VOLTAGE	0xBF			// bajt 1110-(POR)

// Set VCOMH VOLTAGE
#define		OLED_SEG_VCOMH			0xBE			// bajt 010001-(POR)

// Set pre-charge voltage
#define		OLED_PRE_VOLTAGE		0xBC			// bajt 00011000-(POR)

// Set phase lenght
#define		OLED_PHASE_LENGHT		0xB1			// dwa bajty A[3-0], A[7-4]

// Set row period
#define		OLED_ROW_PERIOD			0xB2			// bajt

// Set display clock
#define		OLED_DISPLAY_CLOCK		0xB3			// dwa bajty A[3-0], A[7-4]

// Set grey scale table
#define		OLED_GREY_SCALE_TABLE	0xB8			// osiem bajtów

// Set biasing current
#define		OLED_BIASING_CURRENT	0xCF			// bajt F0 HIGH  70 LOW

// NOP
#define		OLED_NOP				0xE3

// READ STATUS	D6 = 0 ON D6=1 OFF

