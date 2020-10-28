#pragma once

#include <iostream>
#include <stdlib.h>
#include "curl/curl.h"

struct memory {
	char *response;
	size_t size;
};

static size_t cb(void *data, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct memory *mem = (struct memory *)userp;

	char *ptr = (char*)realloc(mem->response, mem->size + realsize + 1);
	if (ptr == NULL)
		return 0;  /* out of memory! */

	mem->response = ptr;
	memcpy(&(mem->response[mem->size]), data, realsize);
	mem->size += realsize;
	mem->response[mem->size] = 0;

	return realsize;
}