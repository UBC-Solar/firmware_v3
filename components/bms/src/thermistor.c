

#include "LTC6811"
//assumption: a single reading from each slave board's LTC6811


/*
Function name: thermistor_mux
Purpose: read all 12 thermistor battVoltages
input: internal functions pull readings from hardware registers
output: uint8_t thermistor_volt_array[12][2], 12 sets of data of 2 bytes
*/
void thermistor_mux(uint8_t thermistor_volt_array[12][2]){


  //start I2C (STCOMM)
  LTC_sendCmd(LTC_CMD_STCOMM);

  /*
  steps:
    1) control MUX
      1.1) WRCOMM: write to COMM register:
        1.1.1) ICOMn[3:0] = I2C_START
        1.1.2) D0[7:4] = 0b0000, (it's MSB)
        1.1.3) D0[3:0] = MUX_Sn  (it's MSB)
        1.1.4) FCOMn[3:0] = I2C_MASTER_NACK_STOP
      1.2) STCOMM: send the COMM register content
        1.2.1) read acknowledge
    2) read GPIO1 for thermistor reading
  */




  //a loop that reads for each of the 12 thermistor to the mux
  // for(i = 1; i < 13; i++){
  //   thermistor_reading(i)
  // }
}

/*
Function name: thermistor_reading
Purpose: get voltage readings of thermistors and output into a register
input: internal functions pull readings from hardware registers
output: uint8_t GPIO1voltages[2], two bytes long because that's how long it is.
*/
void thermistor_reading(uint8_t GPIO1voltages[2]) {
  uint8_t GPIO123voltages[NUM_LTC][6];
  unit8_t RX_message[1] = {0};

  //start conversion
  //ADAX, ADc AuXillary start-conversion command
  LTC_sendCmd(LTC_CMD_ADAX_GPIO1);

  //wait for Conversion to be ready.
  do {
      HAL_SPI_Receive(LTC_SPI_handle, &RX_message, 1, 250);
  } while (!RX_message);

  //retrieve register readings
  //RDAUXA
  //uses RDAXA to
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


void volts2temp (ADC_val) {
  const double beta = 3435.0;
  const double max_ADC = 65535.0; //assuming ADC @ 16 bit
  const double room_temp = 298.15;
  const double R_balance = 10000.0; //from LTC6811 datasheet p.85. note: this doesn't account for tolerance. to be exact, measure the 10k resistor with a multimeter
  const double R_room_temp = 10000.0; //resistance at room temperature (25C)
  //const int sample_num = 10;
  double R_therm = 0;
  double temp_kelvin = 0;
  double temp_celsius = 0;
  //double adc_average = 0; //we will do multiple measurements and average them to eliminate errors
  //int adc_samples[sample_num];

  R_therm = R_balance * ((max_ADC / ADC_val)-1);
  temp_kelvin = (beta * room_temp) / (beta + (room_temp * log(R_therm/R_room_temp)));
  temp_celsius = temp_kelvin - 273.15;
}
