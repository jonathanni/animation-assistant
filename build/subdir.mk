# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../ahcanvas.cpp \
../ahframe.cpp \
../ahimagedb.cpp \
../ahlogwindow.cpp \
../main.cpp 

OBJS += \
./ahcanvas.o \
./ahframe.o \
./ahimagedb.o \
./ahlogwindow.o \
./main.o 

CPP_DEPS += \
./ahcanvas.d \
./ahframe.d \
./ahimagedb.d \
./ahlogwindow.d \
./main.d 

# Set to wxWidgets root directory
WXDIR = 

# Set to wxWidgets build directory
WXBUILD = 

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++0x -I"$(WXBUILD)" -I"$(WXDIR)/include" -I"../" -O3 -g -Wall -c -Wundef -Wno-ctor-dtor-privacy -fno-strict-aliasing -faligned-new -fPIC -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


