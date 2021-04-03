#include "Core/Src/ltc6813_btm.h"








typedef enum {

    DISCHARGE_OFF = 0,

    DISCHARGE_ON = 1

} BTM_module_bal_status_t;









struct BTM_BAL_dch_setting_stack {

    BTM_module_bal_status_t module_dch[18];

};



typedef struct {

    struct BTM_BAL_dch_setting_stack stack[2U];

} BTM_BAL_dch_setting_pack_t;









void BTM_BAL_initDchPack(BTM_BAL_dch_setting_pack_t* dch_pack);

void BTM_BAL_copyDchPack(

    BTM_BAL_dch_setting_pack_t* dch_pack_source,

    BTM_BAL_dch_setting_pack_t* dch_pack_target

);

void BTM_BAL_setDischarge(

    BTM_PackData_t* pack,

    BTM_BAL_dch_setting_pack_t* pack_dch_setting

);
