//
// Created by nzamora on 9/22/2024.
//
#include <stdio.h>
#include <string.h>

int getchar(void)
{
	// TODO UART getchar
	return 0;
}

int putchar(int c)
{
	// TODO UART putchar
	return 0;
}

size_t strlen(const char *s)
{
	size_t len = 0;
	while (*s++ != '\0')
	{
		++len;
	}
	return len;
}
