CXX=pgc++
LD=pgc++

CXXFLAGS=-fast -acc -gpu=cc70
LDFLAGS=-fast -acc -gpu=cc70

NVTXLIB := -I/opt/nvidia/hpc_sdk/Linux_x86_64/23.11/cuda/12.3/include/
boot = -I/path/to/boost/include -L/path/to/boost/lib -lboost_program_options

SOURCES_graph = graph.cu
OBJECTS_graph = $(SOURCES_graph:.cpp=.o)

TARGETS = graph


all: $(TARGETS)

graph: $(OBJECTS_graph)
	$(LD) $(LDFLAGS) -o $@ $^ ${NVTXLIB} ${boot}

run: graph
	./graph

.PHONY: clean
clean:
	-rm -f *.o *.mod core $(TARGETS)

.SUFFIXES: .c .cpp .F90 .o

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $< ${NVTXLIB} ${boot}
.c.o:
	$(CC) $(CFLAGS) -c -o $@ $< ${NVTXLIB} ${boot}
.F90.o:
	$(FC) $(FFLAGS) -c -o $@ $< ${NVTXLIB} ${boot}