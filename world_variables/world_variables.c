#include "world_variables.h"

char **lines = NULL;
size_t lines_cap = INITIAL_CAPACITY;
size_t line_count = 0;
const char *input = "<stdin>";
FILE *file = NULL;
char *code = NULL;
int total_errors = 0;

const Bracket brackets[] = {
    {'(', ')', 0, 0}, {'{', '}', 0, 0}, {'[', ']', 0, 0}};

BracketStackItem *bracket_stack = NULL;
size_t bracket_stack_size = 0;
size_t bracket_stack_capacity = INITIAL_CAPACITY;

ErrorLog *error_logs = NULL;
size_t error_logs_capacity = INITIAL_CAPACITY;
size_t error_count = 0;