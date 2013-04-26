CC = gcc
BIN = life
SOURCE = life.c
LINKS = -lpthread -lncurses

build: clean
	@echo "+----------------------------------------------+"
	@echo "| Making $(BIN)"
	@echo "+----------------------------------------------+"
	$(CC) -o $(BIN) $(SOURCE) $(LINKS)

clean:
	@echo "+----------------------------------------------+"
	@echo "| Removing $(BIN) executable"
	@echo "+----------------------------------------------+"
	@rm -rf $(BIN)

run: build
	@./$(BIN)
