











enum BTM_Error {

    BTM_OK = 0,

    BTM_ERROR_PEC,

    BTM_ERROR_TIMEOUT,

    BTM_ERROR_SELFTEST,

    BTM_ERROR_HAL,

    BTM_ERROR_HAL_BUSY,

    BTM_ERROR_HAL_TIMEOUT

};











enum BTM_MD_e {

    MD_422HZ_1KHZ = 0x0,

    MD_27KHZ_14KHZ = 0x1,

    MD_7KHZ_3KHZ = 0x2,

    MD_26HZ_2KHZ = 0x3

};





enum BTM_CH_e {

 CH_ALL = 0x0,

 CH_1 = 0x1,

 CH_2 = 0x2,

 CH_3 = 0x3,

 CH_4 = 0x4,

 CH_5 = 0x5,

 CH_6 = 0x6

};





enum BTM_CHG_e {

    CHG_ALL = 0x0,

    CHG_GPIO1_6 = 0x1,

    CHG_GPIO2_7 = 0x2,

    CHG_GPIO3_8 = 0x3,

    CHG_GPIO4_9 = 0x4,

    CHG_GPIO5 = 0x5,

    CHG_VREF2 = 0x6

};





enum BTM_CHST_e {

    CHST_ALL = 0x0,

    CHST_SC = 0x1,

    CHST_ITMP= 0x2,

    CHST_VA = 0x3,

    CHST_VD = 0x4

};



typedef enum {

    MODULE_DISABLED = 0,

    MODULE_ENABLED = 1

} BTM_module_enable_t;



typedef enum {

    CS_LOW = 0,

    CS_HIGH = 1

} CS_state_t;

typedef enum {

    CMD_WRCFGA = 0x0001,

    CMD_WRCFGB = 0x0024,

    CMD_RDCFGA = 0x0002,

    CMD_RDCFGB = 0x0026,

    CMD_RDCVA = 0x0004,

    CMD_RDCVB = 0x0006,

    CMD_RDCVC = 0x0008,

    CMD_RDCVD = 0x000A,

    CMD_RDCVE = 0x0009,

    CMD_RDCVF = 0x000B,

    CMD_RDAUXA = 0x000C,

    CMD_RDAUXB = 0x000E,

    CMD_RDAUXC = 0x000D,

    CMD_RDAUXD = 0x000F,

    CMD_RDSTATA = 0x0010,

    CMD_RDSTATB = 0x0012,

    CMD_WRSCTRL = 0x0014,

    CMD_WRPWM = 0x0020,

    CMD_WRPSB = 0x001C,

    CMD_RDSCTRL = 0x0016,

    CMD_RDPWM = 0x0022,

    CMD_RDPSB = 0x001E,

    CMD_STSCTRL = 0x0019,

    CMD_CLRSCTRL= 0x0018,





    CMD_ADCV = 0x0260 | (MD_7KHZ_3KHZ << 7) | (0 << 4),

 CMD_ADCV_CH1 = 0x0260 | (MD_7KHZ_3KHZ << 7) | (0 << 4) | CH_1,

 CMD_ADCV_CH2 = 0x0260 | (MD_7KHZ_3KHZ << 7) | (0 << 4) | CH_2,

 CMD_ADCV_CH3 = 0x0260 | (MD_7KHZ_3KHZ << 7) | (0 << 4) | CH_3,

 CMD_ADCV_CH4 = 0x0260 | (MD_7KHZ_3KHZ << 7) | (0 << 4) | CH_4,

 CMD_ADCV_CH5 = 0x0260 | (MD_7KHZ_3KHZ << 7) | (0 << 4) | CH_5,

 CMD_ADCV_CH6 = 0x0260 | (MD_7KHZ_3KHZ << 7) | (0 << 4) | CH_6,





    CMD_ADOW_PUP = 0x0228 | (MD_7KHZ_3KHZ << 7) | (1 << 6) | (0 << 4),

 CMD_ADOW_PDOWN = 0x0228 | (MD_7KHZ_3KHZ << 7) | (0 << 6) | (0 << 4),











    CMD_ADOL = 0x0201 | (MD_7KHZ_3KHZ << 7) | (0 << 4),





    CMD_ADAX_ALL = 0x0460 | (MD_7KHZ_3KHZ << 7) | CHG_ALL,

    CMD_ADAX_GPIO1_6 = 0x0460 | (MD_7KHZ_3KHZ << 7) | CHG_GPIO1_6,

    CMD_ADAX_GPIO2_7 = 0x0460 | (MD_7KHZ_3KHZ << 7) | CHG_GPIO2_7,

    CMD_ADAX_GPIO3_8 = 0x0460 | (MD_7KHZ_3KHZ << 7) | CHG_GPIO3_8,

    CMD_ADAX_GPIO4_9 = 0x0460 | (MD_7KHZ_3KHZ << 7) | CHG_GPIO4_9,

    CMD_ADAX_GPIO5 = 0x0460 | (MD_7KHZ_3KHZ << 7) | CHG_GPIO5,

    CMD_ADAX_VREF2 = 0x0460 | (MD_7KHZ_3KHZ << 7) | CHG_VREF2,



    CMD_ADAXD_ALL = 0x0400 | (MD_7KHZ_3KHZ << 7) | CHG_ALL,

    CMD_ADAXD_GPIO1_6 = 0x0400 | (MD_7KHZ_3KHZ << 7) | CHG_GPIO1_6,

    CMD_ADAXD_GPIO2_7 = 0x0400 | (MD_7KHZ_3KHZ << 7) | CHG_GPIO2_7,

    CMD_ADAXD_GPIO3_8 = 0x0400 | (MD_7KHZ_3KHZ << 7) | CHG_GPIO3_8,

    CMD_ADAXD_GPIO4_9 = 0x0400 | (MD_7KHZ_3KHZ << 7) | CHG_GPIO4_9,

    CMD_ADAXD_GPIO5 = 0x0400 | (MD_7KHZ_3KHZ << 7) | CHG_GPIO5,

    CMD_ADAXD_VREF2 = 0x0400 | (MD_7KHZ_3KHZ << 7) | CHG_VREF2,











    CMD_ADSTAT_ALL = 0x0468 | (MD_7KHZ_3KHZ << 7) | CHST_ALL,

    CMD_ADSTAT_SC = 0x0468 | (MD_7KHZ_3KHZ << 7) | CHST_SC,

    CMD_ADSTAT_ITMP = 0x0468 | (MD_7KHZ_3KHZ << 7) | CHST_ITMP,

    CMD_ADSTAT_VA = 0x0468 | (MD_7KHZ_3KHZ << 7) | CHST_VA,

    CMD_ADSTAT_VD = 0x0468 | (MD_7KHZ_3KHZ << 7) | CHST_VD,



    CMD_ADSTATD_ALL = 0x0408 | (MD_7KHZ_3KHZ << 7) | CHST_ALL,

    CMD_ADSTATD_SC = 0x0408 | (MD_7KHZ_3KHZ << 7) | CHST_SC,

    CMD_ADSTATD_ITMP= 0x0408 | (MD_7KHZ_3KHZ << 7) | CHST_ITMP,

    CMD_ADSTATD_VA = 0x0408 | (MD_7KHZ_3KHZ << 7) | CHST_VA,

    CMD_ADSTATD_VD = 0x0408 | (MD_7KHZ_3KHZ << 7) | CHST_VD,











    CMD_ADCVAX = 0x046F | (MD_7KHZ_3KHZ << 7) | (0 << 4),



    CMD_ADCVSC = 0x0467 | (MD_7KHZ_3KHZ << 7) | (0 << 4),



    CMD_CLRCELL = 0x0711,

    CMD_CLRAUX = 0x0712,

    CMD_CLRSTAT = 0x0713,

    CMD_PLADC = 0x0714,

    CMD_DIAGN = 0x0715,

    CMD_WRCOMM = 0x0721,

    CMD_RDCOMM = 0x0722,

    CMD_STCOMM = 0x0723,

    CMD_MUTE = 0x0028,

    CMD_UNMUTE = 0x0029

} BTM_command_t;

struct BTM_module {







    BTM_module_enable_t enable;

    uint16_t voltage;

    float temperature;

    int status;

};



struct BTM_stack {

    uint8_t cfgra[6];

    uint8_t cfgrb[6];

    unsigned int stack_voltage;

    struct BTM_module module[18];





};



typedef struct {

    unsigned int pack_voltage;

    struct BTM_stack stack[2U];







    int PH_status;

    uint8_t PH_SOC_LOCATION;



} BTM_PackData_t;





typedef struct {

    enum BTM_Error error;

    unsigned int device_num;









} BTM_Status_t;

uint16_t BTM_calculatePec15(uint8_t* data, int len);

void BTM_init(BTM_PackData_t * pack);

void BTM_wakeup(void);

void BTM_sendCmd(BTM_command_t command);

BTM_Status_t BTM_sendCmdAndPoll(BTM_command_t command);

void BTM_writeRegisterGroup(BTM_command_t command, uint8_t tx_data[][6]);

BTM_Status_t BTM_readRegisterGroup(BTM_command_t command, uint8_t rx_data[][6]);

BTM_Status_t BTM_readBatt(BTM_PackData_t * packData);

float BTM_regValToVoltage(unsigned int raw_reading);

void BTM_writeCS(CS_state_t new_state);
