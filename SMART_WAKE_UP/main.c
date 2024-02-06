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


static uint8_t state = 0;

static void MAIN_DisplayDate(void);
static void MAIN_Settings(void);

int main(void) 
{
	SH1106_Init();
	BUTTONS_Init();
	DS3231_Init();
	GPIO_PinMode(GPIOB, 7, OUTPUT);
	SH1106_ClearBuffer();
	SH1106_SendBuffer();
	
	while (1) 
	{
		BUTTONS_KeyState();
		GPIO_DigitalWrite(GPIOB, 7, state);	
		switch (BUTTON_Switch)
		{
			case 0:
				//MAIN_Initialization();
				MAIN_DisplayDate();
				break;
			case 1:
				MAIN_Settings();
				break;
		}
		state ^= 1;
	}
}

static void MAIN_DisplayDate(void)
{
	SH1106_ClearBuffer();
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
	SH1106_SendBuffer();
}

static void MAIN_Settings(void)
{
	SH1106_ClearBuffer();
	SH1106_FontPrint(1, 0, 0, Arial12x12, "Settings");
	SH1106_FontPrint(1, 0, 13, Arial12x12, "A ton service Paul");
	SH1106_SendBuffer();	
}



