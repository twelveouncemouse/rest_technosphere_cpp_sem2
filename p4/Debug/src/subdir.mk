################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/BaseItem.cpp \
../src/CmdInterperter.cpp \
../src/CommandItem.cpp \
../src/LisonShell.cpp \
../src/OperItem.cpp \
../src/p4.cpp 

OBJS += \
./src/BaseItem.o \
./src/CmdInterperter.o \
./src/CommandItem.o \
./src/LisonShell.o \
./src/OperItem.o \
./src/p4.o 

CPP_DEPS += \
./src/BaseItem.d \
./src/CmdInterperter.d \
./src/CommandItem.d \
./src/LisonShell.d \
./src/OperItem.d \
./src/p4.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


