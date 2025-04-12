CC = gcc
LIBS_PATH = ./libs
SRCMODULES = $(LIBS_PATH)/Input.c $(LIBS_PATH)/Menu.c $(LIBS_PATH)/Services.c $(LIBS_PATH)/Operations.c $(LIBS_PATH)/Core.c
OBJMODULES = $(SRCMODULES:.c=.o)
CFLAGS = -Wall -g

%.o: $(LIBS_PATH)/%.c
	$(CC) $(CFLAGS) -c $< -o $(LIBS_PATH)/$@

main: main.c $(OBJMODULES)
	$(CC) $(CFLAGS) $^ -o $@

run: main
	./$^

clean:
	rm -rf $(OBJMODULES) main
