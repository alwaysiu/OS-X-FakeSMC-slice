/*
 *  SmartFanIIIController.cpp
 *  FakeSMCSuperIOMonitor
 *
 *  Created by Mozodojo on 13/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "SmartFanIIIController.h"

void SmartFanIIIController::ForcePWM(UInt8 slope)
{
	DebugLog("Forcing Fan #%d SLOPE=0x%x", m_Offset, slope);
	
	//FIXME: Check for DC and PWM mode (if DC, then only 6 bits are used!)
	
	outb(m_Provider->GetAddress()+WINBOND_ADDRESS_REGISTER_OFFSET, WINBOND_FAN_OUTPUT_VALUE[0]);
	outb(m_Provider->GetAddress()+WINBOND_DATA_REGISTER_OFFSET, slope);
}

void SmartFanIIIController::Initialize()
{
	if (m_Index>2)
	{

		//bool* valid;
		char tmpKey[5];
	
		//Back up config register
	
		outb(m_Provider->GetAddress() + WINBOND_ADDRESS_REGISTER_OFFSET, WINBOND_FAN_CONFIG);
		UInt8 config=inb(m_Provider->GetAddress()+WINBOND_DATA_REGISTER_OFFSET);
	
			switch((config&0x30)>>4) {
				case 0x00:
					IOLog("Fan %d is in manual mode\n", m_Index);
					break;
				case 0x01:
					IOLog("Fan %d is in Thermal Cruise(TM) mode\n", m_Index);
					break;
				case 0x2:
					IOLog("Fan %d is in Fan Speed Cruise(TM) mode (why???)\n", m_Index);
					break;
				case 0x3:
					IOLog("Fan %d is in Smart Fan III(TM) mode\n", m_Index);			
			}
	
		//Switch to manual mode
		outb(m_Provider->GetAddress()+WINBOND_ADDRESS_REGISTER_OFFSET, WINBOND_FAN_CONFIG);
		outb(m_Provider->GetAddress()+WINBOND_DATA_REGISTER_OFFSET, config&0xcf);
	
	
	
		char value[2];
		
		UInt16 initial = m_Maximum = m_Provider->ReadTachometer(m_Offset, false);
		
		//Forcing maximum speed
		ForcePWM(0xff);
		
		UInt16 last = initial, count = 0;
		
		//Waiting cooler will speed up to maximum
		while (count < 5)
		{
			IOSleep(1000);
			
			m_Maximum = m_Provider->ReadTachometer(m_Offset, false);
			
			if (m_Maximum < last + 50)
			{
				count++;
			}
			else 
			{
				last = m_Maximum;
			}
		};
		
		//Forcing minimum speed
		ForcePWM(0x00);
		
		count = 0;
		
		//Waiting cooler will slow down to minimum
		while (count < 5)
		{
			IOSleep(1000);
			
			m_Minimum = m_Provider->ReadTachometer(m_Offset, false);
			
			if (m_Minimum > last - 50)
			{
				count++;
			}
			else 
			{
				last = m_Minimum;
			}
		}
		
	
					
		m_Maximum = m_Maximum / 50 * 50;
		
		DebugLog("Fan #%d MAX=%drpm", m_Offset, m_Maximum);
		
		value[0] = (m_Maximum << 2) >> 8;
		value[1] = (m_Maximum << 2) & 0xff;
				
		snprintf(tmpKey, 5, "F%dMx", m_Index);
		FakeSMCAddKey(tmpKey, "fpe2", 2, value);
		
		initial = initial / 50 * 50;
		
		value[0] = (initial << 2) >> 8;
		value[1] = (initial << 2) & 0xff;
		
		snprintf(tmpKey, 5, "F%dTg", m_Index);
		FakeSMCAddKey(tmpKey, "fpe2", 2, value);
				
		if (m_Maximum > initial + 100)
		{
			value[0] = 0;//(initial << 2) >> 8;
			value[1] = 0;//(initial << 2) & 0xff;
					
			m_Key = (char*)IOMalloc(5);
			snprintf(m_Key, 5, "F%dMn", m_Index);
					
			InfoLog("Binding key %s", m_Key);
					
			FakeSMCAddKey(m_Key, "fpe2", 2, value, this);
		}
		else 
		{
			value[0] = (initial << 2) >> 8;
			value[1] = (initial << 2) & 0xff;
			
			m_Key = (char*)IOMalloc(5);
			snprintf(m_Key, 5, "F%dMn", m_Index);
			
			FakeSMCAddKey(m_Key, "fpe2", 2, value);
		}
	
	
		//Restore configuration register
		outb(m_Provider->GetAddress()+WINBOND_ADDRESS_REGISTER_OFFSET, WINBOND_FAN_CONFIG);
		outb(m_Provider->GetAddress()+WINBOND_DATA_REGISTER_OFFSET, config);
	}
}

void SmartFanIIIController::OnKeyRead(__unused const char* key, __unused char* data)
{
}

void SmartFanIIIController::OnKeyWrite(__unused const char* key, char* data)
{
	if (m_Maximum>m_Minimum){
		UInt16 rpm = (UInt16(data[0] << 8) | (data[1] & 0xff)) >> 2;
		UInt8 slope;
		if (rpm<m_Minimum)
			slope=0;
		else if (rpm>m_Maximum)
			slope=0x7f;
		else
			slope = (rpm-m_Minimum) * 127 / (m_Maximum-m_Minimum);
		//DebugLog("Fan %d start slope=0x%x", m_Offset, slope, rpm);
		
		outb(m_Provider->GetAddress() + WINBOND_ADDRESS_REGISTER_OFFSET, WINBOND_SMARTFANIII_MIN_PWM[m_Offset]);
		outb(m_Provider->GetAddress() + WINBOND_DATA_REGISTER_OFFSET, slope);
	}	
}