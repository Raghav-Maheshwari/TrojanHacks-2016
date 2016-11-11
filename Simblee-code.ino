#include <SimbleeForMobile.h>
#include <SimbleeBLE.h>
#include <Wire.h>
#include "laura.h"

// pin 3 on the RGB shield is the red led
// (can be turned on/off from the iPhone app)
int led = 3;
int led_blue = 2;

// pin 5 on the RGB shield is button 1
// (button press will be shown on the iPhone app)
int button = 5;

int buttonState = 0;


//LAURA variables
uint32_t lastMeasure = 0;
uint8_t measureInterval = 250;


void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println("Starting I2C");
  printf("Laura Module Chip ID: 0x%04X\n", lauraIDRead());
  Wire.begin();
  delay(10);
  Serial.println("Configuring Laura Module");

  // led turned on/off from the iPhone app
  pinMode(led, OUTPUT);
  pinMode(led_blue, OUTPUT);


  SimbleeForMobile.advertisementData = "ledbtn";
  SimbleeForMobile.begin();

  // configure, initialize and wake up LAURA module
  lauraConfig();
  lauraInit();
  lauraOn();
  Serial.println("Configuration Complete!");
  delay(350);
}



void loop() {
  int data, errorCode, distMM, pixels, avgAmplitude, refreshRateErr;
  int count = 0;
  Wire.begin();

  // check if we've exceeded the measurement interval
  if (millis() - lastMeasure >= measureInterval) {
    printf("Displaying one Laura measurement.\n");
    errorCode = distMM = 0;

    // get the measurement
    data = lauraMeasure(&errorCode, &distMM, &pixels, &avgAmplitude, &refreshRateErr);

    if (data > 0) {
      printf("Distance in MM: %d mm    Error: %-2d    Pixels: %-2d    Avg. Amp.: %-4d    Refresh: %-2d\n", distMM, errorCode, pixels, avgAmplitude, refreshRateErr);
      gitSimbleeBLE.sendInt(distMM);
    }


    // set the lastMeasure to our current time
    lastMeasure = millis();
  }

  SimbleeForMobile.process();

}

void ui()
{

}


// read the chip ID from the LAURA
int lauraIDRead() {
  int data = i2c_read_word(addrLAURA, addr_ADDA_CHIP_ID);
  return data;
}

// set LAURA configuration
void lauraConfig() {
  i2c_write_word(addrLAURA, addr_ADDA_ICSR, 0x0004); // drive INT_PAD high
  delay(2);
  i2c_write_word(addrLAURA, addr_ADDA_RFSH, 0xE100); // command configuration register A
  delay(2);
  i2c_write_word(addrLAURA, addr_ADDA_AUXCTRL, 0x30FF); // command configuration register B
  delay(2);
  i2c_write_word(addrLAURA, addr_ADDA_HFCFG0, 0x07D0); // command configuration register D
  delay(2);
  i2c_write_word(addrLAURA, addr_ADDA_HFCFG1, 0xE002); // command configuration register 1
  delay(2);
  i2c_write_word(addrLAURA, addr_ADDA_HFCFG2, 0xA041); // command configuration register 2
  delay(2);
  i2c_write_word(addrLAURA, addr_ADDA_HFCFG3, 0x4580); // command configuration register 3
  delay(2);
}

// initialize LAURA sensor
int lauraInit() {
  int data;
  int maxTries = 20;
  int count = 0;

  // read Interrupt Control and Status Register to determine reset status
  do {
    data = i2c_read_byte(addrLAURA, addr_ADDA_ICSR);

    if (data < 0) {
      printf("Read Interrupt Control Failed (%d)\n", data);
      return -1;
    }

    delay(100);
    count++;

    if (!(count % 20)) {
      printf("Waiting for chip to come out of reset (%d)\n", count);
    }
  } while ((count < maxTries) && ((data & ICSR_INT_RESET) != ICSR_INT_RESET));

  // check for and clear any outstanding IRQs
  if (data & (ICSR_INT_RESULT | ICSR_INT_M2H_REQ | ICSR_INT_M2H_ACK | ICSR_INT_RESET)) {
    printf("Laura has outstanding IRQ (0x%X)\n", data);

    // clear all interrupts
    i2c_write_byte(addrLAURA, addr_ADDA_IER, IER_CLEAR_MASK);
  }

  i2c_write_byte(addrLAURA, addr_ADDA_PMU_CFG_UP, 0x06); // MCPU_INIT_HOLD_OFF_N | PATCH_CODE_RETAIN
  i2c_write_byte(addrLAURA, addr_ADDA_IER, IER_ENABLE_MASK); // enable interrupts
  i2c_write_byte(addrLAURA, addr_I2C_ACC_CFG, I2C_AUTO_INC | I2C_AUTO_INC_DATAP | I2C_MCPU_DEBUG_ACC);
  i2c_write_byte(addrLAURA, addr_ADDA_ICSR, ICSR_INT0_CLR_RES_N_ICSR);
}

// enable LAURA sensor
void lauraOn() {
  i2c_write_byte(addrLAURA, addr_ADDA_PMU_CFG_UP, 0x06);

  // wake up MCPU to ON mode
  i2c_write_byte(addrLAURA, addr_ADDA_CMD, CMD_START_MEASURE | CMD_GO_STANDBY | CMD_VALID_MASK);
  delay(100);
}

// take a measurement from the LAURA sensor
int lauraMeasure(int *errorCode, int *distMM, int *pixels, int *avgAmplitude, int *refreshRateErr) {
  int data;
  int count = 0;
  int maxTries = 1000;

  // wait for the LAURA to wake
  while (!isLauraOn()) {
    delay(5);
  }

  // take a single measurement
  i2c_write_byte(addrLAURA, addr_ADDA_CMD, CMD_VALID_MASK | CMD_SINGLE_MEASURE);
  delay(1);

  // wait for the measurement to be ready or timeout
  count = 0;

  do {
    // check for the result interrupt
    data = i2c_read_byte(addrLAURA, addr_ADDA_ICSR);
    count++;

    if ((data & ICSR_INT_RESULT) != ICSR_INT_RESULT) {
      delay(1);
    }
  } while ((count < maxTries) && ((data & ICSR_INT_RESULT) != ICSR_INT_RESULT));

  if (count >= maxTries) {
    printf("Max tries exceeded (%d).\n", count);
    return -1;
  }

  // get the measurement value
  data = i2c_read_word(addrLAURA, addr_ADDA_RSLTR);
  *distMM = (data & 0x1FFC) >> 2;
  *errorCode = (data & 0x6000) >> 13;

  if (data & 0x8000) {
    // get the confidence value of the last measurement value
    data = i2c_read_word(addrLAURA, addr_ADDA_RSL_CON);

    if (data < 0) {
      printf("addr_ADDA_RSLTR failed (%d).\n", data);
      return -1;
    }

    *pixels = (data & 0x000F) >> 0;
    *avgAmplitude = (data & 0x7FF0) >> 4;
    *refreshRateErr = (data & 0x8000) >> 15;
    return *distMM;
  }

  return -1;
}

// check if the LAURA sensor is awake
int isLauraOn() {
  int data, mcpuStatus;

  // read the LAURA device status
  data = i2c_read_byte(addrLAURA, addr_ADDA_DSTATUS);

  if (data < 0) {
    printf("Read addr_ADDA_DSTATUS failed (%d).\n", data);
    return -1;
  }

  // determine MCPU status
  mcpuStatus = data & DSTATUS_PM_STATUS_MASK;

  if (mcpuStatus == DSTATUS_PM_STATUS_MCPU_ON) {
    return 1;
  } else {
    return 0;
  }
}

// read byte via I2C
byte i2c_read_byte(byte i2cAddress, byte regAddress) {
  byte data;

  // begin the transmission with the LAURA sensor
  Wire.beginTransmission(i2cAddress);

  // set the register address to read from
  Wire.write(regAddress);
  Wire.endTransmission();

  // request 1 byte read from the register
  Wire.requestFrom(i2cAddress, 1);

  // read the received data
  data = Wire.read();
  Wire.endTransmission();
  return data;
}

// read 2 bytes via I2C
int i2c_read_word(byte i2cAddress, byte regAddress) {
  int data;
  int byteLow, byteHigh;

  // begin the transmission with the LAURA sensor
  Wire.beginTransmission(i2cAddress);

  // set the register address to read from
  Wire.write(regAddress);
  Wire.endTransmission();

  // request 2 bytes read from the register
  Wire.requestFrom(i2cAddress, 2);

  // read the received data into high and low bytes
  byteLow = Wire.read();
  byteHigh = Wire.read();
  Wire.endTransmission();
  data = (byteHigh << 8) | byteLow;
  return data;
}

// write byte via I2C
void i2c_write_byte(byte i2cAddress, byte regAddress, byte data) {
  // begin transmission with the LAURA sensor
  Wire.beginTransmission(i2cAddress);

  // set the register address to write to
  Wire.write(regAddress);

  // write the byte to the register
  Wire.write(data);
  Wire.endTransmission();
}

// write 2 bytes via I2C
void i2c_write_word(byte i2cAddress, byte regAddress, int data) {
  byte byteLow, byteHigh;
  byteHigh = (data >> 8) & 0xFF;
  byteLow = data & 0xFF;

  // begin the transmission with the LAURA sensor
  Wire.beginTransmission(i2cAddress);

  // set the register address to write to
  Wire.write(regAddress);

  // write the data to the register
  Wire.write(byteLow);
  Wire.write(byteHigh);
  Wire.endTransmission();
}

