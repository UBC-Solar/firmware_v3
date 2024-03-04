################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/CANbus_functions.c \
../Core/Src/adc.c \
../Core/Src/analysis.c \
../Core/Src/btm_bal_settings.c \
../Core/Src/control.c \
../Core/Src/fsm.c \
../Core/Src/ltc6813_btm.c \
../Core/Src/ltc6813_btm_bal.c \
../Core/Src/ltc6813_btm_temp.c \
../Core/Src/main.c \
../Core/Src/selftest.c \
../Core/Src/stm32f1xx_hal_msp.c \
../Core/Src/stm32f1xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f1xx.c \
../Core/Src/util.c 

OBJS += \
./Core/Src/CANbus_functions.o \
./Core/Src/adc.o \
./Core/Src/analysis.o \
./Core/Src/btm_bal_settings.o \
./Core/Src/control.o \
./Core/Src/fsm.o \
./Core/Src/ltc6813_btm.o \
./Core/Src/ltc6813_btm_bal.o \
./Core/Src/ltc6813_btm_temp.o \
./Core/Src/main.o \
./Core/Src/selftest.o \
./Core/Src/stm32f1xx_hal_msp.o \
./Core/Src/stm32f1xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f1xx.o \
./Core/Src/util.o 

C_DEPS += \
./Core/Src/CANbus_functions.d \
./Core/Src/adc.d \
./Core/Src/analysis.d \
./Core/Src/btm_bal_settings.d \
./Core/Src/control.d \
./Core/Src/fsm.d \
./Core/Src/ltc6813_btm.d \
./Core/Src/ltc6813_btm_bal.d \
./Core/Src/ltc6813_btm_temp.d \
./Core/Src/main.d \
./Core/Src/selftest.d \
./Core/Src/stm32f1xx_hal_msp.d \
./Core/Src/stm32f1xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f1xx.d \
./Core/Src/util.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DSTM32F103xE -DUSE_HAL_DRIVER -DDEBUG -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/CANbus_functions.cyclo ./Core/Src/CANbus_functions.d ./Core/Src/CANbus_functions.o ./Core/Src/CANbus_functions.su ./Core/Src/adc.cyclo ./Core/Src/adc.d ./Core/Src/adc.o ./Core/Src/adc.su ./Core/Src/analysis.cyclo ./Core/Src/analysis.d ./Core/Src/analysis.o ./Core/Src/analysis.su ./Core/Src/btm_bal_settings.cyclo ./Core/Src/btm_bal_settings.d ./Core/Src/btm_bal_settings.o ./Core/Src/btm_bal_settings.su ./Core/Src/control.cyclo ./Core/Src/control.d ./Core/Src/control.o ./Core/Src/control.su ./Core/Src/fsm.cyclo ./Core/Src/fsm.d ./Core/Src/fsm.o ./Core/Src/fsm.su ./Core/Src/ltc6813_btm.cyclo ./Core/Src/ltc6813_btm.d ./Core/Src/ltc6813_btm.o ./Core/Src/ltc6813_btm.su ./Core/Src/ltc6813_btm_bal.cyclo ./Core/Src/ltc6813_btm_bal.d ./Core/Src/ltc6813_btm_bal.o ./Core/Src/ltc6813_btm_bal.su ./Core/Src/ltc6813_btm_temp.cyclo ./Core/Src/ltc6813_btm_temp.d ./Core/Src/ltc6813_btm_temp.o ./Core/Src/ltc6813_btm_temp.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/selftest.cyclo ./Core/Src/selftest.d ./Core/Src/selftest.o ./Core/Src/selftest.su ./Core/Src/stm32f1xx_hal_msp.cyclo ./Core/Src/stm32f1xx_hal_msp.d ./Core/Src/stm32f1xx_hal_msp.o ./Core/Src/stm32f1xx_hal_msp.su ./Core/Src/stm32f1xx_it.cyclo ./Core/Src/stm32f1xx_it.d ./Core/Src/stm32f1xx_it.o ./Core/Src/stm32f1xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f1xx.cyclo ./Core/Src/system_stm32f1xx.d ./Core/Src/system_stm32f1xx.o ./Core/Src/system_stm32f1xx.su ./Core/Src/util.cyclo ./Core/Src/util.d ./Core/Src/util.o ./Core/Src/util.su

.PHONY: clean-Core-2f-Src

