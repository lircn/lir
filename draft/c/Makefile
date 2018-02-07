COMPILER         = g++
#OPTIMIZATION_OPT = -g -O0
OPTIONS = $(OPTIMIZATION_OPT)
LIBS = -lcurl -lmysqlclient -lz

BUILD_LIST += t
OBJS =

all: $(BUILD_LIST)

$(BUILD_LIST) : %: %.cpp $(OBJS)
	$(COMPILER) $(OPTIONS) -o $@ $(OBJS) $< $(LINKER_OPT) $(LIBS)

%.o: %.cpp
	$(COMPILER) $(OPTIONS) -c $< $(LINKER_OPT) $(LIBS)

strip_bin :
	@for f in $(BUILD_LIST); do if [ -f $$f ]; then strip -s $$f; echo $$f; fi done; 

valgrind :
	@for f in $(BUILD_LIST); do \
		if [ -f $$f ]; then \
			cmd="valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=$$f.log -v ./$$f"; \
			echo $$cmd; \
			$$cmd; \
		fi done;

clean:
	rm -f core *.o *.bak $(BUILD_LIST) *stackdump *#

#
# The End !
#
