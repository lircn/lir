COMPILER         = g++
LIBS =

BUILD_LIST += find_num

OPTS=-std=c++11 -g -DDD=1

all: $(BUILD_LIST)

$(BUILD_LIST) : %: %.cpp
	$(COMPILER) $(OPTS) -o $@ $< $(LIBS)

%.o: %.cpp
	$(COMPILER) $(OPTS) -c $< $(LIBS)

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
