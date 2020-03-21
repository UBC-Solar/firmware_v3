

#include "LTC6811"
//assumption: a single reading from each slave board's LTC6811

void thermistor_mux(){
  //start I2C (STCOMM)
  LTC_sendCmd(LTC_CMD_STCOMM);

  //a loop that reads for each of the 12 thermistor to the mux
  // for(i = 1; i < 13; i++){
  //   thermistor_reading(i)
  // }
}


void thermistor_reading(uint8_t GPIO1voltages[2]) {
  uint8_t GPIO123voltages[NUM_LTC][6];
  unit8_t RX_message[1] = {0};

  //start conversion
  //ADAX
  LTC_sendCmd(LTC_CMD_ADAX_GPIO1);

  //wait for Conversion to be ready.
  do {
      HAL_SPI_Receive(LTC_SPI_handle, &RX_message, 1, 250);
  } while (!RX_message);

  //retrieve register readings
  //RDAUXA
  LTC_readRegisterGroup(LTC_CMD_RDAUXA, GPIO123voltages[0]);


  //volts2temp(GPIO123voltages[PLACEHOLDER]);
  //there will probably be a better placement for the above func

  //output reading by assigning to pointed array the first two bytes of GPIO123voltages
  for(int board = 1; board <= NUM_LTC; board++) {
    for(int i = 0; i < 2; i++) {
      GPIO1voltages[i] = GPIO123voltages[board][i];
    }
  }

}


void volts2temp () {
  const double beta = 3435.0;
  const double max_ADC = 1023.0; //assuming ADC @ 10 bit
  const double room_temp = 298.15;

}
