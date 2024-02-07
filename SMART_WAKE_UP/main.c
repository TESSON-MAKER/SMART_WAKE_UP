#include "sh1106.h"
#include "tim.h"
#include "st7920_fonts.h"
#include "buttons.h"
#include "ds3231.h"
#include "gpio.h"

const char *days[] = {"NA", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
const char *months[] = {"NA", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};



static uint8_t DS3231_Second = 0;
static uint8_t DS3231_Minute = 0;
static uint8_t DS3231_Hour = 0;
static uint8_t DS3231_DayWeek = 1;
static uint8_t DS3231_DayMonth = 1;
static uint8_t DS3231_Month = 1;
static uint8_t DS3231_Year = 0;
static uint8_t DS3231_Century = 0;

uint8_t update = 0;
int move = 0;
static uint8_t state = 0;

static void MAIN_DisplayDate(void);
static void MAIN_Settings(void);
static void MAIN_Initialization(void);

int main(void) 
{
	SH1106_Init();
	BUTTONS_Init();
	DS3231_Init();
	GPIO_PinMode(GPIOB, 7, OUTPUT);
	SH1106_ClearBuffer();
	SH1106_SendBuffer();
	
	/*uint8_t dataT[7] = {DS3231_DEC_BCD(0), DS3231_DEC_BCD(35), DS3231_DEC_BCD(20), DS3231_DEC_BCD(06), DS3231_DEC_BCD(03), DS3231_DEC_BCD(02), DS3231_DEC_BCD(24)};
	DS3231_WriteMemory(0x68, 0x00, dataT, 7);*/
	
	while (1) 
	{
		SH1106_ClearBuffer();
		BUTTONS_KeyState();
		GPIO_DigitalWrite(GPIOB, 7, state);	
		switch (BUTTON_Switch)
		{
			case 0:
				MAIN_Initialization();
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
	
	SH1106_FontPrint(1, 0, 0, Arial12x12, "Hello Mr TESSON");
	SH1106_FontPrint(1, 7, 13, Arial28x28, "%02d:%02d:%02d", DS3231_Hour, DS3231_Minute, DS3231_Second);
	SH1106_FontPrint(1, 0, 39, Arial12x12, "%s,", days[DS3231_DayWeek]);
	SH1106_FontPrint(1, 0, 52, Arial12x12, "%s %d, 2%d%02d", months[DS3231_Month], DS3231_DayMonth, DS3231_Century, DS3231_Year);
	SH1106_DrawLine(1, 0, 37, 128, 37);
	SH1106_DrawLine(1, 0, 12, 128, 12);
}

static void handling(uint8_t* data, const char* title, int max, int min)
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
	uint8_t isLeapYear = (DS3231_Year % 4 == 0 && DS3231_Year % 100 != 0) || (DS3231_Year % 400 == 0);

	if (BUTTON_TopState) 
	{
		DS3231_DayMonth++;
		BUTTON_TopState = 0;
	}
	else if (BUTTON_BottomState) 
	{
		DS3231_DayMonth--;
		BUTTON_BottomState = 0;
	}

	const uint8_t daysInMonth[] = {31, isLeapYear ? 29 : 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	
	if (DS3231_DayMonth > daysInMonth[DS3231_Month - 1]) DS3231_DayMonth = 0;
	else if (DS3231_DayMonth < 1) DS3231_DayMonth = daysInMonth[DS3231_Month - 1];

	SH1106_FontPrint(1, 0, 13, Arial12x12, "Setting day : %d", DS3231_DayMonth);
}

static void handlingMonth()
{
	uint8_t isLeapYear = (DS3231_Year % 4 == 0 && DS3231_Year % 100 != 0) || (DS3231_Year % 400 == 0);

	if (BUTTON_TopState) 
	{
		DS3231_Month++;
		BUTTON_TopState = 0;
	}
	else if (BUTTON_BottomState) 
	{
		DS3231_Month--;
		BUTTON_BottomState = 0;
	}

	if (DS3231_Month > 12) DS3231_Month = 1;
	else if (DS3231_Month < 1) DS3231_Month = 12;

	const uint8_t daysInMonth[] = {31, isLeapYear ? 29 : 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	if (DS3231_DayMonth > daysInMonth[DS3231_Month - 1]) DS3231_DayMonth = daysInMonth[DS3231_Month - 1];

	SH1106_FontPrint(1, 0, 13, Arial12x12, "Setting month : %d", DS3231_Month);
}

static void handlingYear()
{
	uint8_t isLeapYear = (DS3231_Year % 4 == 0 && DS3231_Year % 100 != 0) || (DS3231_Year % 400 == 0);

	if (BUTTON_TopState) 
	{
		DS3231_Year++;
		BUTTON_TopState = 0;
	}
	else if (BUTTON_BottomState) 
	{
		DS3231_Year--;
		BUTTON_BottomState = 0;
	}

	if (DS3231_Year > 99) DS3231_Year = 0;
	else if (DS3231_Year < 0) DS3231_Year = 99;

	const uint8_t daysInMonth[] = {31, isLeapYear ? 29 : 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	if (DS3231_DayMonth > daysInMonth[DS3231_Month - 1]) DS3231_DayMonth = daysInMonth[DS3231_Month - 1];

	SH1106_FontPrint(1, 0, 13, Arial12x12, "Setting year : %d", DS3231_Year);
}

static void MAIN_Settings(void)
{
	update = 1;

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

static void MAIN_Initialization(void)
{
	move = 0;

	if (update)
	{
		uint8_t dataS[7] = {DS3231_DEC_BCD(DS3231_Second), DS3231_DEC_BCD(DS3231_Minute), DS3231_DEC_BCD(DS3231_Hour), DS3231_DEC_BCD(DS3231_DayWeek), DS3231_DEC_BCD(DS3231_DayMonth), DS3231_DEC_BCD(DS3231_Month), DS3231_DEC_BCD(DS3231_Year)};
		DS3231_WriteMemory(0x68, 0x00, dataS, 7);
		update = 0;
	}
}
