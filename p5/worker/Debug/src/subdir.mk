################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/HashTableBase.cpp \
../src/SysVHashTable.cpp \
../src/UserCommand.cpp \
../src/WorkerProcess.cpp \
../src/p5-worker.cpp 

OBJS += \
./src/HashTableBase.o \
./src/SysVHashTable.o \
./src/UserCommand.o \
./src/WorkerProcess.o \
./src/p5-worker.o 

CPP_DEPS += \
./src/HashTableBase.d \
./src/SysVHashTable.d \
./src/UserCommand.d \
./src/WorkerProcess.d \
./src/p5-worker.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


