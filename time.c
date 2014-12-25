#include <time.h>
#include <stdio.h>
#include <stdlib.h>

int time_number (void)
{
    time_t t1 = time(NULL);
    struct tm *ptr = localtime(&t1);
    int year = ptr->tm_year + 1900;
    int month = ptr->tm_mon + 1;
    int hour = ptr->tm_hour;

    return (hour + month + year);
}
