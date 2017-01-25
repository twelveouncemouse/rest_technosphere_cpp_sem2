################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/CleaningCompany.cpp \
../src/ClientConnection.cpp \
../src/PosixHashTable.cpp \
../src/ServerProcess.cpp \
../src/SocketTricks.cpp \
../src/SynchronizedProcess.cpp \
../src/UserCommand.cpp \
../src/WorkerProcess.cpp \
../src/p6.cpp 

OBJS += \
./src/CleaningCompany.o \
./src/ClientConnection.o \
./src/PosixHashTable.o \
./src/ServerProcess.o \
./src/SocketTricks.o \
./src/SynchronizedProcess.o \
./src/UserCommand.o \
./src/WorkerProcess.o \
./src/p6.o 

CPP_DEPS += \
./src/CleaningCompany.d \
./src/ClientConnection.d \
./src/PosixHashTable.d \
./src/ServerProcess.d \
./src/SocketTricks.d \
./src/SynchronizedProcess.d \
./src/UserCommand.d \
./src/WorkerProcess.d \
./src/p6.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


