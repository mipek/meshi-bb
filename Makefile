# Meshi BlackBox makefile
PROG := bb
SRCS := transmission_debug.cpp main.cpp errors.cpp client_controller.cpp cprintf.cpp
OBJDIR := build

CC      := c++
CFLAGS  := -Wall -Wextra -Werror
LDFLAGS :=

OBJS   = $(SRCS:.cpp=.o)
DEPS   = $(SRCS:.cpp=.d)

.PHONY: all
all: $(PROG)

$(PROG): $(OBJS)
#	mkdir -p build
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -MMD -c $< -o $@

-include $(DEPS)

.PHONY: clean cleaner
clean:
	rm -f $(OBJS) $(DEPS)

cleaner: clean
	rm -rf $(PROG)
