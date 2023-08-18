#include "test.h"

void TEST_hvContactors()
{
    HAL_GPIO_WritePin(LLIM_CTRL_GPIO_Port, LLIM_CTRL_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HLIM_CTRL_GPIO_Port, HLIM_CTRL_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(DCDC_PLUS_CTRL_GPIO_Port, DCDC_PLUS_CTRL_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(DCDC_MINUS_CTRL_GPIO_Port, DCDC_MINUS_CTRL_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(NEG_CTRL_GPIO_Port, NEG_CTRL_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PC_CTRL_GPIO_Port, PC_CTRL_Pin, GPIO_PIN_SET);
}

void TEST_lvControl()
{
    HAL_GPIO_WritePin(AMB_CTRL_GPIO_Port, AMB_CTRL_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(DID_CTRL_GPIO_Port, DID_CTRL_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SPAR1_CTRL_GPIO_Port, SPAR1_CTRL_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MCB_CTRL_GPIO_Port, MCB_CTRL_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(TEL_CTRL_GPIO_Port, TEL_CTRL_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MDI_CTRL_GPIO_Port, MDI_CTRL_Pin, GPIO_PIN_SET);
}

void TEST_fanControl()
{
    HAL_GPIO_WritePin(FAN1_CTRL_GPIO_Port, FAN1_CTRL_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(FAN2_CTRL_GPIO_Port, FAN2_CTRL_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(FAN3_CTRL_GPIO_Port, FAN3_CTRL_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(FAN4_CTRL_GPIO_Port, FAN4_CTRL_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MDU_FAN_CTRL_GPIO_Port, MDU_FAN_CTRL_Pin, GPIO_PIN_SET);
}

void TEST_readCurrentReferences()
{
}

void TEST_estop()
{

}

void TEST_relays()
{
    // HAL_GPIO_WritePin(DCH_RST_GPIO_Port, DCH_RST_Pin, GPIO_PIN_SET); // comment out to enable discharge circuitry
    HAL_GPIO_WritePin(SWAP_CTRL_GPIO_Port, SWAP_CTRL_Pin, GPIO_PIN_SET);
}

void TEST_can(CAN_HandleTypeDef *hcan)
{
}