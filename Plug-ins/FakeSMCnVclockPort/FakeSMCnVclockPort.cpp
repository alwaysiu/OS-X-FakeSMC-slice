/* add your code here */
#include "FakeSMCnVclockPort.h"

#include <stdarg.h>
#include <string.h>
#include <nvclock.h>
#include "backend.h"

OSDefineMetaClassAndStructors(PTnVmon, IOService)

bool isxdigit(char c) {
	if (((c>='0')&&(c<='9'))||((c>='a')&&(c<='f'))||((c>='A')&&(c<='F')))
		return true;
	return false;
}

NVClock nvclock;
NVCard *nv_card = NULL;
IOMemoryMap * nvio;


/*namespace darwin {
	
	
#include "/usr/include/sys/mman.h"
		

		
#define MAXPATHLEN 1024
		
		
		
		int map_mem_card(NVCard* nv_card, void* addr);
		
		
		*/
		
		
		
		
		
		
		void unmap_mem()
    {
    }

    int32_t pciReadLong(unsigned short devbusfn, long offset) {
      return -1;
    }

    int map_mem(const char *dev_name)
    {
      return 0;
    }
/*}*/

int map_mem_card(NVCard* nv_card, void* addr) {
	/* Map the registers of the nVidia chip */
	/* normally pmc is till 0x2000 but extended it for nv40 */
	nv_card->PEXTDEV = (volatile unsigned int*)addr + 0x101000;
	nv_card->PFB     = (volatile unsigned int*)addr + 0x100000;
	nv_card->PMC     = (volatile unsigned int*)addr + 0x000000;
	nv_card->PCIO    = (volatile unsigned char*)addr + 0x601000;
	nv_card->PDISPLAY= (volatile unsigned int*)addr + NV_PDISPLAY_OFFSET;
	nv_card->PRAMDAC = (volatile unsigned int*)addr + 0x680000;
	nv_card->PRAMIN  = (volatile unsigned int*)addr + NV_PRAMIN_OFFSET;
	nv_card->PROM    = (volatile unsigned char*)addr + 0x300000;
	
	/* On Geforce 8xxx cards it appears that the pci config header has been moved */
	if(nv_card->arch & NV5X)
		nv_card->PBUS = (volatile unsigned int*)addr + 0x88000;
	else
		nv_card->PBUS = nv_card->PMC + 0x1800/4;
	
	nv_card->mem_mapped = 1;
	return 1;
}

int PTnVmon::probe_devices()
{
	IOPCIDevice* PTCard=NULL;
	int i=0;
	UInt16 vendor_id;
	IOLog("PTKawainVi: started\n");
	OSData* idKey;
	OSDictionary * iopci = serviceMatching("IOPCIDevice");
	OSString* str;
#if __LP64__
	mach_vm_address_t addr;
	//mach_vm_size_t size;
#else
	vm_address_t addr;
	//vm_size_t size;
#endif
	if (iopci) {
		OSIterator * iterator = getMatchingServices(iopci);
		if (iterator) {
			while (PTCard = OSDynamicCast(IOPCIDevice, iterator->getNextObject())) {
				vendor_id=0;
				str=OSDynamicCast(OSString, PTCard->getProperty("IOName"));
				idKey=OSDynamicCast(OSData, PTCard->getProperty("vendor-id"));
				if (idKey)
					vendor_id=*(UInt32*)idKey->getBytesNoCopy();
				if ((str->isEqualTo("display"))&&(vendor_id==0x10de)){
					PTCard->setMemoryEnable(true);
					nvio = PTCard->mapDeviceMemoryWithIndex(0);		
					addr = (vm_address_t)nvio->getVirtualAddress();
					idKey=OSDynamicCast(OSData, PTCard->getProperty("device-id"));
					if (idKey)
						nvclock.card[i].device_id=*(UInt32*)idKey->getBytesNoCopy();
					IOLog("Vendor ID: %x, Device ID: %x\n", vendor_id, nvclock.card[i].device_id);
					nvclock.card[i].arch = get_gpu_arch(nvclock.card[i].device_id);
					IOLog("Architecture: %x\n", nvclock.card[i].arch);
					nvclock.card[i].number = i;
					nvclock.card[i].card_name = (char*)get_card_name(nvclock.card[i].device_id, &nvclock.card[i].gpu);
					IOLog("%s\n", nvclock.card[i].card_name);
					nvclock.card[i].state = 0;
					
					nvclock.card[i].reg_address = addr;
					map_mem_card( &nvclock.card[i], (void*)addr );
					
					i++;
				}
			}
		}
	}
	if (!i) {
		IOLog("PTKawainVi: No nVidia graphics adapters found\n");
	}	
	
	nvclock.num_cards = i;
	return i;
}


/* On various NV4x cards it is possible to enable additional
 /  pixel and vertex units. On the commandline the user enters
 /  a mask in binary/hexadecimal containing the pipelines to use.
 /  This function convert the hex or binary value to a mask of several
 /  bits that can be passed to the modding functions.
 */
unsigned char nv40_parse_units_mask(char *mask)
{
	int i, check, value=0;
	
	/* Check if binary */
	for(i=0, check=1; mask[i] && check; i++)
		check = ((mask[i] == '0') || (mask[i] == '1')) ? 1 : 0;
	
	if(check)
		value = strtol(mask, (char**)NULL, 2);
	
	/* Check if hex in case the value wasn't binary */
	for(i=0, check=1; mask[i] && check && !value; i++)
	{
		check = isxdigit((unsigned char)mask[i]);
		
		/* strtoll supports both ff and 0xff */
		if(!check && i==1)
			check = (mask[i] == 'x') ? 1 : 0;
	}

	/* Only convert if the value is hex; 10 / 11 are considered binary */
	if(check && !value)
		value = strtol(mask, (char**)NULL, 16);

	if((value > 0) && (value < 256))
		return (unsigned char)value;

	return 0;
}


void unload_nvclock()
{
	//there was a function but it's not needed in a driver
}


IOService*	PTnVmon::probe	(IOService* provider, SInt32* score) {
	IOService* result=IOService::probe(provider, score);
	return result;
}

bool		PTnVmon::start	(IOService* provider) {
	bool result=IOService::start(provider);
	int backend, card_number, deviceid;
	float memclk, nvclk;
	char punit, vunit;

	backend = 0;
	card_number = 0;
	deviceid = 0;
	memclk = 0;
	nvclk = 0;
	punit = 0;
	vunit = 0;
	nvclock.dpy = NULL;
	
	if(!probe_devices())
	{
		char buf[80];
		printf("Error: %s\n", get_error(buf, 80));
		return 0;
	}
	
	/* set the card object to the requested card */
	if(!set_card(card_number))
	{
		char buf[80];
		printf("Error: %s\n", get_error(buf, 80));
		return 0;
	}
	
	/* Check if the card is supported, if not print a message. */
	if(nvclock.card[card_number].gpu == UNKNOWN)
	{
		printf("It seems your card isn't officialy supported in NVClock yet.\n");
		printf("The reason can be that your card is too new.\n");
		printf("If you want to try it anyhow [DANGEROUS], use the option -f to force the setting(s).\n");
		printf("NVClock will then assume your card is a 'normal', it might be dangerous on other cards.\n");
		printf("Also please email the author the pci_id of the card for further investigation.\n[Get that value using the -i option].\n\n");
		return 0;
	}

	
	if(nv_card->caps & (GPU_TEMP_MONITORING))
	{
		tempSensor=new TemperatureSensor("TG0D", "sp78", 2);
		//printf("%s\n", nv_card->card_name);
		printf("=> GPU temperature: %dC\n", nv_card->get_gpu_temp(nv_card->sensor));
		if(nv_card->caps & (BOARD_TEMP_MONITORING))
			printf("=> Board temperature: %dC\n", nv_card->get_board_temp(nv_card->sensor));
		}
		else {
			printf("Error: temperature monitoring isn't supported on your videocard.\n");
		return 0;
	}

	if(nv_card->caps & I2C_FANSPEED_MONITORING)
	{
		int no=GetFNum();
		char key[5];
		snprintf(key, 5, "F%dAc", no);
		fanSensor=new FanSensor(key, "fpe2", 2);
		UpdateFNum(1);
		printf("Current fanspeed: %d RPM\n", 	nv_card->get_i2c_fanspeed_rpm(nv_card->sensor));
	}
	/* Various standard GeforceFX/6 also have some fanspeed monitoring support */
	else if(nv_card->caps & GPU_FANSPEED_MONITORING)
	{
		int no=GetFNum();
		char key[5];
		snprintf(key, 5, "F%dAc", no);
		fanSensor=new FanSensor(key, "fpe2", 2);
		UpdateFNum(1);
		printf("Current fanspeed: %.1f%%\n", nv_card->get_fanspeed());
	}
	return result;
}

bool		PTnVmon::init	(OSDictionary* properties) {
	bool result=IOService::init(properties);
	return result;	
}

void		PTnVmon::free	() {
	IOService::free();
}

void		PTnVmon::stop	(IOService* provider) {
	if(tempSensor)
		delete tempSensor;
	if (fanSensor) {
		delete fanSensor;
		UpdateFNum(-1);
	}
	IOService::stop(provider);
}

void TemperatureSensor::OnKeyRead(const char* key, char* data) {
	if(!set_card(0))
	{
		char buf[80];
		printf("Error: %s\n", get_error(buf, 80));
		return;
	}
	if(nv_card->caps & (GPU_TEMP_MONITORING))
	{
		data[0]=nv_card->get_gpu_temp(nv_card->sensor);
		data[1]=0;
	}
	else {
		printf("Error: temperature monitoring isn't supported on your videocard.\n");
	}
}

void TemperatureSensor::OnKeyWrite(const char* key, char* data) {
	
}

void FanSensor::OnKeyRead(const char* key, char* data) {
	if(!set_card(0))
	{
		char buf[80];
		printf("Error: %s\n", get_error(buf, 80));
		return;
	}
	if(nv_card->caps & I2C_FANSPEED_MONITORING)
	{
		UInt16 rpm=nv_card->get_i2c_fanspeed_rpm(nv_card->sensor);
		data[0]=(rpm<<2)>>8;
		data[1]=(rpm<<2)&0xff;
	}
	/* Various standard GeforceFX/6 also have some fanspeed monitoring support */
	else if(nv_card->caps & GPU_FANSPEED_MONITORING)
	{
		UInt16 rpm=nv_card->get_fanspeed();
		data[0]=(rpm<<2)>>8;
		data[1]=(rpm<<2)&0xff;
	}
}

void FanSensor::OnKeyWrite(const char* key, char* data) {
	
}