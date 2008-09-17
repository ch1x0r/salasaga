all: generate_single_click generate_double_click generate_triple_click

generate_single_click:
	$(CC) $(CFLAGS) generate_single_click.c -l ming -o generate_single_click

generate_double_click:
	$(CC) $(CFLAGS) generate_double_click.c -l ming -o generate_double_click

generate_triple_click:
	$(CC) $(CFLAGS) generate_triple_click.c -l ming -o generate_triple_click

clean:
	rm -f generate_single_click generate_double_click generate_triple_click

