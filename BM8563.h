// ========================================================= library/BM8563/BM8563.h ==
// Name:	BM8563.h
// Version:	0.0.1
// Desc.:	Header file for class BM8563.
//
// Licence:	Permission is hereby granted, free of charge, to any person obtaining
//			a copy of this software and associated documentation files (the
//			"Software"), to deal in the Software without restriction, including
//			without limitation the rights to use, copy, modify, merge, publish,
//			distribute, sublicense, and/or sell copies of the Software, and to permit
//			persons to whom the Software is furnished to do so, subject to the
//			following conditions: This permission notice shall be included in all
//			copies or substantial portions of the Software.
//
//			THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//			IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//			FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//			THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//			LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//			FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//			DEALINGS IN THE SOFTWARE.
// ========================================================= library/BM8563/BM8563.h ==

#if ARDUINO >= 100
	#include <Arduino.h>
#else
	#include <WProgram.h>
#endif
#include "libTagVersion.h"
#include <Wire.h>

#ifndef _BM8563_H
	#define _BM8563_H

	#define BM8563_I2C_ADDRESS				(0x51)

	// [ 0x00 ] Control / Status Register 1
	//
	#define BM8563_CONTROL_STATUS1			(0x00)
	#define BM8563_TESTC_BIT				(0b00001000)
	#define BM8563_STOP_BIT					(0b00100000)
	#define BM8563_TEST1_BIT				(0b10000000)

	// [ 0x01 ] Control / Status Register 2
	//
	#define BM8563_CONTROL_STATUS2			(0x01)

	// bit Timer Interrupt Enable
	// TIE = 0: interrupt from timer is disable
	// TIE = 1: interrupt from timer is enable
	#define BM8563_TIE_BIT					(0b00000001)

	// bit Alarm Interrupt Enable
	// AIE = 0: interrupt from alarm is disable
	// AIE = 1: interrupt from alarm is enable
	#define BM8563_AIE_BIT					(0b00000010)

	// bit Timer Flag
	// TF = 0: 
	// TF = 1: 
	#define BM8563_TF_BIT					(0b00000100)

	// bit Alarm Flag
	// AF = 0:
	// AF = 1:
	#define BM8563_AF_BIT					(0b00001000)

	#define BM8563_TI_TP_BIT				(0b00010000)

	// [ 0x02 ] Time/Date Register (7 bytes)
	//
	#define BM8563_SECONDS					(0x02)
	#define BM8563_VL_BIT					(0b10000000)
	#define BM8563_SECONDS_MASK				(0b01111111) // 0x7F

	#define BM8563_MINUTES					(0x03)
	#define BM8563_MINUTES_MASK				(0b01111111) // 0x7F

	#define BM8563_HOURS					(0x04)
	#define BM8563_HOURS_MASK				(0b00111111) // 0x3F

	#define BM8563_DAY						(0x05)
	#define BM8563_DAY_MASK					(0b00111111) // 0x3F

	// [ 0x06 ] Weekday Register
	//
	//    Bit 2   Bit 1   Bit 0    Day of week
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//    0       0       1        Sunday
	//    0       0       1        Monday
	//    0       1       0        Tuesday
	//    0       1       1        Wednesday
	//    1       0       0        Thursday
	//    1       0       1        Friday
	//    1       1       0        Saturday

	#define BM8563_WEEKDAY					(0x06)
	#define BM8563_WEEKDAY_MASK				(0b00000111)

	// [ 0x07 ] Month Register
	//
	//    Bit 4   Bit 3   Bit 2   Bit 1   Bit 0    Month
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//    0       0       0       0       1        January
	//    0       0       0       1       0        February
	//    0       0       0       1       1        March
	//    0       0       1       0       0        April
	//    0       0       1       0       1        May
	//    0       0       1       1       0        June
	//    0       0       1       1       1        July
	//    0       1       0       0       0        August
	//    0       1       0       0       1        September
	//    1       0       0       0       0        October
	//    1       0       0       0       1        November
	//    1       0       0       1       0        December
	//
	#define BM8563_MONTH					(0x07)
	#define BM8563_MONTH_MASK				(0b00011111)

	#define BM8563_YEAR						(0x08)
	#define BM8563_YEAR_MASK				(0b11111111)

	#define BM8563_TIME_SIZE				(0x07)

	// bit CENTURY
	//     C = 0 century like a 20xx
	//     C = 1 century like a 19xx
	//
	#define	BM8563_CENTURY_BIT				(0b10000000)

	//
	#define BM8563_MINUTE_ALARM				(0x09)
	#define BM8563_HOUR_ALARM				(0x0A)
	#define BM8563_DAY_ALARM				(0x0B)
	#define BM8563_WEEKDAY_ALARM			(0x0C)

	#define BM8563_ALARM_DISABLE_BIT		(0b10000000)
	#define BM8563_ALARM_NONE				(0xFF)
	#define BM8563_ALARM_SIZE				(0x04)

	// [ 0x0D ] CLKOUT Register
	//
	#define BM8563_CLKOUT_CONTROL			(0x0D)

	// [ 0x0E ] Timer Control Register
	//
	#define BM8563_TIMER_CONTROL			(0x0E)
	#define BM8563_TIMER_ENABLE_BIT			(0b10000000) // 
	#define BM8563_TIMER_4_096KHZ_BIT		(0b00000000) //
	#define BM8563_TIMER_64HZ_BIT			(0b00000001) //
	#define BM8563_TIMER_1HZ_BIT			(0b00000010) //
	#define BM8563_TIMER_1_60HZ_BIT			(0b00000011) //

	// [ 0x0F ] Timer Countdown Register
	//
	#define BM8563_TIMER					(0x0F)

	// ~~~~ Structure, Variable, Typedef ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	typedef struct RTC_Time {
		int8_t Hours;
		int8_t Minutes;
		int8_t Seconds;

		RTC_Time() : Hours(), Minutes(), Seconds() {}

		RTC_Time(int8_t h, int8_t m, int8_t s) : Hours(h), Minutes(m), Seconds(s){}
	} RTC_Time_t;

	typedef struct RTC_Date {
		int8_t  WeekDay;
		int8_t  Month;
		int8_t  Date;
		int16_t Year;

		RTC_Date() : WeekDay(), Month(), Date(), Year() {}

		RTC_Date(int8_t w, int8_t m, int8_t d, int16_t y) : WeekDay(w), Month(m),
				Date(d), Year(y){}

	} RTC_Date_t;

	// ~~~~ main class BM8563 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class BM8563 {
		public:
			BM8563(uint8_t address, TwoWire &i2cPort);

			String version(void) const { return BM8563_LIB_VERSION; }

			void begin(void);

			void getBM8563Time(void);

			void setTime(RTC_Time_t *RTC_TimeStruct);
			void setDate(RTC_Date_t *RTC_DateStruct);

			void getTime(RTC_Time_t *RTC_TimeStruct);
			void getDate(RTC_Date_t *RTC_DateStruct);

			int setAlarmIRQ(int afterSeconds);
			int setAlarmIRQ(const RTC_Time_t &RTC_TimeStruct);
			int setAlarmIRQ(const RTC_Date_t &RTC_DateStruct,
					const RTC_Time_t &RTC_TimeStruct);

			void clearIRQ();
			void disableIRQ();

			uint8_t Second;
			uint8_t Minute;
			uint8_t Hour;
			uint8_t Week;
			uint8_t Day;
			uint8_t Month;
			uint8_t Year;

			uint8_t DateString[9];
			uint8_t TimeString[9];

			uint8_t asc[14];

		private:
			uint8_t  _i2cAddress;
			TwoWire *_i2cPort;

			void bcd2asc(void);
			void dataMask();
			void str2time(void);

			uint8_t bcd2byte(uint8_t Value);
			uint8_t byte2bcd(uint8_t Value);

			uint8_t trdata[7];

			// ~~~~ I/function for communication with uP ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

			void writeReg(uint8_t reg, uint8_t data);
			uint8_t readReg(uint8_t reg);

	};

#endif

// ====================================================================================
// Date:	20.12.2020
// Author:	(©) 2020 Wojciech Szajnerman <w.szajnerman@gmail.com>
// ================================================== end of library/BM8563/BM8563.h ==