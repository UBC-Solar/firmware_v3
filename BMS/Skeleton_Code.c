//the main program that will run the BMS of V3 (that I will call Sol Crusher, named after its home university)

//TODO: review how compliers optimise code and how to avoid optimisations from getting rid of needed code. it might be a keyword or something, like "VOLATILE".


//include statements

//function prototypes
void measure_current();
void measure_voltage();
void init_NVIC();
void init_whatevs();


//Global constants/variables (should be in header files, move later)
//TODO: learn how to declare global constants, variables, and volatility
#define VOLTAGE_THRESHHOLD_HIGH 1337
#define VOLTAGE_THRESHHOLD_LOW 1337
#define OVERCURRENT_THRESHOLD 1337
#define TEMPERATURE_HIGH_THRESHOLD 1337
#define TEMPERATURE_LOW_THRESHOLD 1337

global int GLOBAL_interupt

/*
Main function:
	- intended to be the most readable code, so that any new coder can pickup the code quickly.
	- should contain as little variable assignment as possible
*/
main(){
//variable declaration

//interrupt protocols

//initialisation
/*
Note that these functions are not finalised; init_PERIPHERALS() for example is just there incase we need to configure things other than NIVC, PINS, and ADC
*/
init_NVIC();
init_GPIO();
init_ADC();
init_CAN();
init_SPI();
init_PERIPHERALS();

//startup checks
	//overvoltage and under voltage
	measure_voltage();
	
	//overcurrent
	measure_current();
		
	//temperature
	measure_temperature();
	
	
//main looping process

	while(GLOBAL_interupt == 0){
		measure_voltage();
		measure_current();
		measure_temperature();
	}

	if(GLOBAL_interupt == 1)
		GLOBAL_interupt = 0;
return 0;
}

void measure_voltage(){
	if(read_voltage() > VOLT_THRESHHOLD_HIGH){
		//TODO: set interrupt flag
	}
	else if(read_voltage() < VOLTAGE_THRESHHOLD_LOW){
		//TODO: set interrupt flag
	}
}

void measure_current(){
	int raw_current, final_current;
	
	//pull reading from registers
	raw_current = read_current();
	
	//an immediate comparison, for safety
	//TODO: figure out value of raw overcurrent
	if(raw_current >= OVERCURRENT_THRESHOLD){
		//TODO: set interrupt 
	}
	else{
	final_current = calc_current(raw_current);
	
	//set global var for CAN functions to send
	DISPLAY_current = final current;
	}	
	return final_current;
}

//TODO: settle on function heirarchy tree
/*
main(): the most readable, comprehensive code. should be easily read by any new coder introduced to the project.
	measure_x(): handles GLOBAL variable assignment and setting interrupt flags. ensures fast safety response to measurements exceeding safe thresholds.
		read_x(): handles hardware info pulls
			SPI_read_x(): sends request through SPI for x measurement
			PEC_decode(): decodes the incoming SPI message
		calc_x(): handles calculations
			CAN_write_x(): Sends one-way CAN message with accurate measurement
*/