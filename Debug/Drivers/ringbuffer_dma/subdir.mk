################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/ringbuffer_dma/ringbuffer_dma.c 

OBJS += \
./Drivers/ringbuffer_dma/ringbuffer_dma.o 

C_DEPS += \
./Drivers/ringbuffer_dma/ringbuffer_dma.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/ringbuffer_dma/ringbuffer_dma.o: ../Drivers/ringbuffer_dma/ringbuffer_dma.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F446xx -DDEBUG -c -I../Core/Inc -I../Drivers/ringbuffer_dma -I../Drivers/display -I../Drivers/BMP280 -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Drivers/ringbuffer_dma/ringbuffer_dma.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

