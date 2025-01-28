################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/CommHandler.c \
../Core/Src/HP20x.c \
../Core/Src/INA3221.c \
../Core/Src/LoRa_E5.c \
../Core/Src/SEN5x.c \
../Core/Src/SHT4x.c \
../Core/Src/command_processor.c \
../Core/Src/crc.c \
../Core/Src/dma.c \
../Core/Src/gpio.c \
../Core/Src/i2c.c \
../Core/Src/kalman.c \
../Core/Src/main.c \
../Core/Src/stm32l4xx_hal_msp.c \
../Core/Src/stm32l4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32l4xx.c \
../Core/Src/usart.c 

OBJS += \
./Core/Src/CommHandler.o \
./Core/Src/HP20x.o \
./Core/Src/INA3221.o \
./Core/Src/LoRa_E5.o \
./Core/Src/SEN5x.o \
./Core/Src/SHT4x.o \
./Core/Src/command_processor.o \
./Core/Src/crc.o \
./Core/Src/dma.o \
./Core/Src/gpio.o \
./Core/Src/i2c.o \
./Core/Src/kalman.o \
./Core/Src/main.o \
./Core/Src/stm32l4xx_hal_msp.o \
./Core/Src/stm32l4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32l4xx.o \
./Core/Src/usart.o 

C_DEPS += \
./Core/Src/CommHandler.d \
./Core/Src/HP20x.d \
./Core/Src/INA3221.d \
./Core/Src/LoRa_E5.d \
./Core/Src/SEN5x.d \
./Core/Src/SHT4x.d \
./Core/Src/command_processor.d \
./Core/Src/crc.d \
./Core/Src/dma.d \
./Core/Src/gpio.d \
./Core/Src/i2c.d \
./Core/Src/kalman.d \
./Core/Src/main.d \
./Core/Src/stm32l4xx_hal_msp.d \
./Core/Src/stm32l4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32l4xx.d \
./Core/Src/usart.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L412xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/CommHandler.cyclo ./Core/Src/CommHandler.d ./Core/Src/CommHandler.o ./Core/Src/CommHandler.su ./Core/Src/HP20x.cyclo ./Core/Src/HP20x.d ./Core/Src/HP20x.o ./Core/Src/HP20x.su ./Core/Src/INA3221.cyclo ./Core/Src/INA3221.d ./Core/Src/INA3221.o ./Core/Src/INA3221.su ./Core/Src/LoRa_E5.cyclo ./Core/Src/LoRa_E5.d ./Core/Src/LoRa_E5.o ./Core/Src/LoRa_E5.su ./Core/Src/SEN5x.cyclo ./Core/Src/SEN5x.d ./Core/Src/SEN5x.o ./Core/Src/SEN5x.su ./Core/Src/SHT4x.cyclo ./Core/Src/SHT4x.d ./Core/Src/SHT4x.o ./Core/Src/SHT4x.su ./Core/Src/command_processor.cyclo ./Core/Src/command_processor.d ./Core/Src/command_processor.o ./Core/Src/command_processor.su ./Core/Src/crc.cyclo ./Core/Src/crc.d ./Core/Src/crc.o ./Core/Src/crc.su ./Core/Src/dma.cyclo ./Core/Src/dma.d ./Core/Src/dma.o ./Core/Src/dma.su ./Core/Src/gpio.cyclo ./Core/Src/gpio.d ./Core/Src/gpio.o ./Core/Src/gpio.su ./Core/Src/i2c.cyclo ./Core/Src/i2c.d ./Core/Src/i2c.o ./Core/Src/i2c.su ./Core/Src/kalman.cyclo ./Core/Src/kalman.d ./Core/Src/kalman.o ./Core/Src/kalman.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/stm32l4xx_hal_msp.cyclo ./Core/Src/stm32l4xx_hal_msp.d ./Core/Src/stm32l4xx_hal_msp.o ./Core/Src/stm32l4xx_hal_msp.su ./Core/Src/stm32l4xx_it.cyclo ./Core/Src/stm32l4xx_it.d ./Core/Src/stm32l4xx_it.o ./Core/Src/stm32l4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32l4xx.cyclo ./Core/Src/system_stm32l4xx.d ./Core/Src/system_stm32l4xx.o ./Core/Src/system_stm32l4xx.su ./Core/Src/usart.cyclo ./Core/Src/usart.d ./Core/Src/usart.o ./Core/Src/usart.su

.PHONY: clean-Core-2f-Src

