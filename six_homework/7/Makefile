CC=pgcc
CXX=pgc++
FC=pgfortran
LD=pgc++
CFLAGS=-fast -acc=gpu 
FFLAGS=-fast -acc=gpu 
CXXFLAGS=-std=c++11 -Minfo=all -fast -acc=gpu
LDFLAGS= -lboost_program_options -acc=gpu

NVTXLIB := -I/opt/nvidia/hpc_sdk/Linux_x86_64/20.9/cuda/11.0/include -cudalib=cublas
boot = -I/path/to/boost/include -L/path/to/boost/lib -lboost_program_options

all: exe run

exe: laplace.o jacobi.o
	$(LD) $(LDFLAGS) -o $@ $^ ${NVTXLIB} ${boot}


run: exe
	./exe

.PHONY: clean
clean:
	-rm -f *.o *.mod core exe
.SUFFIXES: .c .cpp .F90 .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $< ${NVTXLIB} ${boot}
.c.o:
	$(CC) $(CFLAGS) -c -o $@ $< ${NVTXLIB} ${boot}
.F90.o:
	$(FC) $(FFLAGS) -c -o $@ $< ${NVTXLIB} ${boot} 

tr: 
	$(CXX) $(CXXFLAGS) -c -o $@ $< ${NVTXLIB} ${boot}