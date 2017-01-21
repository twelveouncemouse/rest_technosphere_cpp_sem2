################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Connection.cpp \
../src/Proxy.cpp \
../src/ProxyListener.cpp \
../src/Selector.cpp \
../src/Server.cpp 

OBJS += \
./src/Connection.o \
./src/Proxy.o \
./src/ProxyListener.o \
./src/Selector.o \
./src/Server.o 

CPP_DEPS += \
./src/Connection.d \
./src/Proxy.d \
./src/ProxyListener.d \
./src/Selector.d \
./src/Server.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++1y -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


