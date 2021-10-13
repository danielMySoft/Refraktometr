/*
 * Constants.h
 *
 *  Created on: 10 ���. 2020 �.
 *      Author: Admin
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

//MENU DEFINITIONS
#define MAIN_MENU_POSITION frame.position[0]

#define MAIN_MENU_MEASUREMENT_TYPE 0
#define MAIN_MENU_MEASUREMENT_UNIT 1
#define MAIN_MENU_THERMAL_COMPENSATION 2
#define MAIN_MENU_SETTINGS 3
#define MAIN_MENU_ARCHIVE 4
#define MAIN_MENU_EXIT 5

//SETTINGS SUBMENUS
#define SETTINGS_SUBMENU_SINGLE_MEASUREMENT_TIME 0
#define SETTINGS_SUBMENU_DATE 1
#define SETTINGS_SUBMENU_TIME 2
#define SETTINGS_SUBMENU_LANGUAGE 3

//DATE / TIME SUBMENUS
#define DATE_DAY_INPUT_SCREEN 2
#define DATE_MONTH_INPUT_SCREEN 1
#define DATE_YEAR_INPUT_SCREEN 0
#define TIME_HOUR_INPUT_SCREEN 0
#define TIME_MINUTE_INPUT_SCREEN 1
#define TIME_SECOND_INPUT_SCREEN 2

//ARCHIVE SUBMENUS
#define ARCHIVE_SUBMENU_BROWSE 0
#define ARCHIVE_SUBMENU_STORAGE_MODE 1
#define ARCHIVE_SUBMENU_ERASE 2

#define ARCHIVE_STORAGE_MODE_PROHIBITED 0
#define ARCHIVE_STORAGE_MODE_BY_KEYBOARD 1

//MAIN SCREEN
const char Main_Screen[][2][18] = {
		{"KOMP.ZA\x84"," COMP.ON"},
		{"BRAK","NO"},
		{"PROBK","SAMPLE"},
		{"KOMP.WY\x84","COMP.OFF"},
		{"CZEKAJ  "," WAIT   "},
		{"SEK.","SEC."},
		{"NACISNIJ", "PRESS"}
};

// MAIN MENU
const char Main_Menu[][2][20] = {
		{"MENU G\x84\x86WNE","MAIN MENU"},
		{"RODZAJ POMIARU","MEASUREMENT TYPE"},
		{"JEDNOSTKA POMIARU","MEASUREMENT UNIT"},
		{"KOMPENSACJA TERM.","THERMAL COMP."},
		{"USTAWIENIA","SETTINGS"},
		{"ARCHIWUM","ARCHIVE"},
		{"KONIEC EDYCJI","EXIT"},
		{"COFNIJ","BACK"}

};

const char Measurement_Type[][2][12] = {
		{"POJEDYNCZY","SINGLE"},
		{"WIELOKROTNY","CONTINUOUS"}
};

const char Measurement_Unit[][2][17] = {
		{"WSP. ZA\x84\x41MANIA","REFRACTIVE INDEX"}
};

const char Thermal_Compensation[][2][18] = {
		{"KOMPENSACJA TEMP.","THERMAL COMP."},
		{"WY\x84\x81\x43ZONA","OFF"},
		{"ZA\x84\x81\x43ZONA","ON"}
};

const char Settings[][2][20] = {
		{"USTAWIENIA","SETTINGS"},
		{"CZAS POMIARU POJED.","SINGLE MEASURE TIME"},
		{"DATA","DATE"},
		{"CZAS","TIME"},
		{"J\x83ZYK","LANGUAGE"},
		{"USTAW DAT\x83", "SET DATE"},
		{"USTAW CZAS", "SET TIME"},
		{"DZIE\x85", "DAY"},
		{"MIESI\x81\x43", "MONTH"},
		{"ROK", "YEAR"},
		{"GODZINA", "HOURS"},
		{"MINUTA", "MINUTES"},
		{"SEKUNDA", "SECONDS"}
};

const char Archive[][2][14] = {
		{"PRZEGL\x81\x44\x41NIE","BROWSE"},
		{"TRYB ZAPISU","STORAGE MODE"},
		{"KASOWANIE","ERASE ARCHIVE"},
		{"BRAK","NO"},
		{"ZABRONIONY","PROHIBITED"},
		{"Z KLAWIATURY","BY KEYBOARD"},
		{"KASOWANIE","ERASING"},
		{"ARCHIWUM","ARCHIVE"},
		{"SKASOWANE","ERASED"},
		{"NR","No"},
		{"Z", "OF"}
};

const char Languages[][8] = {
		{"POLSKI"},{"ENGLISH"}
};

#endif /* CONSTANTS_H_ */