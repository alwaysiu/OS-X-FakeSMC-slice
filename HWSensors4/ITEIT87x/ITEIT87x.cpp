/*
 *  IT8718F.cpp
 *  HWSensors
 *
 *  Created by mozo on 08/10/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include <architecture/i386/pio.h>
#include "ITEIT87x.h"
#include "FakeSMC.h"

#define Debug false

#define LogPrefix "ITEIT87x: "
#define DebugLog(string, args...)	do { if (Debug) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

#define super SuperIOMonitor
OSDefineMetaClassAndStructors(IT87x, SuperIOMonitor)

OSDefineMetaClassAndStructors(IT87xSensor, SuperIOSensor)

#pragma mark IT87xSensor implementation

SuperIOSensor * IT87xSensor::withOwner(SuperIOMonitor *aOwner, const char* aKey, const char* aType, unsigned char aSize, SuperIOSensorGroup aGroup, unsigned long aIndex)
{
	SuperIOSensor *me = new IT87xSensor;
	
    if (me && !me->initWithOwner(aOwner, aKey, aType, aSize, aGroup, aIndex)) {
        me->release();
        return 0;
    }
	
    return me;
}

long IT87xSensor::getValue()
{
	UInt16 value = 0;
	switch (group) {
		case kSuperIOTemperatureSensor:
			value = owner->readTemperature(index);
			break;
		case kSuperIOVoltageSensor:
			value = owner->readVoltage(index);
			break;
		case kSuperIOTachometerSensor:
			value = owner->readTachometer(index);
			break;
        case kSuperIOSmartGuardPWMControl:
            value = OSDynamicCast(IT87x, owner)->readSmartGuardPWMControl(index);
            break;
        case kSuperIOSmartGuardTempFanStop:
            value = OSDynamicCast(IT87x, owner)->readSmartGuardTempFanStop(index);
            break;
        case kSuperIOSmartGuardTempFanStart:
            value = OSDynamicCast(IT87x, owner)->readSmartGuardTempFanStart(index);
            break;
        case kSuperIOSmartGuardTempFanFullOn:
            value = OSDynamicCast(IT87x, owner)->readSmartGuardTempFanFullOn(index);
            break;
        case kSuperIOSmartGuardPWMStart:
            value = OSDynamicCast(IT87x, owner)->readSmartGuardPWMStart(index);
            break;
        case kSuperIOSmartGuardTempFanFullOff:
            value = OSDynamicCast(IT87x, owner)->readSmartGuardTempFanFullOff(index);
            break;
        case kSuperIOSmartGuardTempFanControl:
            value = OSDynamicCast(IT87x, owner)->readSmartGuardFanControl(index);
            break;
            
		default:
			break;
	}
	if (*((uint32_t*)type) == *((uint32_t*)TYPE_FP2E)) {
		value = encode_fp2e(value);
	}
	else if (*((uint32_t*)type) == *((uint32_t*)TYPE_FPE2)) {
		value = encode_fpe2(value);
	}
	
	return value;
}


void IT87xSensor::setValue(UInt16 value)
{
    
	switch (group) {
            
        case kSuperIOSmartGuardPWMControl:
            OSDynamicCast(IT87x, owner)->writeSmartGuardPWMControl(index,value);
            break;
        case kSuperIOSmartGuardTempFanStop:
            OSDynamicCast(IT87x, owner)->writeSmartGuardTempFanStop(index,value);
            break;
        case kSuperIOSmartGuardTempFanStart:
            OSDynamicCast(IT87x, owner)->writeSmartGuardTempFanStart(index,value);
            break;
        case kSuperIOSmartGuardTempFanFullOn:
            OSDynamicCast(IT87x, owner)->writeSmartGuardTempFanFullOn(index,value);
            break;
        case kSuperIOSmartGuardPWMStart:
            OSDynamicCast(IT87x, owner)->writeSmartGuardPWMStart(index,value);
            break;
        case kSuperIOSmartGuardTempFanFullOff:
            OSDynamicCast(IT87x, owner)->writeSmartGuardTempFanFullOff(index,value);
            break;
        case kSuperIOSmartGuardTempFanControl:
            OSDynamicCast(IT87x, owner)->writeSmartGuardFanControl(index,value);
            break;
            
		default:
			break;
	}
    
    // Damn... need someone to explain the encoding scheme of SMC keys... but actually works well without it
    //	if (*((uint32_t*)type) == *((uint32_t*)TYPE_FP2E)) {
    //		value = encode_fp2e(value);
    //	}
    //	else if (*((uint32_t*)type) == *((uint32_t*)TYPE_FPE2)) {
    //		value = encode_fpe2(value);
    //	}
    //	
    //	return value;
}

#pragma mark IT87x implementation

long IT87x::readSmartGuardPWMControl(unsigned long index)
{
	return readByte(address, ITE_SMARTGUARDIAN_PWM_CONTROL[index]);
}


void IT87x::writeSmartGuardPWMControl(unsigned long index,UInt16 value)
{
    
	writeByte(address,ITE_SMARTGUARDIAN_PWM_CONTROL[index], value);
}

long IT87x::readSmartGuardTempFanStop(unsigned long index)
{
	return readByte(address, ITE_SMARTGUARDIAN_TEMPERATURE_STOP[index]);
}

void IT87x::writeSmartGuardTempFanStop(unsigned long index,UInt16 value)
{
	writeByte(address,ITE_SMARTGUARDIAN_TEMPERATURE_STOP[index], value);
}

long IT87x::readSmartGuardTempFanStart(unsigned long index)
{
	return readByte(address, ITE_SMARTGUARDIAN_TEMPERATURE_START[index]);
}

void IT87x::writeSmartGuardTempFanStart(unsigned long index,UInt16 value)
{
    writeByte(address, ITE_SMARTGUARDIAN_TEMPERATURE_START[index], value);
}

long IT87x::readSmartGuardTempFanFullOn(unsigned long index)
{
    
	return readByte(address, ITE_SMARTGUARDIAN_TEMPERATURE_FULL_ON[index]);
}

void IT87x::writeSmartGuardTempFanFullOn(unsigned long index,UInt16 value)
{
    writeByte(address,ITE_SMARTGUARDIAN_TEMPERATURE_FULL_ON[index], value);
}

long IT87x::readSmartGuardPWMStart(unsigned long index)
{
    
	return readByte(address, ITE_SMARTGUARDIAN_START_PWM[index]);
}

void IT87x::writeSmartGuardPWMStart(unsigned long index,UInt16 value)
{
    writeByte(address,ITE_SMARTGUARDIAN_START_PWM[index], value);
}

long IT87x::readSmartGuardTempFanFullOff(unsigned long index)
{
	
	return readByte(address, ITE_SMARTGUARDIAN_TEMPERATURE_DELTA[index]);
}

void IT87x::writeSmartGuardTempFanFullOff(unsigned long index,UInt16 value)
{
    writeByte(address,ITE_SMARTGUARDIAN_TEMPERATURE_DELTA[index], value);
}

long IT87x::readSmartGuardFanControl(unsigned long index)
{
	
	return readByte(address, ITE_SMARTGUARDIAN_CONTROL[index]);
}

void IT87x::writeSmartGuardFanControl(unsigned long index,UInt16 value)
{
    writeByte(address,ITE_SMARTGUARDIAN_CONTROL[index], value);
}

long IT87x::readTemperature(unsigned long index)
{
    
	return readByte(address, ITE_TEMPERATURE_BASE_REG + index);
}

long IT87x::readVoltage(unsigned long index)
{
    
    return readByte(address, ITE_VOLTAGE_BASE_REG + index) * voltageGain;
    
}

long IT87x::readTachometer(unsigned long index)
{
	int value = readByte(address, ITE_FAN_TACHOMETER_REG[index]);
	
	value |= readByte(address, ITE_FAN_TACHOMETER_EXT_REG[index]) << 8;
	
	return value > 0x3f && value < 0xffff ? (float)(1350000 + value) / (float)(value * 2) : 0;
}




bool IT87x::probePort()
{	
	UInt16 id = listenPortWord(SUPERIO_CHIP_ID_REGISTER);
	
	if (id == 0 || id == 0xffff) {
        DebugLog("invalid super I/O chip ID=0x%x", id);
		return false;
    }
	hasSmartGuardian = false;
	switch (id)
	{
		case IT8512F:
		case IT8712F:
		case IT8716F:
		case IT8718F:
		case IT8720F: 
		case IT8721F: 
		case IT8726F:
		case IT8728F:
		case IT8752F:
        case IT8772E:
			model = id; 
			break; 
		default:
			WarningLog("found unsupported chip ID=0x%x", id);
			return false;
	}
    
	selectLogicalDevice(ITE_ENVIRONMENT_CONTROLLER_LDN);
    
    IOSleep(50);
	
	if (!getLogicalDeviceAddress()) {
        DebugLog("can't get monitoring LDN address");
		return false;
    }
    
	UInt8 vendor = readByte(address, ITE_VENDOR_ID_REGISTER);
	
	if (vendor != ITE_VENDOR_ID) {
        DebugLog("invalid vendor ID=0x%x", vendor);
		return false;
    }
	
	if ((readByte(address, ITE_CONFIGURATION_REGISTER) & 0x10) == 0) {
        DebugLog("invalid configuration register value");
		return false;
    }
	
    if (id == IT8721F || id == IT8728F || id == IT8772E)
        voltageGain = 12;
    else
        voltageGain = 16;
    
    UInt8 version = readByte(address, ITE_VERSION_REGISTER) & 0x0F;
    
    if (id == IT8712F && version < 8)
        has16bitFanCounter = false;
    else
        has16bitFanCounter = true;
	
	return true;
    
}

void IT87x::enter()
{
	outb(registerPort, 0x87);
	outb(registerPort, 0x01);
	outb(registerPort, 0x55);
	
	if (registerPort == 0x4e) 
	{
		outb(registerPort, 0xaa);
	}
	else
	{
		outb(registerPort, 0x55);
	}
}

void IT87x::exit()
{
	outb(registerPort, SUPERIO_CONFIGURATION_CONTROL_REGISTER);
	outb(valuePort, 0x02);
}

const char *IT87x::getModelName()
{
	switch (model) 
	{
        case IT8512F: return "IT8512F";
        case IT8712F: return "IT8712F";
        case IT8716F: return "IT8716F";
        case IT8718F: return "IT8718F";
        case IT8720F: return "IT8720F";
        case IT8721F: return "IT8721F";
        case IT8726F: return "IT8726F";
		case IT8728F: return "IT8728F";
        case IT8752F: return "IT8752F";
        case IT8772E: return "IT8772E";
	}
	
	return "unknown";
}




bool IT87x::startPlugin()
{
	DebugLog("starting...");

	
	InfoLog("found ITE %s", getModelName());
    OSDictionary* list = OSDynamicCast(OSDictionary, getProperty("Sensors Configuration"));
    OSDictionary* configuration = list ? OSDynamicCast(OSDictionary, list->getObject(getModelName())) : 0;
    
    if (list && !configuration) 
        configuration = OSDynamicCast(OSDictionary, list->getObject("Default"));
	
	// Temperature Sensors
	if (configuration) {
		for (int i = 0; i < 3; i++) 
		{				
			char key[8];
			
			snprintf(key, 8, "TEMPIN%X", i);
            if(readTemperature(i)<MAX_TEMP_THRESHOLD)  // Need to check if temperature sensor valid
    			if (OSString* name = OSDynamicCast(OSString, configuration->getObject(key)))
                    if (name->isEqualTo("Processor")) {
                        if (!addSensor(KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, i))
                            WarningLog("error adding heatsink temperature sensor");
                    }
                    else if (name->isEqualTo("System")) {				
                        if (!addSensor(KEY_NORTHBRIDGE_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor,i))
                            WarningLog("error adding system temperature sensor");
                    }
                    else if (name->isEqualTo("Auxiliary")) {				
                        if (!addSensor(KEY_AMBIENT_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor,i))
                            WarningLog("error adding auxiliary temperature sensor");
				}
		}
	}
	else {
        if(readTemperature(0)<MAX_TEMP_THRESHOLD)  // Need to check if temperature sensor valid
             if (!addSensor(KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 0))
                 WarningLog("error adding heatsink temperature sensor");
        if(readTemperature(1)<MAX_TEMP_THRESHOLD)  // Need to check if temperature sensor valid
            if (!addSensor(KEY_AMBIENT_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 1))
                WarningLog("error adding auxiliary temperature sensor");
        if(readTemperature(2)<MAX_TEMP_THRESHOLD)  // Need to check if temperature sensor valid
            if (!addSensor(KEY_NORTHBRIDGE_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 2))
                WarningLog("error adding system temperature sensor");
	}
	
	
	// Voltage
	if (configuration) {
		for (int i = 0; i < 9; i++) //Zorglub
		{				
			char key[5];
			
			snprintf(key, 5, "VIN%X", i);
			
			if (OSString* name = OSDynamicCast(OSString, configuration->getObject(key))) {
				if (name->isEqualTo("Processor")) {
					if (!addSensor(KEY_CPU_VOLTAGE, TYPE_FP2E, 2, kSuperIOVoltageSensor, i))
						WarningLog("error adding CPU voltage sensor");
				}
				else if (name->isEqualTo("Memory")) {
					if (!addSensor(KEY_MEMORY_VOLTAGE, TYPE_FP2E, 2, kSuperIOVoltageSensor, i))
						WarningLog("error adding memory voltage sensor");
				}
			}
		}
	}
	
	// Tachometers
	for (int i = 0; i < 5; i++) {
		OSString* name = NULL;
		char key[5];
		if (configuration) {
			char key_temp[7];
			
			snprintf(key_temp, 7, "FANIN%X", i);
			
			name = OSDynamicCast(OSString, configuration->getObject(key_temp));
		}
		
		UInt32 nameLength = name ? strlen(name->getCStringNoCopy()) : 0;
		
		if (readTachometer(i) > 10 || nameLength > 0)
            // Pff WTF ??? Add tachometer if it doesn't exist in a system but only the name defined in the config???   
        {
            
			if (!addTachometer(i, (nameLength > 0 ? name->getCStringNoCopy() : 0)))
                // Need to look at this a bit later
				WarningLog("error adding tachometer sensor %d", i);
            
            
            
        }
        
        // Check if this chip support SmartGuardian feature  
        
        hasSmartGuardian=false;
        if(configuration)
        {
            if(OSBoolean* smartGuard=OSDynamicCast(OSBoolean, configuration->getObject("SmartGuardian")))
                if(smartGuard->isTrue())
                    hasSmartGuardian=true;
            
        }
        
        if(hasSmartGuardian)
        {
            // Ugly development hack started for (SuperIOSensorGroup)
            snprintf(key,5,KEY_FORMAT_FAN_TARGET_SPEED,i);
            if (!addSensor(key, TYPE_UI8, 1, (SuperIOSensorGroup)kSuperIOSmartGuardPWMControl, i))
                WarningLog("error adding PWM fan control");
            
            snprintf(key,5,KEY_FORMAT_FAN_START_TEMP,i);
            if (!addSensor(key, TYPE_UI8, 1, (SuperIOSensorGroup)kSuperIOSmartGuardTempFanStart, i))
                WarningLog("error adding start temp fan control");
            
            snprintf(key,5,KEY_FORMAT_FAN_OFF_TEMP,i);
            if (!addSensor(key, TYPE_UI8, 1, (SuperIOSensorGroup)kSuperIOSmartGuardTempFanStop, i))
                WarningLog("error adding stop temp fan control");
            
            snprintf(key,5,KEY_FORMAT_FAN_FULL_TEMP,i);
            if (!addSensor(key, TYPE_UI8, 1, (SuperIOSensorGroup)kSuperIOSmartGuardTempFanFullOn, i))
                WarningLog("error adding full speed temp fan control");
            
            snprintf(key,5,KEY_FORMAT_FAN_START_PWM,i);
            if (!addSensor(key, TYPE_UI8, 1, (SuperIOSensorGroup)kSuperIOSmartGuardPWMStart, i))
                WarningLog("error adding start PWM fan control");
            
            snprintf(key,5,KEY_FORMAT_FAN_TEMP_DELTA,i);
            if (!addSensor(key, TYPE_UI8, 1, (SuperIOSensorGroup)kSuperIOSmartGuardTempFanFullOff, i))
                WarningLog("error adding temp full off fan control");
            
            snprintf(key,5,KEY_FORMAT_FAN_CONTROL,i);
            if (!addSensor(key, TYPE_UI8, 1, (SuperIOSensorGroup)kSuperIOSmartGuardTempFanControl, i))
                WarningLog("error adding register fan control");
        }
	}
	
	return true;	
}

int IT87x::getPortsCount()
{
    return 1;
}


SuperIOSensor * IT87x::addSensor(const char* name, const char* type, unsigned char size, SuperIOSensorGroup group, unsigned long index)
{
	if (NULL != getSensor(name))
		return 0;
	
	if (SuperIOSensor *sensor = IT87xSensor::withOwner(this, name, type, size, group, index))
		if (sensors->setObject(sensor))
			if(kIOReturnSuccess == fakeSMC->callPlatformFunction(kFakeSMCAddKeyHandler, false, (void *)name, (void *)type, (void *)size, (void *)this))
				return sensor;
	
	return 0;
}



IOReturn IT87x::callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 )
{
	if (functionName->isEqualTo(kFakeSMCGetValueCallback)) {
		const char* name = (const char*)param1;
		void * data = param2;
		//UInt32 size = (UInt64)param3;
		
        
		if (name && data)
			if (SuperIOSensor * sensor = getSensor(name)) {
				UInt16 value = sensor->getValue();
				
				bcopy(&value, data, 2);
				
				return kIOReturnSuccess;
			}
		
		return kIOReturnBadArgument;
	}
    
	if (functionName->isEqualTo(kFakeSMCSetValueCallback)) {
		const char* name = (const char*)param1;
		void * data = param2;
		//UInt32 size = (UInt64)param3;
		
        
		if (name && data)
			if (IT87xSensor *sensor = OSDynamicCast(IT87xSensor, getSensor(name))) {
				UInt16 value;
                bcopy(data, &value, 2);
				sensor->setValue(value);
				
				
				return kIOReturnSuccess;
			}
		
		return kIOReturnBadArgument;
	}    
    
	return super::callPlatformFunction(functionName, waitForFunction, param1, param2, param3, param4);
}
