// ======================================================= library/BM8563/BM8563.cpp ==
// Name:	BM8563.cpp
// Version:	0.0.1
// Desc.:	
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
// ======================================================= library/BM8563/BM8563.cpp ==

#include "BM8563.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Section PUBLIC
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

RTC::RTC(uint8_t address, TwoWire &i2cPort) {
	_i2cAddress = address;
	_i2cPort    = &i2cPort;
}

void RTC::begin(void) {
	_i2cPort->begin(21, 22);

	writeReg(BM8563_CONTROL_STATUS1, 0x00);
	writeReg(BM8563_CONTROL_STATUS2, 0x00);
	writeReg(BM8563_CLKOUT_CONTROL,  0x00);
}

void RTC::getBM8563Time(void) {
	_i2cPort->beginTransmission(_i2cAddress);
	_i2cPort->write(0x02);
	_i2cPort->endTransmission();

	_i2cPort->requestFrom(_i2cAddress, (uint8_t)7);
	while (_i2cPort->available()) {
		trdata[0] = _i2cPort->read();
		trdata[1] = _i2cPort->read();
		trdata[2] = _i2cPort->read();
		trdata[3] = _i2cPort->read();
		trdata[4] = _i2cPort->read();
		trdata[5] = _i2cPort->read();
		trdata[6] = _i2cPort->read();
	}

	dataMask();
	bcd2asc();
	str2time();
}

void RTC::getTime(RTC_Time_t *RTC_TimeStruct) {

	uint8_t buffer[3] = { 0 };

	_i2cPort->beginTransmission(_i2cAddress);
	_i2cPort->write(0x02);
	_i2cPort->endTransmission();

	_i2cPort->requestFrom(_i2cAddress, (uint8_t)3);
	while (_i2cPort->available()) {
		buffer[0] = _i2cPort->read(); // rejestr sekund
		buffer[1] = _i2cPort->read(); // rejestr minut
		buffer[2] = _i2cPort->read(); // rejestr godzin
	}

	RTC_TimeStruct->Seconds = bcd2byte(buffer[0] & BM8563_SECONDS_MASK);
	RTC_TimeStruct->Minutes = bcd2byte(buffer[1] & BM8563_MINUTES_MASK);
	RTC_TimeStruct->Hours   = bcd2byte(buffer[2] & BM8563_HOURS_MASK);
}

void RTC::setTime(RTC_Time_t *RTC_TimeStruct) {

	if (RTC_TimeStruct == nullptr) return;

	_i2cPort->beginTransmission(_i2cAddress);
	_i2cPort->write(0x02);
	_i2cPort->write(byte2bcd(RTC_TimeStruct->Seconds));
	_i2cPort->write(byte2bcd(RTC_TimeStruct->Minutes));
	_i2cPort->write(byte2bcd(RTC_TimeStruct->Hours));
	_i2cPort->endTransmission();
}

void RTC::getDate(RTC_Date_t *RTC_DateStruct) {

	uint8_t buffer[4] = { 0 };

	_i2cPort->beginTransmission(_i2cAddress);
	_i2cPort->write(0x05);
	_i2cPort->endTransmission();

	_i2cPort->requestFrom(_i2cAddress, (uint8_t)4);
	while (_i2cPort->available()) {
		buffer[0] = _i2cPort->read();
		buffer[1] = _i2cPort->read();
		buffer[2] = _i2cPort->read();
		buffer[3] = _i2cPort->read();
	}

	RTC_DateStruct->Date    = bcd2byte(buffer[0] & BM8563_DAY_MASK);
	RTC_DateStruct->WeekDay = bcd2byte(buffer[1] & BM8563_WEEKDAY_MASK);
	RTC_DateStruct->Month   = bcd2byte(buffer[2] & BM8563_MONTH_MASK);

	if (buffer[2] & BM8563_CENTURY_BIT)
		RTC_DateStruct->Year = 1900 + bcd2byte(buffer[3] & 0xFF);
	else
		RTC_DateStruct->Year = 2000 + bcd2byte(buffer[3] & 0xFF);
}

void RTC::setDate(RTC_Date_t *RTC_DateStruct) {

	if (RTC_DateStruct == nullptr) return;

	_i2cPort->beginTransmission(_i2cAddress);
	_i2cPort->write(BM8563_DAY);
	_i2cPort->write(byte2bcd(RTC_DateStruct->Date));
	_i2cPort->write(byte2bcd(RTC_DateStruct->WeekDay));

	if (RTC_DateStruct->Year < 2000) {
		_i2cPort->write(byte2bcd(RTC_DateStruct->Month) | 0x80);
		_i2cPort->write(byte2bcd((uint8_t)(RTC_DateStruct->Year % 100)));
	} else {
		_i2cPort->write(byte2bcd(RTC_DateStruct->Month) | 0x00);
		_i2cPort->write(byte2bcd((uint8_t)(RTC_DateStruct->Year % 100)));
	}

	_i2cPort->endTransmission();
}

int RTC::setAlarmIRQ(int afterSeconds) {

	uint8_t regValue = readReg(BM8563_CONTROL_STATUS2);

	if (afterSeconds < 0) {
		regValue &= ~(1 << 0);
		writeReg(BM8563_CONTROL_STATUS2, regValue);

		regValue = 0x03;
		writeReg(BM8563_TIMER_CONTROL, regValue);

		return -1;
	}

	uint8_t typeValue = 2;
	uint8_t div = 1;

	if (afterSeconds > 255) {
		div = 60;
		typeValue = 0x83;
	} else {
		typeValue = 0x82;
	}

	afterSeconds = (afterSeconds / div) & 0xFF;
	writeReg(BM8563_TIMER, afterSeconds);
	writeReg(BM8563_TIMER_CONTROL, typeValue);

	regValue |=  (1 << 0);
	regValue &= ~(1 << 7);
	writeReg(BM8563_CONTROL_STATUS2, regValue);

	return afterSeconds * div;
}

int RTC::setAlarmIRQ(const RTC_Time_t &RTC_TimeStruct) {

	uint8_t irqEnable = false;

	uint8_t outBuffer[4] = { 0x80, 0x80, 0x80, 0x80 };

	if (RTC_TimeStruct.Minutes >= 0) {
		irqEnable = true;
		outBuffer[0] = byte2bcd(RTC_TimeStruct.Minutes) & BM8563_MINUTES_MASK;
	}

	if (RTC_TimeStruct.Hours >= 0) {
		irqEnable = true;
		outBuffer[1] = byte2bcd(RTC_TimeStruct.Hours) & BM8563_HOURS_MASK;
	}

	outBuffer[2] = 0x00;
	outBuffer[3] = 0x00;

	uint8_t regValue = readReg(BM8563_CONTROL_STATUS2);

	if (irqEnable) 
		regValue |= (1 << 1);
	else
		regValue &= ~(1 << 1);

	for (int i = 0; i < 4; i++)
		writeReg(BM8563_MINUTE_ALARM + i, outBuffer[i]);

	writeReg(BM8563_CONTROL_STATUS2, regValue);

	return irqEnable ? 1 : 0;
}

int RTC::setAlarmIRQ(const RTC_Date_t &RTC_DateStruct,
			const RTC_Time_t &RTC_TimeStruct) {

	uint8_t irqEnable = false;

	uint8_t outBuffer[4] = { 0x80, 0x80, 0x80, 0x80 };

	if (RTC_TimeStruct.Minutes >= 0) {
		irqEnable = true;
		outBuffer[0] = byte2bcd(RTC_TimeStruct.Minutes) & BM8563_MINUTES_MASK;
	}

	if (RTC_TimeStruct.Hours >= 0) {
		irqEnable = true;
		outBuffer[1] = byte2bcd(RTC_TimeStruct.Hours) & BM8563_HOURS_MASK;
	}

	if (RTC_DateStruct.Date >= 0) {
		irqEnable = true;
		outBuffer[2] = byte2bcd(RTC_DateStruct.Date) & BM8563_DAY_MASK;
	}

	if (RTC_DateStruct.WeekDay >= 0) {
		irqEnable = true;
		outBuffer[3] = byte2bcd(RTC_DateStruct.WeekDay) & BM8563_WEEKDAY_MASK;
	}

	uint8_t regValue = readReg(BM8563_CONTROL_STATUS2);

	if (irqEnable)
		regValue |=  (1 << 1);
	else
		regValue &= ~(1 << 1);

	for (int i = 0; i < 4; i++) 
		writeReg(BM8563_MINUTE_ALARM + i, outBuffer[i]);

	writeReg(BM8563_CONTROL_STATUS2, regValue);

	return irqEnable ? 1 : 0;
}

void RTC::clearIRQ() {

	uint8_t data = readReg(BM8563_CONTROL_STATUS2);
	writeReg(BM8563_CONTROL_STATUS2, data & 0xF3); // clear bits TIE i AIE
}

void RTC::disableIRQ() {

	clearIRQ();

	uint8_t data = readReg(BM8563_CONTROL_STATUS2);
	writeReg(BM8563_CONTROL_STATUS2, data & 0xFC); // clear bits TIE i AIE
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// sekcja PRIVATE
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void RTC::bcd2asc(void) {
	uint8_t i, j;

	for (j = 0, i = 0; i < 7; i++) {
		asc[j++] = (trdata[i] & 0xF0) >> 4 | 0x30;
		asc[j++] = (trdata[i] & 0x0F)      | 0x30;
	}
}

void RTC::dataMask() {
	trdata[0] = trdata[0] & BM8563_SECONDS_MASK; // sekundy
	trdata[1] = trdata[1] & BM8563_MINUTES_MASK; // minuty
	trdata[2] = trdata[2] & BM8563_HOURS_MASK;   // godziny
	trdata[3] = trdata[3] & BM8563_DAY_MASK;     // dzień
	trdata[4] = trdata[4] & BM8563_WEEKDAY_MASK; // dzień tygodnia
	trdata[5] = trdata[5] & BM8563_MONTH_MASK;   // miesiąc
	trdata[6] = trdata[6] & BM8563_YEAR_MASK;    // rok
}

void RTC::str2time(void) {
	Second = (asc[0] - 0x30) * 10 + asc[1] - 0x30;
	Minute = (asc[2] - 0x30) * 10 + asc[3] - 0x30;
	Hour   = (asc[4] - 0x30) * 10 + asc[5] - 0x30;
}

uint8_t RTC::bcd2byte(uint8_t value) {
	uint8_t temp = 0;
	temp = ((uint8_t)(value & (uint8_t)0xF0) >> (uint8_t)0x4) * 10;

	return (temp + (value & (uint8_t)0x0F));
}

uint8_t RTC::byte2bcd(uint8_t value) {

	uint8_t bcdHigh = 0;

	while (value >= 10) {
		bcdHigh++;
		value -= 10;
	}

	return ((uint8_t)(bcdHigh << 4) | value);
}

void RTC::writeReg(uint8_t reg, uint8_t data) {
	_i2cPort->beginTransmission(_i2cAddress);
	_i2cPort->write(reg);
	_i2cPort->write(data);
	_i2cPort->endTransmission();
}

uint8_t RTC::readReg(uint8_t reg) {
	_i2cPort->beginTransmission(_i2cAddress);
	_i2cPort->write(reg);
	_i2cPort->endTransmission();

	_i2cPort->requestFrom(_i2cAddress, (uint8_t)1);
	return _i2cPort->read();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Section PROTECTED
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ====================================================================================
// Date:	20.12.2020
// Author:	(©) 2020 Wojciech Szajnerman <w.szajnerman@gmail.com>
// ================================================ end of library/BM8563/BM8563.cpp ==