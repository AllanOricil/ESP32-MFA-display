#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <mbedtls/md.h>
#include <mbedtls/pkcs5.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include "constants.h"

#ifdef __cplusplus
extern "C"
{
#endif

	void init_encryption(const char *salt);
	bool key_exists();
	void generate_key(const char *password);
	bool decrypt_text(const char *input, char *output, size_t output_len);
	bool encrypt_text(const char *input, char *output, size_t output_len);

#ifdef __cplusplus
}

#include <SPIFFS.h>
#include <esp_log.h>
#include "storage.hpp"

#endif

#endif // ENCRYPTION_H
