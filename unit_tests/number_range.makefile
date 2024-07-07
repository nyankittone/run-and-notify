unit_test_bin/$(NAME):
	$(CC) -I$(INCLUDE) $(OBJ_DIR)/number_range.o unit_tests/$(NAME).c -o $@

