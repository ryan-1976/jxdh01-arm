################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/circlebuff.c \
../src/monitor.c \
../src/mqtt-pub.c \
../src/mqtt-sub.c \
../src/msgDispatcher.c \
../src/tabProc.c 

OBJS += \
./src/circlebuff.o \
./src/monitor.o \
./src/mqtt-pub.o \
./src/mqtt-sub.o \
./src/msgDispatcher.o \
./src/tabProc.o 

C_DEPS += \
./src/circlebuff.d \
./src/monitor.d \
./src/mqtt-pub.d \
./src/mqtt-sub.d \
./src/msgDispatcher.d \
./src/tabProc.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-linux-gnueabihf-gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


