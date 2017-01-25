################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/ClientConnection.cpp \
../src/HashTableBase.cpp \
../src/MasterProcess.cpp \
../src/UserCommand.cpp \
../src/p5-master.cpp 

OBJS += \
./src/ClientConnection.o \
./src/HashTableBase.o \
./src/MasterProcess.o \
./src/UserCommand.o \
./src/p5-master.o 

CPP_DEPS += \
./src/ClientConnection.d \
./src/HashTableBase.d \
./src/MasterProcess.d \
./src/UserCommand.d \
./src/p5-master.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


