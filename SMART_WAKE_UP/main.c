#include "sh1106.h"
#include "tim.h"
#include "buttons.h"
#include "ds3231.h"
#include "gpio.h"

const char *days[] = {"NA", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
const char *months[] = {"NA", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

static int8_t DS3231_Second = 0;
static int8_t DS3231_Minute = 0;
static int8_t DS3231_Hour = 0;
static int8_t DS3231_DayWeek = 1;
static int8_t DS3231_DayMonth = 1;
static int8_t DS3231_Month = 1;
static int8_t DS3231_Year = 0;
static int8_t DS3231_Century = 0;

static uint8_t UpdateToDisplay = 0;
static uint8_t UpdateToSetting = 0;

int move = 0;
static uint8_t state = 0;

static void MAIN_DisplayDate(void);
static void MAIN_Settings(void);
static void MAIN_Initialization(void);
//static void keyboard(void);

int main(void) 
{
	SH1106_Init();
	BUTTONS_Init();
	DS3231_Init();
	GPIO_PinMode(GPIOB, 7, OUTPUT);
	//GPIO_PinMode(GPIOB, 0, OUTPUT);
	GPIO_PinMode(GPIOB, 14, OUTPUT);
	SH1106_ClearBuffer();
	SH1106_SendBuffer();
	
	/*uint8_t dataT[7] = {DS3231_DEC_BCD(0), DS3231_DEC_BCD(35), DS3231_DEC_BCD(20), DS3231_DEC_BCD(06), DS3231_DEC_BCD(03), DS3231_DEC_BCD(02), DS3231_DEC_BCD(24)};
	DS3231_WriteMemory(0x68, 0x00, dataT, 7);*/
	
	while (1) 
	{
		SH1106_ClearBuffer();
		BUTTONS_KeyState();
		GPIO_DigitalWrite(GPIOB, 7, state);	
		//GPIO_DigitalWrite(GPIOB, 0, !state);
		GPIO_DigitalWrite(GPIOB, 14, !state);	
		
		switch (BUTTON_Switch)
		{
			case 0:
				MAIN_DisplayDate();
				break;
			case 1:
				MAIN_Settings();
				break;
		}
		state ^= 1;
		SH1106_SendBuffer();
	}
	
}

static void MAIN_DisplayDate(void)
{
	UpdateToSetting = 1;
	
	if (UpdateToDisplay)
	{
		uint8_t dataS[7] = {DS3231_DEC_BCD(DS3231_Second), DS3231_DEC_BCD(DS3231_Minute), DS3231_DEC_BCD(DS3231_Hour), DS3231_DEC_BCD(DS3231_DayWeek), DS3231_DEC_BCD(DS3231_DayMonth), DS3231_DEC_BCD(DS3231_Month), DS3231_DEC_BCD(DS3231_Year)};
		DS3231_WriteMemory(0x68, 0x00, dataS, 7);
		
		BUTTON_TopState = 0;
		BUTTON_BottomState = 0;
		BUTTON_RightState = 0;
		BUTTON_LeftState = 0;
		
		move = 0;
		UpdateToDisplay = 0;
	}
	
	uint8_t data[7] = {0};
	DS3231_Read(0x68,0x0,data,7);
	DS3231_Second = DS3231_BCD_DEC(data[0] & 0x7F);
	DS3231_Minute = DS3231_BCD_DEC(data[1]);
	DS3231_Hour = DS3231_BCD_DEC(data[2] & 0x3F);
	DS3231_DayWeek = DS3231_BCD_DEC(data[3]);
	DS3231_DayMonth = DS3231_BCD_DEC(data[4] );
	DS3231_Month = DS3231_BCD_DEC(data[5]);
	DS3231_Year = DS3231_BCD_DEC(data[6]);
	DS3231_Century = DS3231_BCD_DEC(data[5] & 0x80);
	
	//keyboard();
	SH1106_FontPrint(1, 0, 0, Arial12x12, "Hello Mr TESSON");
	SH1106_FontPrint(1, 7, 13, Arial28x28, "%02d:%02d:%02d", DS3231_Hour, DS3231_Minute, DS3231_Second);
	SH1106_FontPrint(1, 0, 39, Arial12x12, "%s,", days[DS3231_DayWeek]);
	SH1106_FontPrint(1, 0, 52, Arial12x12, "%s %d, 2%d%02d", months[DS3231_Month], DS3231_DayMonth, DS3231_Century, DS3231_Year);
	SH1106_DrawLine(1, 0, 37, 128, 37);
	SH1106_DrawLine(1, 0, 12, 128, 12);
}

static void handling(int8_t* data, const char* title, int max, int min)
{
	if (BUTTON_TopState) 
	{
		(*data)++;
		BUTTON_TopState = 0;
	}
	if (BUTTON_BottomState) 
	{
		(*data)--;
		BUTTON_BottomState = 0;
	}

	if (*data > max) *data = min;
	if (*data < min) *data = max;

	SH1106_FontPrint(1, 0, 13, Arial12x12, "Setting %s : %d", title, *data);
}

static void handlingDay()
{
	uint8_t isLeapYear = (DS3231_Year %4 == 0 && DS3231_Year %100 != 0) || (DS3231_Year %400 == 0);

	if (BUTTON_TopState) 
	{
		DS3231_DayMonth++;
		BUTTON_TopState = 0;
	}
	if (BUTTON_BottomState) 
	{
		DS3231_DayMonth--;
		BUTTON_BottomState = 0;
	}

	if ((DS3231_Month == 4 || DS3231_Month == 6 || DS3231_Month == 9 || DS3231_Month == 11) && (DS3231_DayMonth > 30)) DS3231_DayMonth=0;
	if ((DS3231_Month == 4 || DS3231_Month == 6 || DS3231_Month == 9 || DS3231_Month == 11) && (DS3231_DayMonth < 0)) DS3231_DayMonth=30;
	if ((DS3231_Month == 1 || DS3231_Month == 3 || DS3231_Month == 5 || DS3231_Month == 7 || DS3231_Month == 9 || DS3231_Month == 11) && (DS3231_DayMonth > 31)) DS3231_DayMonth=0;
	if ((DS3231_Month == 1 || DS3231_Month == 3 || DS3231_Month == 5 || DS3231_Month == 7 || DS3231_Month == 9 || DS3231_Month == 11) && (DS3231_DayMonth < 0)) DS3231_DayMonth=31;
	if ((DS3231_Month == 2) && (DS3231_DayMonth > 28)) DS3231_DayMonth=0;
	if ((DS3231_Month == 2) && (DS3231_DayMonth < 0)) DS3231_DayMonth=28;
	if (DS3231_Month == 2 && isLeapYear && DS3231_DayMonth > 29) DS3231_DayMonth = 0;
	if (DS3231_Month == 2 && !isLeapYear && DS3231_DayMonth > 28) DS3231_DayMonth = 0;
	if (DS3231_Month == 2 && isLeapYear && DS3231_DayMonth < 0) DS3231_DayMonth = 29;
	if (DS3231_Month == 2 && !isLeapYear && DS3231_DayMonth < 0) DS3231_DayMonth = 28;

	SH1106_FontPrint(1, 0, 13, Arial12x12, "Setting day : %d", DS3231_DayMonth);
}

static void handlingMonth()
{
	uint8_t isLeapYear = (DS3231_Year %4 == 0 && DS3231_Year %100 != 0) || (DS3231_Year %400 == 0);

	if (BUTTON_TopState) 
	{
		DS3231_Month++;
		BUTTON_TopState = 0;
	}
	if (BUTTON_BottomState) 
	{
		DS3231_Month--;
		BUTTON_BottomState = 0;
	}

	if (DS3231_Month>12) DS3231_Month=1;
	if (DS3231_Month<1) DS3231_Month=12;

	if ((DS3231_Month == 4 || DS3231_Month == 6 || DS3231_Month == 9 || DS3231_Month == 11) && (DS3231_DayMonth > 30)) DS3231_DayMonth=30;  //Cas ou dans 1 mois, il n'y a que 30 jours

	if (DS3231_Month == 2 && isLeapYear && DS3231_DayMonth > 29) DS3231_DayMonth = 29;                                 //Cas de Fevrier dans les annees bissextiles (29 jours)
	if (DS3231_Month == 2 && !isLeapYear && DS3231_DayMonth > 28) DS3231_DayMonth = 28;                                //Cas de Fevrier hors annees bissextiles (28 jours)

	SH1106_FontPrint(1, 0, 13, Arial12x12, "Setting month : %d", DS3231_Month);
}

static void handlingYear()
{
	uint8_t isLeapYear = (DS3231_Year %4 == 0 && DS3231_Year %100 != 0) || (DS3231_Year %400 == 0);

	if (BUTTON_TopState) 
	{
		DS3231_Year++;
		BUTTON_TopState = 0;
	}
	if (BUTTON_BottomState) 
	{
		DS3231_Year--;
		BUTTON_BottomState = 0;
	}

	if (DS3231_Year>99) DS3231_Year=0;
	if (DS3231_Year<0) DS3231_Year=99;

	if (DS3231_Month == 2 && isLeapYear && DS3231_DayMonth > 29) DS3231_DayMonth = 29;           // Cas de Fevrier dans les annees bissextiles (29 jours)
	if (DS3231_Month == 2 && !isLeapYear && DS3231_DayMonth > 28) DS3231_DayMonth = 28;          // Cas de Fevrier hors annees bissextiles (28 jours)

	SH1106_FontPrint(1, 0, 13, Arial12x12, "Setting year : %d", DS3231_Year);
}

static void MAIN_Settings(void)
{
	UpdateToDisplay = 1;
	
	if (UpdateToSetting)
	{
		BUTTON_TopState = 0;
		BUTTON_BottomState = 0;
		BUTTON_RightState = 0;
		BUTTON_LeftState = 0;
		
		UpdateToSetting = 0;
	}

	if (BUTTON_RightState) 
	{
		move++;
		BUTTON_RightState = 0;
	}
	if (BUTTON_LeftState) 
	{
		move--;
		BUTTON_LeftState = 0;
	}
  
	if (move > 6) move = 0;
	if (move < 0) move = 6;
  
	switch (move)
	{
		case 0:
			handling(&DS3231_Second, "sec", 59, 0);
			break;
		case 1:
			handling(&DS3231_Minute, "min", 59, 0);
			break;
		case 2:
			handling(&DS3231_Hour, "hour", 23, 0);
			break;
		case 3:
			handling(&DS3231_DayWeek, "dayW", 7, 1);
			break;
		case 4:
			handlingDay();
			break;
		case 5:
			handlingMonth();
			break;
		case 6:
			handlingYear();
			break;
	}
}

/*static void keyboard(void)
{
    // Incrémente move si le bouton TopState est activé
	if (BUTTON_TopState) 
	{
		move++;
		BUTTON_TopState = 0;
	}
	
    // Décrémente move si le bouton BottomState est activé
	if (BUTTON_BottomState) 
	{
		move--;
		BUTTON_BottomState = 0;
	}
	
    // Assure que move reste dans la plage 0-25
	if (move > 25) 
        move = 0;
	if (move < 0) 
        move = 25;
	
    // Boucle à travers les trois groupes de lettres
	for(int group = 0; group < 3; group++)
	{
        // Calcul de la plage d'indices de lettres pour le groupe actuel
		int start_index = group * 10;
		int end_index = start_index + 10;
		
        // Parcoure les lettres dans la plage d'indices actuelle
		for(int i = start_index; i < end_index; i++)
		{
            // Calcul des informations sur la lettre actuelle
			uint16_t NumLetter = 65 + i;
			uint8_t dataSize = Arial12x12[0];
			uint16_t index_LetterSize = 4 + NumLetter * dataSize;
			uint8_t letterSize = Arial12x12[index_LetterSize];
			
            // Calcul de la position verticale de la lettre sur l'écran
			uint8_t letterCoor = (i - start_index) * 12 + (12 - letterSize) / 2;
			
            // Vérifie si la lettre est sélectionnée
			if (i == move) 
			{
                // Dessine un rectangle rempli et affiche la lettre en surbrillance
				SH1106_DrawFilledRectangle(1, (i - start_index) * 12, group * 12, 12, 12);
				SH1106_DrawCharacter(0, letterCoor, group * 12 + 2, Arial12x12, NumLetter);
			}
			else
			{
                // Dessine un rectangle vide et affiche la lettre normalement
				SH1106_DrawRectangle(1, (i - start_index) * 12, group * 12, 12, 12);
				SH1106_DrawCharacter(1, letterCoor, group * 12 + 2, Arial12x12, NumLetter);
			}
		}
	}
}	*/

	
	
