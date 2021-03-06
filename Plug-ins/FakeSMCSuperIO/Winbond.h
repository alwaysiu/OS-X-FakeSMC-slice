/*
 *  Winbond.h
 *  FakeSMCLPCMonitor
 *
 *  Created by Mozodojo on 29/05/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 *  This code contains parts of original code from Open Hardware Monitor
 *  Copyright 2010 Michael Möller. All rights reserved.
 *
 */

#ifndef _WINBOND_H
#define _WINBOND_H 

#include "SuperIO.h"

const UInt8 WINBOND_HARDWARE_MONITOR_LDN	= 0x0B;

const UInt16 WINBOND_VENDOR_ID = 0x5CA3;
const UInt8 WINBOND_HIGH_BYTE = 0x80;

// Winbond Hardware Monitor
const UInt8 WINBOND_ADDRESS_REGISTER_OFFSET = 0x05;
const UInt8 WINBOND_DATA_REGISTER_OFFSET = 0x06;

// Winbond Hardware Monitor Registers
const UInt8 WINBOND_BANK_SELECT_REGISTER = 0x4E;
//const UInt8 WINBOND_VENDOR_ID_REGISTER = 0x4F;
const UInt8 WINBOND_TEMPERATURE_SOURCE_SELECT_REG = 0x49;

//private string[] TEMPERATURE_NAME = 
//new string[] {"CPU", "Auxiliary", "System"};
const UInt8 WINBOND_TEMPERATURE[]			= { 0x50, 0x50, 0x27 };
const UInt8 WINBOND_TEMPERATURE_BANK[]		= { 1,    2,    0 };

const UInt8 WINBOND_VOLTAGE					= 0x20;

const UInt8 WINBOND_TACHOMETER[]			= { 0x28, 0x29, 0x2A, 0x3F, 0x53 };
const UInt8 WINBOND_TACHOMETER_BANK[]		= { 0, 0, 0, 0, 5 };

const UInt8 WINBOND_TACHOMETER_DIV0[]		= { 0x47, 0x47, 0x4B, 0x59, 0x59 };
const UInt8 WINBOND_TACHOMETER_DIV0_BIT[]	= { 4,    6,    6,    0,    2 };
const UInt8 WINBOND_TACHOMETER_DIV1[]		= { 0x47, 0x47, 0x4B, 0x59, 0x59 };
const UInt8 WINBOND_TACHOMETER_DIV1_BIT[]	= { 5,    7,    7,    1,    3 };
const UInt8 WINBOND_TACHOMETER_DIV2[]		= { 0x5D, 0x5D, 0x5D, 0x4C, 0x59 };
const UInt8 WINBOND_TACHOMETER_DIV2_BIT[]	= { 5,    6,    7,    7,    7 };

const UInt8 WINBOND_TACHOMETER_DIVISOR[]	= { 0x47, 0x4B, 0x4C, 0x59, 0x5D };
const UInt8 WINBOND_TACHOMETER_DIVISOR0[]	= { 36, 38, 30, 8, 10 };
const UInt8 WINBOND_TACHOMETER_DIVISOR1[]	= { 37, 39, 31, 9, 11 };
const UInt8 WINBOND_TACHOMETER_DIVISOR2[]	= { 5, 6, 7, 23, 15 };

// Fan Control
const UInt8 WINBOND_FAN_CONFIG[]			= { 0x04, 0x04, 0x12, 0x62 };
const UInt8 WINBOND_FAN_CONTROL_BIT[]		= { 0x02, 0x04, 0x01, 0x04 };
const UInt8 WINBOND_FAN_MODE_BIT[]			= { 0x00, 0x01, 0x00, 0x06 };
const UInt8 WINBOND_FAN_OUTPUT[]			= { 0x01, 0x03, 0x11, 0x61 };

class Winbond : public SuperIO
{
private:
	UInt8	m_FanLimit;
	UInt16	m_FanValue[5];
	bool	m_FanValueObsolete[5];
	
public:
	void	WriteByte(UInt8 bank, UInt8 reg, UInt8 value);
	UInt8	ReadByte(UInt8 bank, UInt8 reg);
	void	UpdateTachometers();
	
	SInt16	ReadTemperature(UInt8 index);
	SInt16	ReadVoltage(UInt8 index);
	SInt16	ReadTachometer(UInt8 index, bool force_update);
	
	virtual bool	ControllersTimerEvent();
	
	virtual void	Enter();
	virtual void	Exit();
	
	virtual bool	ProbePort();
	
	virtual void	Start();
};

#endif