CC	= g++

OMP	= -fopenmp

CPPFLAGS= -Wall -O2 $(OMP) -I/opt/AMDAPP/include
LFLAGS	= $(OMP) -lm -lOpenCL -L/opt/opencl/lib/x86_64
OBJ = main.o

default:run

run: $(OBJ)
	$(CC) $(LFLAGS) -o run $(OBJ)

main.o : main.cpp kernel.h

kernel.h: TemplateC_Kernels.cl
	./utils/template-converter kernel_str $< >| kernel.h

clean:;
	rm -rf $(OBJ) run kernel.h
