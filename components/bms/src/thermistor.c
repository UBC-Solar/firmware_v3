

#include "LTC6811"
//assumption: a single reading from each slave board's LTC6811



/*
Function name: thermistor_series
Purpose: Algorithm for reading thermistors across multiple mux (2 mux's)
input:
  No user defined input.
  Internal functions pull readings from hardware registers
Output:
  uint8_t thermistor_volt_series[2][6], 2 sets of 6 voltages
Internal functions:
  thermistor_mux()
  thermistor_reading()
*/
void thermistor_series(){
  uint_8 COMM1 = 0b0000_0000;
  uint_8 thermistor_volt_series[MUX_CHANNELS][NUM_LTC][ONE_THIRD_OF_COMM] = {0};
    /*
    Algorithm:
      0) Initialize
      1) read mux1
        1.1) set mux_address or COMM1 bits for mux1
        1.2) call thermistor_mux()
        1.3) call thermistor_reading() SIX TIMES
      2) read mux2
        1.1) set mux_address or COMM1 bits for mux2
        1.2) call thermistor_mux()
        1.3) call thermistor_reading() SIX TIMES
      3) done
    */

  //each cycle, the COMM1 address bits increment.
  //this for-loop should only go up to 4 cycles,
  //because there are only 4 possible mux address
  for(int n = 0; n < NUMBER_OF_MUX; n++, COMM1 = COMM1 + 0b0010_0000){
    thermistor_mux(COMM1, thermistor_volt_series);
    }
  }
}


/*
Function name: thermistor_mux
Purpose:
  switch between all 12 channels OF A SINGLE MUX
  and gather the associated themistor reading
input:
  User defined:
    COMM1, the exact address bits
  internal functions pull readings from hardware registers
output: uint8_t thermistor_volt_array[12][2], 12 sets of data of 2 bytes
*/
void thermistor_mux(uint8_t COMM1, uint_8 thermistor_volt_series[MUX_CHANNELS][NUM_LTC][ONE_THIRD_OF_COMM]){

//the mux_message first dimension is redundant, it's more like the "table" indicator
//i.e. MUX_message[0] contains all pointers to all "rows" of all of table 49 in datasheet
//the rows are actually uint8_t variables for sanity sake; 1 table of 6 rows of y sized variables
uint8_t MUX_message[1][6] = {0};
uint8_t MUX_S[8] = {MUX_S0,
                    MUX_S1,
                    MUX_S2,
                    MUX_S3,
                    MUX_S4,
                    MUX_S5,
                    MUX_S6,
                    MUX_S7};

uint8_t COMM0;
//uint8_t COMM1;
uint8_t COMM2;
uint8_t COMM3;
uint8_t COMM4 = 0b0000_0000;
uint8_t COMM5 = 0b0000_0000;

uint8_t NULL_message = 0b0000_0000;

  /* Algorithm of function (Note: this function is for a single mux)
  steps:
    0) known reset state: send to mux the enable signal?????

    1) control MUX with COMM registers
      1.1) WRCOMM: write to COMM register:

        Address half:
        1.1.1) ICOMn[3:0] = I2C_START
        1.1.2) D0[7:4] = 0b0000, (it's MSB)
        1.1.3) D0[3:0] = address stuff (it's MSB)
        1.1.4) FCOMn[3:0] = I2C_ACK (note that we assume ACK instead of NACK because NACK would tell the slave that is the end of the transmission)

        Command half:
        1.1.5) ICOMn[3:0] = I2C_BLANK
        1.1.6) D1[7:4] = 0b0000, (it's MSB)
        1.1.7) D1[3:0] = MUX_Sn  (it's MSB)
        1.1.8) FCOMn[3:0] = I2C_MASTER_NACK_STOP

      1.2) STCOMM: send the COMM register content
        1.2.1) read acknowledge bit???????? skip
        1.2.2) wait for appropriate clock cycles to pass using HAL_SPI_Transmit(NULLxsomething, sweet_nothings, idk)
    2) read GPIO1 for thermistor reading FOR each mux channel
    3) repeat until n = MUX_CHANNELS
  */

  //1.1) write to COMM register
  for(int n = 0; n < MUX_CHANNELS; n++) {
    //Address half of COMM message:
    //1.1.1) ICOMn[3:0] = I2C_START
    //1.1.2) D0[7:4] = 0b0000, (it's MSB)

    //__ICOMM[3:0]_D[7:4]
    COMM0 = 0b0110_1001; // 1001 is a defined number from LTC1380 datasheet p.8

    //1.1.3) D0[3:0] = address stuff (it's MSB)
    //1.1.4) FCOMn[3:0] = I2C_ACK (note that we assume ACK instead of NACK because NACK would tell the slave that is the end of the transmission)
    //COMM1 assigned by the thermistor_series

    MUX_message[0][0] = COMM0;
    MUX_message[0][1] = COMM1;

    //1.1.1) ICOMn[3:0] = I2C_BLANK
    //1.1.6) D1[7:4] = 0b0000, (it's MSB)
    COMM2 = 0b0000_0000;

    //mux channel switching
      //1.1.7) D1[3:0] = MUX_Sn  (it's MSB)
      //1.1.8) FCOMn[3:0] = I2C_MASTER_NACK_STOP
    COMM3 = MUX_S[n] << 4 //bit shifted value of MUX_S[n] == 0bXXXX_0000
    COMM3 = COMM3 | I2C_MASTER_NACK_STOP;
    MUX_message[0][2] = COMM2;
    MUX_message[0][3] = COMM3;

    //redundancy zero-fill for rest of COMM registers
    MUX_message[0][4] = COMM4;
    MUX_message[0][5] = COMM5;

    //sends from STM32 to 6811 for 6811 to send to MUX the I2C command
    LTC_writeRegisterGroup(LTC_CMD_WRCOMM, MUX_message[0]);

    //1.2) STCOMM: send the COMM register content
    LTC_sendCmd(LTC_CMD_STCOMM);


    //timing delays needed for I2C, 3 clock cycles per bit sent
    //can be done by sending null values to an unspecified register
    //or by reading a "safe" register
    HAL_SPI_Transmit(BTM_SPI_handle, &NULL_message, 1, BTM_TIMEOUT_VAL); // BTM_SPI_handle is a global variable, and BTM_TIMEOUT_VAL is a symbolic constant

    //gather thermistor readings
    thermistor_reading(thermistor_volt_series[n][0]);
  }
}

/*
Function name: thermistor_reading
Purpose: get voltage readings of thermistors and output into a register
input: internal functions pull readings from hardware registers
output: uint8_t GPIO1voltages[2], two bytes long because that's how long it is.
*/
void thermistor_reading(uint8_t GPIO1voltages[NUM_LTC][ONE_THIRD_OF_COMM]) {
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
  LTC_readRegisterGroup(LTC_CMD_RDAUXA, GPIO123voltages[0]);

  //output reading by assigning to pointed array the first two bytes of GPIO123voltages
  for(int board = 1; board <= NUM_LTC; board++) {
    for(int i = 0; i < 2; i++) {
      GPIO1voltages[board][i] = GPIO123voltages[board][i];
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

  R_therm = R_balance * ((max_ADC / ADC_val) - 1);
  temp_kelvin = (beta * room_temp) / (beta + (room_temp * log(R_therm/R_room_temp)));
  temp_celsius = temp_kelvin - 273.15;
}
