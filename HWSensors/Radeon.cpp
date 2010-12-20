/*
 *  Radeon.cpp
 *  HWSensors
 *
 *  Created by Sergey on 20.12.10.
 *  Copyright 2010 Slice. All rights reserved.
 *
 */

#include "Radeon.h"
#include "FakeSMC.h"

#define kGenericPCIDevice "IOPCIDevice"
#define kTimeoutMSecs 1000
#define fVendor "vendor-id"
#define fDevice "device-id"
#define kIOPCIConfigBaseAddress0 0x10

#define INVID8(offset) (mmio_base[offset])
#define INVID16(offset) OSReadLittleInt16((mmio_base), offset)
#define INVID(offset) OSReadLittleInt32((mmio_base), offset)
#define OUTVID(offset,val) OSWriteLittleInt32((mmio_base), offset, val)

#define Debug FALSE

#define LogPrefix "RadeonMonitor: "
#define DebugLog(string, args...)	do { if (Debug) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

#define super IOService
OSDefineMetaClassAndStructors(RadeonMonitor, IOService)

bool RadeonMonitor::addSensor(const char* key, const char* type, unsigned char size, int index)
{
	if (kIOReturnSuccess == fakeSMC->callPlatformFunction(kFakeSMCAddKeyHandler, false, (void *)key, (void *)type, (void *)size, (void *)this))
		return sensors->setObject(key, OSNumber::withNumber(index, 32));	
	return false;
}

IOService* RadeonMonitor::probe(IOService *provider, SInt32 *score)
{
	if (super::probe(provider, score) != this) return 0;
	UInt32 vendor_id, device_id, class;
	bool ret = 0;
	if (OSDictionary * dictionary = serviceMatching(kGenericPCIDevice)) {
		if (OSIterator * iterator = getMatchingServices(dictionary)) {
			
			IOPCIDevice* device = 0;
			
			while (device = OSDynamicCast(IOPCIDevice, iterator->getNextObject())) {
				OSData *data = OSDynamicCast(OSData, device->getProperty("vendor-id"));
				if (data)
					vendor_id = *(UInt32*)data->getBytesNoCopy();
				
				data = OSDynamicCast(OSData, device->getProperty("device-id"));				
				if (data)
					device_id = *(UInt32*)data->getBytesNoCopy();
				
				data = OSDynamicCast(OSData, device->getProperty("class"));				
				if (data)
					class = *(UInt32*)data->getBytesNoCopy();
				
				if ((vendor_id==0x1002) && (class = 0x03000000)) {
					InfoLog("found %lx Radeon chip", (long unsigned int)device_id);
					VCard = device;
					ret = 1; //TODO - count a number of cards
				}
				else {
					InfoLog("ATI Radeon not found!");
				}
			}
		}
	}
	if(ret)
		return this;
	else return 0;
}

bool RadeonMonitor::start(IOService * provider)
{
	if (!provider || !super::start(provider)) return false;
	
	if (!(fakeSMC = waitForService(serviceMatching(kFakeSMCService)))) {
		WarningLog("Can't locate fake SMC device, kext will not load");
		return false;
	}
	
	IOMemoryDescriptor *		theDescriptor;
	IOPhysicalAddress bar = (IOPhysicalAddress)((VCard->configRead32(kMCHBAR)) & ~0xf);
	DebugLog("Fx3100: register space=%08lx\n", (long unsigned int)bar);
	theDescriptor = IOMemoryDescriptor::withPhysicalAddress (bar, 0x2000, kIODirectionOutIn); // | kIOMapInhibitCache);
	if(theDescriptor != NULL)
	{
		mmio = theDescriptor->map();
		if(mmio != NULL)
		{
			mmio_base = (volatile UInt8 *)mmio->getVirtualAddress();
#if DEBUG				
			DebugLog(" MCHBAR mapped\n");
			for (int i=0; i<0x2f; i +=16) {
				DebugLog("%04lx: ", (long unsigned int)i+0x1000);
				for (int j=0; j<16; j += 1) {
					DebugLog("%02lx ", (long unsigned int)INVID8(i+j+0x1000));
				}
				DebugLog("\n");
			}
#endif				
		}
		else
		{
			InfoLog(" MCHBAR failed to map\n");
			return -1;
		}			
	}	
	
	char name[5];
	//try to find empty key
	for (int i = 0; i < 0x10; i++) {
		
		snprintf(name, 5, KEY_FORMAT_GPU_DIODE_TEMPERATURE, i); 
		
		UInt8 length = 0;
		void * data = 0;
		
		IOReturn result = fakeSMC->callPlatformFunction(kFakeSMCGetKeyValue, true, (void *)name, (void *)&length, (void *)&data, 0);
		
		if (kIOReturnSuccess == result) {
			continue;
		}
		if (addSensor(name, TYPE_SP78, 2, i)) {
			numCard = i;
			break;
		}
	}
	
	if (kIOReturnSuccess != fakeSMC->callPlatformFunction(kFakeSMCAddKeyHandler, false, (void *)name, (void *)TYPE_SP78, (void *)2, this)) {
		WarningLog("Can't add key to fake SMC device, kext will not load");
		return false;
	}
	
	return true;	
}


bool RadeonMonitor::init(OSDictionary *properties)
{
    if (!super::init(properties))
		return false;
	
	if (!(sensors = OSDictionary::withCapacity(0)))
		return false;
	
	return true;
}

void RadeonMonitor::stop (IOService* provider)
{
	sensors->flushCollection();
	
	super::stop(provider);
}

void RadeonMonitor::free ()
{
	sensors->release();
	
	super::free();
}

IOReturn RadeonMonitor::callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 )
{
	UInt16 t;
	
	if (functionName->isEqualTo(kFakeSMCGetValueCallback)) {
		const char* name = (const char*)param1;
		void* data = param2;
		
		if (name && data) {
			if (OSNumber *number = OSDynamicCast(OSNumber, sensors->getObject(name))) {				
				UInt32 index = number->unsigned16BitValue();
				if (index != numCard) {
					return kIOReturnBadArgument;
				}
			}
			short value;
			if (mmio_base) {
				OUTVID(TIC1, 3);
				//		if ((INVID16(TSC1) & (1<<15)) && !(INVID16(TSC1) & (1<<8)))//enabled and ready
				for (int i=0; i<1000; i++) {  //attempts to ready
					
					if (INVID16(TSS1) & (1<<10))   //valid?
						break;
					IOSleep(10);
				}				
				value = INVID8(TR1);
			}				
			
			t = 140 - value;
			bcopy(&t, data, 2);
			
			return kIOReturnSuccess;
		}
		
		//DebugLog("bad argument key name or data");
		
		return kIOReturnBadArgument;
	}
	
	return super::callPlatformFunction(functionName, waitForFunction, param1, param2, param3, param4);
}
