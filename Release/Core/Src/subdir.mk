################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/LTC681x.c \
../Core/Src/calculations.c \
../Core/Src/can.c \
../Core/Src/isoSpi.c \
../Core/Src/main.c \
../Core/Src/operation.c \
../Core/Src/pwm.c \
../Core/Src/stm32g4xx_hal_msp.c \
../Core/Src/stm32g4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32g4xx.c \
../Core/Src/temp_calc.c 

OBJS += \
./Core/Src/LTC681x.o \
./Core/Src/calculations.o \
./Core/Src/can.o \
./Core/Src/isoSpi.o \
./Core/Src/main.o \
./Core/Src/operation.o \
./Core/Src/pwm.o \
./Core/Src/stm32g4xx_hal_msp.o \
./Core/Src/stm32g4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32g4xx.o \
./Core/Src/temp_calc.o 

C_DEPS += \
./Core/Src/LTC681x.d \
./Core/Src/calculations.d \
./Core/Src/can.d \
./Core/Src/isoSpi.d \
./Core/Src/main.d \
./Core/Src/operation.d \
./Core/Src/pwm.d \
./Core/Src/stm32g4xx_hal_msp.d \
./Core/Src/stm32g4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32g4xx.d \
./Core/Src/temp_calc.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32G431xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/LTC681x.d ./Core/Src/LTC681x.o ./Core/Src/LTC681x.su ./Core/Src/calculations.d ./Core/Src/calculations.o ./Core/Src/calculations.su ./Core/Src/can.d ./Core/Src/can.o ./Core/Src/can.su ./Core/Src/isoSpi.d ./Core/Src/isoSpi.o ./Core/Src/isoSpi.su ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/operation.d ./Core/Src/operation.o ./Core/Src/operation.su ./Core/Src/pwm.d ./Core/Src/pwm.o ./Core/Src/pwm.su ./Core/Src/stm32g4xx_hal_msp.d ./Core/Src/stm32g4xx_hal_msp.o ./Core/Src/stm32g4xx_hal_msp.su ./Core/Src/stm32g4xx_it.d ./Core/Src/stm32g4xx_it.o ./Core/Src/stm32g4xx_it.su ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32g4xx.d ./Core/Src/system_stm32g4xx.o ./Core/Src/system_stm32g4xx.su ./Core/Src/temp_calc.d ./Core/Src/temp_calc.o ./Core/Src/temp_calc.su

.PHONY: clean-Core-2f-Src

