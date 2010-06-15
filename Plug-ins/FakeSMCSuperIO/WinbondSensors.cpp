/*
 *  WinbondSensors.cpp
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 15/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "WinbondSensors.h"

void WinbondTemperatureSensor::OnKeyRead(__unused const char* key, char* data)
{
	data[0] = m_Provider->ReadTemperature(0);
	data[1] = 0;
}

void WinbondTemperatureSensor::OnKeyWrite(__unused const char* key, __unused char* data)
{
	
}

void WinbondVoltageSensor::OnKeyRead(__unused const char* key, char* data)
{
	UInt16 value = fp2e_Encode(m_Provider->ReadVoltage(m_Offset));
	
	data[0] = (value & 0xff00) >> 8;
	data[1] = value & 0x00ff;
}

void WinbondVoltageSensor::OnKeyWrite(__unused const char* key, __unused char* data)
{
	
}

void WinbondTachometerSensor::OnKeyRead(__unused const char* key, char* data)
{
	UInt16 value = m_Provider->ReadTachometer(m_Offset, false);
	
	data[0] = (value >> 6) & 0xff;
	data[1] = (value << 2) & 0xff;
}

void WinbondTachometerSensor::OnKeyWrite(__unused const char* key, __unused char* data)
{
	
}