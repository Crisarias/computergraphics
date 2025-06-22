#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

void validate_number_argument(char *option)
{
    char *endp = NULL;
    long l = -1;
    if (((l = strtol(option, &endp, 0)), (endp && *endp)))
    {
        fprintf(stderr, "invalid argument %s - expecting a number.\n",
                option);
        exit(1);
    };
}

void validate_number_value(int value, char *option)
{
    if (value <= 0)
    {
        fprintf(stderr, "\nError: The %s option must be major than 0.\n",
                option);
        exit(1);
    };
}