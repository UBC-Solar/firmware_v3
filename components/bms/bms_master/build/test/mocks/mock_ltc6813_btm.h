/* AUTOGENERATED FILE. DO NOT EDIT. */
#ifndef _MOCK_LTC6813_BTM_H
#define _MOCK_LTC6813_BTM_H

#include "unity.h"
#include <stdbool.h>
#include <stdint.h>
#include "ltc6813_btm.h"

/* Ignore the following warnings, since we are copying code */
#if defined(__GNUC__) && !defined(__ICC) && !defined(__TMS470__)
#if __GNUC__ > 4 || (__GNUC__ == 4 && (__GNUC_MINOR__ > 6 || (__GNUC_MINOR__ == 6 && __GNUC_PATCHLEVEL__ > 0)))
#pragma GCC diagnostic push
#endif
#if !defined(__clang__)
#pragma GCC diagnostic ignored "-Wpragmas"
#endif
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wduplicate-decl-specifier"
#endif

void mock_ltc6813_btm_Init(void);
void mock_ltc6813_btm_Destroy(void);
void mock_ltc6813_btm_Verify(void);




#define BTM_calculatePec15_IgnoreAndReturn(cmock_retval) BTM_calculatePec15_CMockIgnoreAndReturn(__LINE__, cmock_retval)
void BTM_calculatePec15_CMockIgnoreAndReturn(UNITY_LINE_TYPE cmock_line, uint16_t cmock_to_return);
#define BTM_calculatePec15_StopIgnore() BTM_calculatePec15_CMockStopIgnore()
void BTM_calculatePec15_CMockStopIgnore(void);
#define BTM_calculatePec15_ExpectAndReturn(data, len, cmock_retval) BTM_calculatePec15_CMockExpectAndReturn(__LINE__, data, len, cmock_retval)
void BTM_calculatePec15_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, uint8_t* data, int len, uint16_t cmock_to_return);
typedef uint16_t (* CMOCK_BTM_calculatePec15_CALLBACK)(uint8_t* data, int len, int cmock_num_calls);
void BTM_calculatePec15_AddCallback(CMOCK_BTM_calculatePec15_CALLBACK Callback);
void BTM_calculatePec15_Stub(CMOCK_BTM_calculatePec15_CALLBACK Callback);
#define BTM_calculatePec15_StubWithCallback BTM_calculatePec15_Stub
#define BTM_init_Ignore() BTM_init_CMockIgnore()
void BTM_init_CMockIgnore(void);
#define BTM_init_StopIgnore() BTM_init_CMockStopIgnore()
void BTM_init_CMockStopIgnore(void);
#define BTM_init_Expect(pack) BTM_init_CMockExpect(__LINE__, pack)
void BTM_init_CMockExpect(UNITY_LINE_TYPE cmock_line, BTM_PackData_t* pack);
typedef void (* CMOCK_BTM_init_CALLBACK)(BTM_PackData_t* pack, int cmock_num_calls);
void BTM_init_AddCallback(CMOCK_BTM_init_CALLBACK Callback);
void BTM_init_Stub(CMOCK_BTM_init_CALLBACK Callback);
#define BTM_init_StubWithCallback BTM_init_Stub
#define BTM_wakeup_Ignore() BTM_wakeup_CMockIgnore()
void BTM_wakeup_CMockIgnore(void);
#define BTM_wakeup_StopIgnore() BTM_wakeup_CMockStopIgnore()
void BTM_wakeup_CMockStopIgnore(void);
#define BTM_wakeup_Expect() BTM_wakeup_CMockExpect(__LINE__)
void BTM_wakeup_CMockExpect(UNITY_LINE_TYPE cmock_line);
typedef void (* CMOCK_BTM_wakeup_CALLBACK)(int cmock_num_calls);
void BTM_wakeup_AddCallback(CMOCK_BTM_wakeup_CALLBACK Callback);
void BTM_wakeup_Stub(CMOCK_BTM_wakeup_CALLBACK Callback);
#define BTM_wakeup_StubWithCallback BTM_wakeup_Stub
#define BTM_sendCmd_Ignore() BTM_sendCmd_CMockIgnore()
void BTM_sendCmd_CMockIgnore(void);
#define BTM_sendCmd_StopIgnore() BTM_sendCmd_CMockStopIgnore()
void BTM_sendCmd_CMockStopIgnore(void);
#define BTM_sendCmd_Expect(command) BTM_sendCmd_CMockExpect(__LINE__, command)
void BTM_sendCmd_CMockExpect(UNITY_LINE_TYPE cmock_line, BTM_command_t command);
typedef void (* CMOCK_BTM_sendCmd_CALLBACK)(BTM_command_t command, int cmock_num_calls);
void BTM_sendCmd_AddCallback(CMOCK_BTM_sendCmd_CALLBACK Callback);
void BTM_sendCmd_Stub(CMOCK_BTM_sendCmd_CALLBACK Callback);
#define BTM_sendCmd_StubWithCallback BTM_sendCmd_Stub
#define BTM_sendCmdAndPoll_IgnoreAndReturn(cmock_retval) BTM_sendCmdAndPoll_CMockIgnoreAndReturn(__LINE__, cmock_retval)
void BTM_sendCmdAndPoll_CMockIgnoreAndReturn(UNITY_LINE_TYPE cmock_line, BTM_Status_t cmock_to_return);
#define BTM_sendCmdAndPoll_StopIgnore() BTM_sendCmdAndPoll_CMockStopIgnore()
void BTM_sendCmdAndPoll_CMockStopIgnore(void);
#define BTM_sendCmdAndPoll_ExpectAndReturn(command, cmock_retval) BTM_sendCmdAndPoll_CMockExpectAndReturn(__LINE__, command, cmock_retval)
void BTM_sendCmdAndPoll_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, BTM_command_t command, BTM_Status_t cmock_to_return);
typedef BTM_Status_t (* CMOCK_BTM_sendCmdAndPoll_CALLBACK)(BTM_command_t command, int cmock_num_calls);
void BTM_sendCmdAndPoll_AddCallback(CMOCK_BTM_sendCmdAndPoll_CALLBACK Callback);
void BTM_sendCmdAndPoll_Stub(CMOCK_BTM_sendCmdAndPoll_CALLBACK Callback);
#define BTM_sendCmdAndPoll_StubWithCallback BTM_sendCmdAndPoll_Stub
#define BTM_writeRegisterGroup_Ignore() BTM_writeRegisterGroup_CMockIgnore()
void BTM_writeRegisterGroup_CMockIgnore(void);
#define BTM_writeRegisterGroup_StopIgnore() BTM_writeRegisterGroup_CMockStopIgnore()
void BTM_writeRegisterGroup_CMockStopIgnore(void);
#define BTM_writeRegisterGroup_Expect(command, tx_data) BTM_writeRegisterGroup_CMockExpect(__LINE__, command, tx_data)
void BTM_writeRegisterGroup_CMockExpect(UNITY_LINE_TYPE cmock_line, BTM_command_t command, uint8_t** tx_data);
typedef void (* CMOCK_BTM_writeRegisterGroup_CALLBACK)(BTM_command_t command, uint8_t** tx_data, int cmock_num_calls);
void BTM_writeRegisterGroup_AddCallback(CMOCK_BTM_writeRegisterGroup_CALLBACK Callback);
void BTM_writeRegisterGroup_Stub(CMOCK_BTM_writeRegisterGroup_CALLBACK Callback);
#define BTM_writeRegisterGroup_StubWithCallback BTM_writeRegisterGroup_Stub
#define BTM_readRegisterGroup_IgnoreAndReturn(cmock_retval) BTM_readRegisterGroup_CMockIgnoreAndReturn(__LINE__, cmock_retval)
void BTM_readRegisterGroup_CMockIgnoreAndReturn(UNITY_LINE_TYPE cmock_line, BTM_Status_t cmock_to_return);
#define BTM_readRegisterGroup_StopIgnore() BTM_readRegisterGroup_CMockStopIgnore()
void BTM_readRegisterGroup_CMockStopIgnore(void);
#define BTM_readRegisterGroup_ExpectAndReturn(command, rx_data, cmock_retval) BTM_readRegisterGroup_CMockExpectAndReturn(__LINE__, command, rx_data, cmock_retval)
void BTM_readRegisterGroup_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, BTM_command_t command, uint8_t** rx_data, BTM_Status_t cmock_to_return);
typedef BTM_Status_t (* CMOCK_BTM_readRegisterGroup_CALLBACK)(BTM_command_t command, uint8_t** rx_data, int cmock_num_calls);
void BTM_readRegisterGroup_AddCallback(CMOCK_BTM_readRegisterGroup_CALLBACK Callback);
void BTM_readRegisterGroup_Stub(CMOCK_BTM_readRegisterGroup_CALLBACK Callback);
#define BTM_readRegisterGroup_StubWithCallback BTM_readRegisterGroup_Stub
#define BTM_readBatt_IgnoreAndReturn(cmock_retval) BTM_readBatt_CMockIgnoreAndReturn(__LINE__, cmock_retval)
void BTM_readBatt_CMockIgnoreAndReturn(UNITY_LINE_TYPE cmock_line, BTM_Status_t cmock_to_return);
#define BTM_readBatt_StopIgnore() BTM_readBatt_CMockStopIgnore()
void BTM_readBatt_CMockStopIgnore(void);
#define BTM_readBatt_ExpectAndReturn(packData, cmock_retval) BTM_readBatt_CMockExpectAndReturn(__LINE__, packData, cmock_retval)
void BTM_readBatt_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, BTM_PackData_t* packData, BTM_Status_t cmock_to_return);
typedef BTM_Status_t (* CMOCK_BTM_readBatt_CALLBACK)(BTM_PackData_t* packData, int cmock_num_calls);
void BTM_readBatt_AddCallback(CMOCK_BTM_readBatt_CALLBACK Callback);
void BTM_readBatt_Stub(CMOCK_BTM_readBatt_CALLBACK Callback);
#define BTM_readBatt_StubWithCallback BTM_readBatt_Stub
#define BTM_regValToVoltage_IgnoreAndReturn(cmock_retval) BTM_regValToVoltage_CMockIgnoreAndReturn(__LINE__, cmock_retval)
void BTM_regValToVoltage_CMockIgnoreAndReturn(UNITY_LINE_TYPE cmock_line, float cmock_to_return);
#define BTM_regValToVoltage_StopIgnore() BTM_regValToVoltage_CMockStopIgnore()
void BTM_regValToVoltage_CMockStopIgnore(void);
#define BTM_regValToVoltage_ExpectAndReturn(raw_reading, cmock_retval) BTM_regValToVoltage_CMockExpectAndReturn(__LINE__, raw_reading, cmock_retval)
void BTM_regValToVoltage_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, unsigned int raw_reading, float cmock_to_return);
typedef float (* CMOCK_BTM_regValToVoltage_CALLBACK)(unsigned int raw_reading, int cmock_num_calls);
void BTM_regValToVoltage_AddCallback(CMOCK_BTM_regValToVoltage_CALLBACK Callback);
void BTM_regValToVoltage_Stub(CMOCK_BTM_regValToVoltage_CALLBACK Callback);
#define BTM_regValToVoltage_StubWithCallback BTM_regValToVoltage_Stub
#define BTM_writeCS_Ignore() BTM_writeCS_CMockIgnore()
void BTM_writeCS_CMockIgnore(void);
#define BTM_writeCS_StopIgnore() BTM_writeCS_CMockStopIgnore()
void BTM_writeCS_CMockStopIgnore(void);
#define BTM_writeCS_Expect(new_state) BTM_writeCS_CMockExpect(__LINE__, new_state)
void BTM_writeCS_CMockExpect(UNITY_LINE_TYPE cmock_line, CS_state_t new_state);
typedef void (* CMOCK_BTM_writeCS_CALLBACK)(CS_state_t new_state, int cmock_num_calls);
void BTM_writeCS_AddCallback(CMOCK_BTM_writeCS_CALLBACK Callback);
void BTM_writeCS_Stub(CMOCK_BTM_writeCS_CALLBACK Callback);
#define BTM_writeCS_StubWithCallback BTM_writeCS_Stub

#if defined(__GNUC__) && !defined(__ICC) && !defined(__TMS470__)
#if __GNUC__ > 4 || (__GNUC__ == 4 && (__GNUC_MINOR__ > 6 || (__GNUC_MINOR__ == 6 && __GNUC_PATCHLEVEL__ > 0)))
#pragma GCC diagnostic pop
#endif
#endif

#endif