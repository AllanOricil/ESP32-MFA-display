#include "encryption.hpp"

static const char *TAG = "main";

const unsigned char *_salt;

bool key_exists()
{
	return is_file_available(SPIFFS, KEY_FILE_PATH);
}

void generate_key(const char *password)
{
	uint8_t key[KEY_SIZE];
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_md_context_t md_ctx;

	mbedtls_entropy_init(&entropy);
	mbedtls_ctr_drbg_init(&ctr_drbg);

	const char *personalization = "key_gen";
	int ret = mbedtls_ctr_drbg_seed(
		&ctr_drbg,
		mbedtls_entropy_func,
		&entropy,
		(const unsigned char *)personalization,
		strlen(personalization));
	if (ret != 0)
	{
		ESP_LOGD(TAG, "CTR-DRBG seed initialization failed with error: %d", ret);
		mbedtls_entropy_free(&entropy);
		return;
	}

	mbedtls_md_init(&md_ctx);
	ret = mbedtls_md_setup(
		&md_ctx,
		mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),
		0);
	if (ret != 0)
	{
		ESP_LOGE(TAG, "Failed to setup MD context");
		mbedtls_ctr_drbg_free(&ctr_drbg);
		mbedtls_entropy_free(&entropy);
		return;
	}

	ret = mbedtls_pkcs5_pbkdf2_hmac(
		&md_ctx,
		(const unsigned char *)password,
		strlen(password),
		_salt,
		SALT_SIZE,
		ITERATIONS,
		KEY_SIZE,
		key);
	if (ret != 0)
	{
		ESP_LOGE(TAG, "Key derivation failed with error: %d", ret);
	}
	else
	{
		ESP_LOGI(TAG, "Key successfully derived");
	}

	mbedtls_md_free(&md_ctx);
	mbedtls_ctr_drbg_free(&ctr_drbg);
	mbedtls_entropy_free(&entropy);

	File file = SPIFFS.open(KEY_FILE_PATH, FILE_WRITE);
	if (!file)
	{
		ESP_LOGD(TAG, "Failed to open file for writing");
		return;
	}

	if (file.write(key, KEY_SIZE) != KEY_SIZE)
	{
		ESP_LOGD(TAG, "Failed to write the entire key to SPIFFS");
	}
	else
	{
		ESP_LOGD(TAG, "Key successfully stored in SPIFFS");
	}
	file.close();
}

bool load_key(uint8_t *key)
{
	File keyFile = SPIFFS.open(KEY_FILE_PATH, FILE_READ);
	if (!keyFile)
	{
		ESP_LOGD(TAG, "Failed to open key file");
		return false;
	}

	if (keyFile.read(key, KEY_SIZE) != KEY_SIZE)
	{
		ESP_LOGD(TAG, "Failed to read the complete key");
		keyFile.close();
		return false;
	}

	keyFile.close();
	return true;
}

bool decrypt_text(const char *input, char *output, size_t output_len)
{
	size_t input_len = strlen(input);
	uint8_t key[KEY_SIZE];
	if (!load_key(key))
	{
		return false;
	}

	mbedtls_aes_context aes;
	mbedtls_aes_init(&aes);

	if (mbedtls_aes_setkey_dec(&aes, key, KEY_SIZE * 8) != 0)
	{
		ESP_LOGD(TAG, "Failed to set AES decryption key");
		mbedtls_aes_free(&aes);
		return false;
	}

	if (input_len % 16 != 0)
	{
		ESP_LOGD(TAG, "Input length must be a multiple of AES block size (16 bytes)");
		mbedtls_aes_free(&aes);
		return false;
	}

	for (size_t i = 0; i < input_len; i += 16)
	{
		if (mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT,
								  (const unsigned char *)(input + i),
								  (unsigned char *)(output + i)) != 0)
		{
			ESP_LOGD(TAG, "Decryption failed at block %zu", i / 16);
			mbedtls_aes_free(&aes);
			return false;
		}
	}

	if (input_len < output_len)
	{
		output[input_len] = '\0';
	}

	mbedtls_aes_free(&aes);
	return true;
}

bool encrypt_text(const char *input, char *output, size_t output_len)
{
	size_t input_len = strlen(input);
	uint8_t key[KEY_SIZE];
	if (!load_key(key))
	{
		return false;
	}

	mbedtls_aes_context aes;
	mbedtls_aes_init(&aes);

	if (mbedtls_aes_setkey_enc(&aes, key, KEY_SIZE * 8) != 0)
	{
		ESP_LOGD(TAG, "Failed to set AES encryption key");
		mbedtls_aes_free(&aes);
		return false;
	}

	size_t padded_len = (input_len + 15) / 16 * 16;
	if (padded_len > output_len)
	{
		ESP_LOGD(TAG, "Output buffer too small for encrypted data");
		mbedtls_aes_free(&aes);
		return false;
	}

	char *padded_input = (char *)malloc(padded_len);
	if (!padded_input)
	{
		ESP_LOGD(TAG, "Failed to allocate memory for padded input");
		mbedtls_aes_free(&aes);
		return false;
	}
	memcpy(padded_input, input, input_len);
	memset(padded_input + input_len, padded_len - input_len, padded_len - input_len);

	for (size_t i = 0; i < padded_len; i += 16)
	{
		if (mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT,
								  (const unsigned char *)(padded_input + i),
								  (unsigned char *)(output + i)) != 0)
		{
			ESP_LOGD(TAG, "Encryption failed at block %zu", i / 16);
			free(padded_input);
			mbedtls_aes_free(&aes);
			return false;
		}
	}

	free(padded_input);
	mbedtls_aes_free(&aes);
	return true;
}

void init_encryption(const char *salt)
{
	_salt = (const unsigned char *)salt;
}