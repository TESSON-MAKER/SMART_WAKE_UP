#include "sh1106.h"
#include "tim.h"
#include "buttons.h"
#include "ds3231.h"
#include "gpio.h"

const char *days[] = {"NA", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
const char *months[] = {"NA", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

/*const int numberCols = 9;
const int numberLetter1 = 27;
const int numberLetter2 = 27;
const int numberLetter3 = 15;

uint8_t letters1[numberLetter1] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', ' '};
uint8_t letters2[numberLetter2] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', ' '};
uint8_t letters3[numberLetter3] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.', '/', '&', '%', '!'};

uint8_t keyBoard = 0;*/

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
	GPIO_PinMode(GPIOB, 14, OUTPUT);
	SH1106_ClearBuffer();
	SH1106_SendBuffer();
	
	while (1) 
	{
		SH1106_ClearBuffer();
		BUTTONS_KeyState();
		GPIO_DigitalWrite(GPIOB, 7, state);	
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
	DS3231_DayMonth = DS3231_BCD_DEC(data[4]);
	DS3231_Month = DS3231_BCD_DEC(data[5]);
	DS3231_Year = DS3231_BCD_DEC(data[6]);
	DS3231_Century = DS3231_BCD_DEC(data[5] & 0x80);
	
	//keyboard();
	SH1106_DrawStr(1, 0, 0, &Arial12x12, "Hello Mr TESSON");
	SH1106_FontPrint(1, 7, 13, &Arial28x28, "%02d:%02d:%02d", DS3231_Hour, DS3231_Minute, DS3231_Second);
	SH1106_FontPrint(1, 0, 39, &Arial12x12, "%s,", days[DS3231_DayWeek]);
	SH1106_FontPrint(1, 0, 52, &Arial12x12, "%s %d, 2%d%02d", months[DS3231_Month], DS3231_DayMonth, DS3231_Century, DS3231_Year);
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

	SH1106_FontPrint(1, 0, 13, &Arial12x12, "Setting %s : %d", title, *data);
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

	SH1106_FontPrint(1, 0, 13, &Arial12x12, "Setting day : %d", DS3231_DayMonth);
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

	SH1106_FontPrint(1, 0, 13, &Arial12x12, "Setting month : %d", DS3231_Month);
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

	SH1106_FontPrint(1, 0, 13, &Arial12x12, "Setting year : %d", DS3231_Year);
}

static void MAIN_Settings(void)
{
	//keyboard();
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

char string[50]; // Array to store the entered characters
int number = 0; // Counter for the number of entered characters

/*static void keyboard(void)
{

	const int side = 128 / numberCols;
	const int height = 12;

	uint8_t row = 0;
	uint8_t col = 0;

	if (BUTTON_LeftState) 
	{
		move++;
		BUTTON_LeftState = 0;
	}
	else if (BUTTON_RightState) 
	{
		move--;
		BUTTON_RightState = 0;
	}
	else if (BUTTON_TopState)
	{
		keyBoard ++;
		BUTTON_TopState = 0;
	}

	if (keyBoard>2) keyBoard=0;
	if (keyBoard<0) keyBoard=2;

	int maxMove = 0;
	switch(keyBoard)
	{
		case 0:
			maxMove = numberLetter1 - 1;
			break;
		case 1:
			maxMove = numberLetter2 - 1;
			break;
		case 2:
			maxMove = numberLetter3 - 1;
			break;
	}

	if (move > maxMove) move = 0;
	if (move < 0) move = maxMove;

	for (int i = 0; i <= maxMove; i++)
	{       
		if ((i % numberCols == 0) && (i != 0)) row++;

		col = i % numberCols;
		int coorX = side * col;
		int coorY = 14 + height * row;
		int select = row * numberCols + col;

		// Determining the character to display
		char character;

		switch(keyBoard)
		{
			case 0:
				character = letters1[i];
				break;
			case 1:
				character = letters2[i];
				break;
			case 2:
				character = letters3[i];
				break;
		}

		uint8_t letterNumber = character - Arial12x12.asciiOffset;
		uint16_t index_letterSize = letterNumber * Arial12x12.datasize;
		uint8_t letterSize = Arial12x12.data[index_letterSize];
		uint8_t posLetter = (side - letterSize)/2;

		// Displaying the character
		if (select == move) 
		{
			SH1106_DrawFilledRectangle(1, coorX, coorY, side, height);
			SH1106_DrawCharacter(0, coorX + posLetter, coorY + 2, &Arial12x12, character);

			if (BUTTON_BottomState && number < 49) // Checking the array limit
			{
				string[number++] = character; // Adding the character to the array
				BUTTON_BottomState = 0;
			}
		}
		else
		{
			SH1106_DrawRectangle(1, coorX, coorY, side, height);
			SH1106_DrawCharacter(1, coorX + posLetter, coorY + 2, &Arial12x12, character);
		}
	}

	// Update the display once after adding all characters
	SH1106_DrawStr(1, 2, 2, &Arial12x12, string);
	SH1106_DrawRectangle(1, 0, 0, 127, 13);
}*/






