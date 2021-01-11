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
	char *ret = dst;

	while (*dst)
		dst += 1;
	while ((*dst++ = *src++))
		;

	return ret;
}

int strcmp(const char *s1, const char *s2) {
	assert(s1 != NULL && s2 != NULL);

	while (*s1 && *s2 && (*s1 == *s2)) {
		s1 += 1;
		s2 += 1;
	}

	if (*s1 == *s2)
		return 0;
	else if (*s1 > *s2)
		return 1;

	return -1;
}

int strncmp(const char *s1, const char *s2, size_t n) {
	if (!n)
		return 0;
	while (--n && *s1 && (*s1 == *s2)) {
		s1++;
		s2++;
	}

	return *(unsigned char *)s1 - *(unsigned char *)s2;
}

void *memset(void *v, int c, size_t n) {
	char *ret = (char *)v;
	assert(v != NULL || n > 0);
	while (n--)
		*ret++ = c;
	return v;
}

void *memcpy(void *out, const void *in, size_t n) {
	char *csrc = (char *)in, *cdest = (char *)out;
	for (int i = 0; i < n; i++) {
		cdest[i] = csrc[i];
	}
	return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
	assert(s1 != NULL || s2 != NULL || n > 0);
	unsigned char *csrc = (unsigned char *)s2, *cdest = (unsigned char *)s1;
	for (int i = 0; i < n; i++) {
		if (csrc[i] < cdest[i])
			return -1;
		if (csrc[i] > cdest[i])
			return 1;
	}
	return 0;
}

#endif
