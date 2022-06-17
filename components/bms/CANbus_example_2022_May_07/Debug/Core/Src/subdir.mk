################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/BLINKY_bluepill.c \
../Core/Src/CAN_messages.c \
../Core/Src/CANbus_functions.c \
../Core/Src/Pack_packdata.c \
../Core/Src/analysis.c \
../Core/Src/headers_for_mocking.c \
../Core/Src/ltc6813_btm.c \
../Core/Src/main.c \
../Core/Src/stm32f1xx_hal_msp.c \
../Core/Src/stm32f1xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f1xx.c 

OBJS += \
./Core/Src/BLINKY_bluepill.o \
./Core/Src/CAN_messages.o \
./Core/Src/CANbus_functions.o \
./Core/Src/Pack_packdata.o \
./Core/Src/analysis.o \
./Core/Src/headers_for_mocking.o \
./Core/Src/ltc6813_btm.o \
./Core/Src/main.o \
./Core/Src/stm32f1xx_hal_msp.o \
./Core/Src/stm32f1xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f1xx.o 

C_DEPS += \
./Core/Src/BLINKY_bluepill.d \
./Core/Src/CAN_messages.d \
./Core/Src/CANbus_functions.d \
./Core/Src/Pack_packdata.d \
./Core/Src/analysis.d \
./Core/Src/headers_for_mocking.d \
./Core/Src/ltc6813_btm.d \
./Core/Src/main.d \
./Core/Src/stm32f1xx_hal_msp.d \
./Core/Src/stm32f1xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f1xx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/BLINKY_bluepill.d ./Core/Src/BLINKY_bluepill.o ./Core/Src/BLINKY_bluepill.su ./Core/Src/CAN_messages.d ./Core/Src/CAN_messages.o ./Core/Src/CAN_messages.su ./Core/Src/CANbus_functions.d ./Core/Src/CANbus_functions.o ./Core/Src/CANbus_functions.su ./Core/Src/Pack_packdata.d ./Core/Src/Pack_packdata.o ./Core/Src/Pack_packdata.su ./Core/Src/analysis.d ./Core/Src/analysis.o ./Core/Src/analysis.su ./Core/Src/headers_for_mocking.d ./Core/Src/headers_for_mocking.o ./Core/Src/headers_for_mocking.su ./Core/Src/ltc6813_btm.d ./Core/Src/ltc6813_btm.o ./Core/Src/ltc6813_btm.su ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/stm32f1xx_hal_msp.d ./Core/Src/stm32f1xx_hal_msp.o ./Core/Src/stm32f1xx_hal_msp.su ./Core/Src/stm32f1xx_it.d ./Core/Src/stm32f1xx_it.o ./Core/Src/stm32f1xx_it.su ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f1xx.d ./Core/Src/system_stm32f1xx.o ./Core/Src/system_stm32f1xx.su

.PHONY: clean-Core-2f-Src

