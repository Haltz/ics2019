#include "klib.h"

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
	if (s == NULL)
		return 0;

	size_t len = 0;
	while ((*s++) != '\0')
		len++;

	return len;
}

char *strcpy(char *dst, const char *src) {
	if (dst == NULL || src == NULL)
		return dst;

	char *t = dst;
	while ((*t++ = *src++) != '\0')
		;

	return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
	if (dst == NULL || src == NULL)
		return dst;

	char *t = dst;
	size_t i = 0;
	for (i = 0; i < n && (*src != '\0'); i++) {
		*t++ = *src++;
	}

	return dst;
}

char *strcat(char *dst, const char *src) {
	return NULL;
}

int strcmp(const char *s1, const char *s2) {
	return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
	return 0;
}

void *memset(void *v, int c, size_t n) {
	return NULL;
}

void *memcpy(void *out, const void *in, size_t n) {
	return NULL;
}

int memcmp(const void *s1, const void *s2, size_t n) {
	return 0;
}

#endif
