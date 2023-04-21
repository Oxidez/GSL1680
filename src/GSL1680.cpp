// Include needed library
#include <stdint.h>
#include <Wire.h>
#include <avr/pgmspace.h>
#include <GSL1680.h>
#include <GSL_FW.h>

// Registry variables
uint8_t addr = 0x40;
uint8_t dataReg = 0x80;

#define SERIAL_ERROR if(GSL1680_DEBUG_ERROR)Serial
#define SERIAL_INFORMATION if(GSL1680_DEBUG_INFO)Serial
// Variable needed for writing the firmware in upper memory on Arduino MEGA
#define GET_FAR_ADDRESS(var) \
({ \
uint_farptr_t tmp; \
\
__asm__ __volatile__( \
\
"ldi %A0, lo8(%1)" "\n\t" \
"ldi %B0, hi8(%1)" "\n\t" \
"ldi %C0, hh8(%1)" "\n\t" \
"clr %D0" "\n\t" \
: \
"=d" (tmp) \
: \
"p" (&(var)) \
); \
tmp; \
})

GSL1680::GSL1680() {
    GSL1680_DEBUG_ERROR = true;
    GSL1680_DEBUG_INFO = true;
}

GSL1680::GSL1680(bool error, bool info) {
    GSL1680_DEBUG_ERROR = error;
    GSL1680_DEBUG_INFO = info;
}

void GSL1680::begin(uint8_t WAKE, uint8_t INTRPT) {
    SERIAL_INFORMATION.println("Boot up starting");
    pinMode(WAKE, OUTPUT);          
    digitalWrite(WAKE, LOW);        
    pinMode(INTRPT, INPUT_PULLUP);  
    delay(200);
    SERIAL_INFORMATION.println("Toggle WAKE pin");
	digitalWrite(WAKE, HIGH);
	delay(20);
	digitalWrite(WAKE, LOW);
	delay(20);
	digitalWrite(WAKE, HIGH);
	delay(20);
    Wire.begin();
    // GSL1680 driver starting sequence
	SERIAL_INFORMATION.println("Clear registry");
	clear_reg();
	SERIAL_INFORMATION.println("Reset driver");
	reset();
	SERIAL_INFORMATION.println("Loading firmware");
	load_fw();
    SERIAL_INFORMATION.println("Starting driver");
	startchip();
    delay(50);
	// SERIAL_INFORMATION.println("Reset driver");
	// reset();
	// SERIAL_INFORMATION.println("Start driver");
    // startchip();
	SERIAL_INFORMATION.println("Checking memory data");
    check_mem_data(WAKE);
    delay(100);
	SERIAL_INFORMATION.println("Boot up complete");
}

// data_I2C_Write
void GSL1680::data_I2C_Write(uint8_t regAddr, uint8_t *val, uint16_t cnt) {
  uint16_t i=0;
  Wire.beginTransmission(addr);
  Wire.write(regAddr);     // register 0
  for(i=0;i<cnt;i++,val++) //data
  {		
    Wire.write( *val );    // value
  }
  uint8_t retVal = Wire.endTransmission(); 
  if (retVal != 0){
    SERIAL_ERROR.print("I2C write error: ");
	SERIAL_ERROR.print(retVal);
	SERIAL_ERROR.print(""); 
	SERIAL_ERROR.println(regAddr, HEX);
  } 
}

//data_I2C_Read
uint8_t GSL1680::data_I2C_Read( uint8_t regAddr, uint8_t * pBuf, uint8_t len ) {
  Wire.beginTransmission(addr);
  Wire.write( regAddr );  // register 0
  uint8_t retVal = Wire.endTransmission();
  uint8_t returned = Wire.requestFrom(addr, len);    // request 1 bytes from slave device #2
  uint8_t i;
  for (i = 0; (i < len) && Wire.available(); i++)
  {
    pBuf[i] = Wire.read();
  }
  return i;
}

// Clear registry function
void GSL1680::clear_reg() {
	uint8_t regAddr;	
	uint8_t Wrbuf[4] = {0x00};
    uint8_t len=1;
	regAddr = 0xe0;
	Wrbuf[0] = 0x88;
	data_I2C_Write(regAddr, Wrbuf, 1); 	
	delay(1);
	regAddr = 0x80;
	Wrbuf[0] = 0x03;
	data_I2C_Write(regAddr, Wrbuf, 1); 	
	delay(1);
	regAddr = 0xe4;
	Wrbuf[0] = 0x04;
	data_I2C_Write(regAddr, Wrbuf, 1); 	
	delay(1);
	regAddr = 0xe0;
	Wrbuf[0] = 0x00;
	data_I2C_Write(regAddr, Wrbuf, 1); 	
	delay(1);
}

// Reset function
void GSL1680::reset() {
	uint8_t regAddr;	
	uint8_t Wrbuf[4] = {0x00};
	regAddr = 0xe0;
	Wrbuf[0] = 0x88;
    data_I2C_Write(regAddr,Wrbuf, 1);
	delay(1);
	regAddr = 0xe4;
	Wrbuf[0] = 0x04;
    data_I2C_Write(regAddr,Wrbuf, 1);
	delay(1);
	regAddr = 0xbc;
	Wrbuf[0] = 0x00;
	Wrbuf[1] = 0x00;
	Wrbuf[2] = 0x00;
	Wrbuf[3] = 0x00;
    data_I2C_Write(regAddr,Wrbuf, 4);
	delay(1);
}

// Load firmware function
void GSL1680::load_fw() {
#if defined(ARDUINO_AVR_MEGA)
  // Arduino Mega_FW	
	uint8_t regAddr;
	uint8_t Wrbuf[4] = {0x00};
	uint16_t source_line=0;
	uint16_t source_len = sizeof(GSLX680_FW)/sizeof(struct fw_data);
    SERIAL_INFORMATION.print("Firmware size: "); SERIAL_INFORMATION.println(sizeof(GSLX680_FW));
    SERIAL_INFORMATION.print("Line numbers : "); SERIAL_INFORMATION.println(source_len);
	for (source_line = 0; source_line < source_len; source_line++) 
	{   // Write in the upper memory
		regAddr = pgm_read_byte_far(GET_FAR_ADDRESS(GSLX680_FW[0].offset)+source_line*5);
        Wrbuf[0] = (char) (pgm_read_dword_far(GET_FAR_ADDRESS(GSLX680_FW[0].val)+source_line*5) & 0x000000ff);
        Wrbuf[1] = (char) ((pgm_read_dword_far(GET_FAR_ADDRESS(GSLX680_FW[0].val)+source_line*5) & 0x0000ff00) >> 8);
        Wrbuf[2] = (char) ((pgm_read_dword_far(GET_FAR_ADDRESS(GSLX680_FW[0].val)+source_line*5) & 0x00ff0000) >> 16);
        Wrbuf[3] = (char) ((pgm_read_dword_far(GET_FAR_ADDRESS(GSLX680_FW[0].val)+source_line*5) & 0xff000000) >> 24);
        // Write in the lower memory
	/*	regAddr = pgm_read_byte_near(GET_FAR_ADDRESS(GSLX680_FW[0].offset)+source_line*5);
        Wrbuf[0] = (char) (pgm_read_dword_near(GET_FAR_ADDRESS(GSLX680_FW[0].val)+source_line*5) & 0x000000ff);
        Wrbuf[1] = (char) ((pgm_read_dword_near(GET_FAR_ADDRESS(GSLX680_FW[0].val)+source_line*5) & 0x0000ff00) >> 8);
        Wrbuf[2] = (char) ((pgm_read_dword_near(GET_FAR_ADDRESS(GSLX680_FW[0].val)+source_line*5) & 0x00ff0000) >> 16);
        Wrbuf[3] = (char) ((pgm_read_dword_near(GET_FAR_ADDRESS(GSLX680_FW[0].val)+source_line*5) & 0xff000000) >> 24);
    */
		data_I2C_Write(regAddr,Wrbuf, 4);
	}
#elif defined(ARDUINO_SAM_DUE)
// Arduino DUE
	uint8_t regAddr;
	uint8_t Wrbuf[4]= {0x00};
	uint16_t source_line=0;
	uint16_t source_len = sizeof(GSLX680_FW)/sizeof(struct fw_data);
    SERIAL_INFORMATION.print("Firmware size: "); SERIAL_INFORMATION.println(sizeof(GSLX680_FW));
    SERIAL_INFORMATION.print("Line numbers : "); SERIAL_INFORMATION.println(source_len);
	for (source_line=0; source_line<source_len; source_line++)
	{
		regAddr = GSLX680_FW[source_line].offset;
		Wrbuf[0] = (char)(GSLX680_FW[source_line].val & 0x000000ff);
		Wrbuf[1] = (char)((GSLX680_FW[source_line].val & 0x0000ff00) >> 8);
		Wrbuf[2] = (char)((GSLX680_FW[source_line].val & 0x00ff0000) >> 16);
		Wrbuf[3] = (char)((GSLX680_FW[source_line].val & 0xff000000) >> 24);
		data_I2C_Write(regAddr,Wrbuf, 4);
	} 
#elif defined(ESP32)
// ESP32
	uint8_t regAddr;
	uint8_t Wrbuf[4]= {0x00};
	uint16_t source_line=0;
	uint16_t source_len = sizeof(GSLX680_FW)/sizeof(struct fw_data);
    SERIAL_INFORMATION.print("Firmware size: "); SERIAL_INFORMATION.println(sizeof(GSLX680_FW));
    SERIAL_INFORMATION.print("Line numbers : "); SERIAL_INFORMATION.println(source_len);	
	for (source_line=0; source_line<source_len; source_line++)
	{
		regAddr = GSLX680_FW[source_line].offset;
		Wrbuf[0] = (char)(GSLX680_FW[source_line].val & 0x000000ff);
		Wrbuf[1] = (char)((GSLX680_FW[source_line].val & 0x0000ff00) >> 8);
		Wrbuf[2] = (char)((GSLX680_FW[source_line].val & 0x00ff0000) >> 16);
		Wrbuf[3] = (char)((GSLX680_FW[source_line].val & 0xff000000) >> 24);
		data_I2C_Write(regAddr,Wrbuf, 4);
	}   
#else
// Board unknown or clone, we use same method of loading the firmware as for Arduino MEGA
	uint8_t regAddr;
	uint8_t Wrbuf[4] = {0x00};
	uint16_t source_line=0;
	uint16_t source_len = sizeof(GSLX680_FW)/sizeof(struct fw_data);
    SERIAL_INFORMATION.print("Firmware size: "); SERIAL_INFORMATION.println(sizeof(GSLX680_FW));
    SERIAL_INFORMATION.print("Line numbers : "); SERIAL_INFORMATION.println(source_len);
	for (source_line = 0; source_line < source_len; source_line++) 
	{   // Write in the upper memory
		regAddr = pgm_read_byte_far(GET_FAR_ADDRESS(GSLX680_FW[0].offset)+source_line*5);
        Wrbuf[0] = (char) (pgm_read_dword_far(GET_FAR_ADDRESS(GSLX680_FW[0].val)+source_line*5) & 0x000000ff);
        Wrbuf[1] = (char) ((pgm_read_dword_far(GET_FAR_ADDRESS(GSLX680_FW[0].val)+source_line*5) & 0x0000ff00) >> 8);
        Wrbuf[2] = (char) ((pgm_read_dword_far(GET_FAR_ADDRESS(GSLX680_FW[0].val)+source_line*5) & 0x00ff0000) >> 16);
        Wrbuf[3] = (char) ((pgm_read_dword_far(GET_FAR_ADDRESS(GSLX680_FW[0].val)+source_line*5) & 0xff000000) >> 24);
        // Write in the lower memory
	/*	regAddr = pgm_read_byte_near(GET_FAR_ADDRESS(GSLX680_FW[0].offset)+source_line*5);
        Wrbuf[0] = (char) (pgm_read_dword_near(GET_FAR_ADDRESS(GSLX680_FW[0].val)+source_line*5) & 0x000000ff);
        Wrbuf[1] = (char) ((pgm_read_dword_near(GET_FAR_ADDRESS(GSLX680_FW[0].val)+source_line*5) & 0x0000ff00) >> 8);
        Wrbuf[2] = (char) ((pgm_read_dword_near(GET_FAR_ADDRESS(GSLX680_FW[0].val)+source_line*5) & 0x00ff0000) >> 16);
        Wrbuf[3] = (char) ((pgm_read_dword_near(GET_FAR_ADDRESS(GSLX680_FW[0].val)+source_line*5) & 0xff000000) >> 24); */   
		data_I2C_Write(regAddr,Wrbuf, 4);
	}
#endif	
}

// Start driver function
void GSL1680::startchip() {
	uint8_t Wrbuf[4] = {0x00};
	Wrbuf[0] = 0x00;
    data_I2C_Write(0xe0,Wrbuf, 1);
}

// Check memory data function
void GSL1680::check_mem_data(uint8_t WAKE) {
	uint8_t write_buf;
	uint8_t read_buf[4]  = {0};
	delay(30);
	data_I2C_Read(0xb0, read_buf, 4);
	if((read_buf[3] != 0x5a) & (read_buf[2] != 0x5a) & ( read_buf[1] != 0x5a) & ( read_buf[0] != 0x5a)) { 
        SERIAL_INFORMATION.println("Driver initialization failure.  Restarting!");
		delay(1000);
		SERIAL_INFORMATION.println("Toggle Wake");
		digitalWrite(WAKE, LOW ); 
		delay(20);		
		digitalWrite(WAKE, HIGH );
		delay(20);	
    SERIAL_INFORMATION.println("Clear registry");
 	clear_reg();
	SERIAL_INFORMATION.println("Reset driver");
	reset();
	SERIAL_INFORMATION.println("Loading firmware");
	load_fw();
	SERIAL_INFORMATION.println("Starting driver");
	startchip();
	// reset();
	// startchip(); 
    if((read_buf[3] != 0x5a) & (read_buf[2] != 0x5a) & ( read_buf[1] != 0x5a) & ( read_buf[0] != 0x5a)) {     
	   SERIAL_INFORMATION.println("Driver initialization failure.  Restarting!");
       while(1);
     }
	} else {    
      SERIAL_INFORMATION.println("Driver started");  
    }
}

//Read data from touch event
uint8_t GSL1680::dataread() {	uint8_t touch_data[24] = {0}; 
	uint8_t reg = 0x80;
	data_I2C_Read(reg, touch_data, 24);        
    ts_event.fingers=touch_data[0];	
	ts_event.y5 = (uint16_t)(touch_data[23])<<8 | (uint16_t)touch_data[22];
    ts_event.x5 = (uint16_t)(touch_data[21])<<8 | (uint16_t)touch_data[20];		
    ts_event.y4 = (uint16_t)(touch_data[19])<<8 | (uint16_t)touch_data[18];
	ts_event.x4 = (uint16_t)(touch_data[17])<<8 | (uint16_t)touch_data[16];						
	ts_event.y3 = (uint16_t)(touch_data[15])<<8 | (uint16_t)touch_data[14];
	ts_event.x3 = (uint16_t)(touch_data[13])<<8 | (uint16_t)touch_data[12];					  
	ts_event.y2 = (uint16_t)(touch_data[11])<<8 | (uint16_t)touch_data[10];
	ts_event.x2 = (uint16_t)(touch_data[9])<<8 | (uint16_t)touch_data[8];						 
	ts_event.y1 = (uint16_t)(touch_data[7])<<8 | (uint16_t)touch_data[6];
	ts_event.x1 = (uint16_t)(touch_data[5])<<8 | (uint16_t)touch_data[4];				
	return ts_event.fingers;	
}