CXX = g++-11 -std=c++20
CXXFLAGS = -Wall -g -MMD  # use -MMD to generate dependencies
SOURCES = $(wildcard *.cc)   # list of all .cc files in the current directory
OBJECTS = ${SOURCES:.cc=.o}  # .o files depend upon .cc files with same names
DEPENDS = ${OBJECTS:.o=.d}   # .d file is list of dependencies for corresponding .cc file
EXEC=p5

# First target in the makefile is the default target.
$(EXEC): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(EXEC)  $(LIBFLAGS)

%.o: %.cc 
	$(CXX) -c -o $@ $< $(CXXFLAGS) $(LIBFLAGS)

-include ${DEPENDS}

.PHONY: clean
clean:
	rm  -f $(OBJECTS) $(DEPENDS) $(EXEC)

.PHONY: run array two
run: $(EXEC)
	./$(EXEC) < program.wlp4 > program.asm

array: $(EXEC)
	./$(EXEC) < program.wlp4 > program.asm
	cs241.linkasm < program.asm > program.merl
	cs241.linker program.merl print.merl alloc.merl > linked.merl
	mips.array linked.merl

two: $(EXEC)
	./$(EXEC) < program.wlp4 > program.asm
	cs241.linkasm < program.asm > program.merl
	cs241.linker program.merl print.merl alloc.merl > linked.merl
	mips.twoints linked.merl

twos: $(EXEC)
	./$(EXEC) < program.wlp4 > program.asm
	cs241.linkasm < program.asm > program.merl
	cs241.linker program.merl print.merl alloc.merl > linked.merl
	mips.stepper_twoints linked.merl


.PHONY: test
test: $(EXEC)
	valgrind ./$(EXEC) < program.wlp4 > program.asm

.PHONY: runa
runa:
	cs241.linkasm < program.asm > program.merl
	cs241.linker program.merl print.merl alloc.merl > linked.merl
	mips.array linked.merl
