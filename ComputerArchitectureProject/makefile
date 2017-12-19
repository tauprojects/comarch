CC = gcc

OBJS2 = sim.o 


# The executabel filename of the sim app
EXEC2 = sim


C_COMP_FLAG = -Wall -g  -DNDEBUG 

#C_COMP_FLAG = -std=c99 -Wall -Wextra \
#-Werror -pedantic-errors -DNDEBUG

all: $(EXEC2)


sim.o:	sim.h   
		$(CC) $(C_COMP_FLAG)  -c $*.c


		
$(EXEC2): $(OBJS2)
		$(CC) $(OBJS2) -o $@

# a rule for building a simple c souorce file


clean:
		rm -f $(OBJS2) $(EXEC2)

