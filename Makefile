all:
	clang main.c utilities/utilities.c world_variables/world_variables.c tinyexpr/tinyexpr.c  tuple/tuple.c interpreter/interpreter.c math/math.c parentheses/parentheses.c -lm -O3 -o null

format:
	clang-format -i $(shell find . -type f \( -name "*.c" -o -name "*.h" \))

clean:
	rm -f null