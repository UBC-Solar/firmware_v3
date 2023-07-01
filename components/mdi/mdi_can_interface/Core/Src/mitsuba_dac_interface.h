



uint16_t Parse_Acceleration(uint32_t pedal_data);
void Send_Regen(float regen, uint8_t DAC_REGEN_ADDR, I2C_HandleTypeDef *hi2c1);
void Send_Test_Message(uint8_t* TxData, int32_t velocity, uint32_t acceleration); 