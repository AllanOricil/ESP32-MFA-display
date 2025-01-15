#include "encryption.hpp"

static const char *TAG = "main";

uint8_t _salt[SALT_SIZE] = {0};
uint8_t _key[KEY_SIZE] = {0};

bool key_exists()
{
	return is_file_available(SPIFFS, KEY_FILE_PATH);
}

bool generate_key(const char *password)
{
	ESP_LOGI(TAG, "generating derived key");
	uint8_t salt[SALT_SIZE];
	uint8_t key[KEY_SIZE];
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_md_context_t md_ctx;

	mbedtls_entropy_init(&entropy);
	mbedtls_ctr_drbg_init(&ctr_drbg);
	mbedtls_md_init(&md_ctx);

	// TODO: can this be static?
	const char *ctr_drbg_personalization = "key_derivation";
	int ret = mbedtls_ctr_drbg_seed(
		&ctr_drbg,
		mbedtls_entropy_func,
		&entropy,
		(const unsigned char *)ctr_drbg_personalization,
		strlen(ctr_drbg_personalization));
	if (ret != 0)
	{
		ESP_LOGE(TAG, "Failed to seed the CTR_DRBG: %d", ret);
		mbedtls_md_free(&md_ctx);
		mbedtls_ctr_drbg_free(&ctr_drbg);
		mbedtls_entropy_free(&entropy);
		return false;
	}

	ret = mbedtls_ctr_drbg_random(
		&ctr_drbg,
		salt,
		sizeof(salt));
	if (ret != 0)
	{
		ESP_LOGE(TAG, "Salt generation failed: %d", ret);
		mbedtls_md_free(&md_ctx);
		mbedtls_ctr_drbg_free(&ctr_drbg);
		mbedtls_entropy_free(&entropy);
		return false;
	}

	const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
	if (!md_info)
	{
		ESP_LOGE(TAG, "Failed to get MD info for SHA256");
		mbedtls_md_free(&md_ctx);
		mbedtls_ctr_drbg_free(&ctr_drbg);
		mbedtls_entropy_free(&entropy);
		return false;
	}

	ret = mbedtls_md_setup(
		&md_ctx,
		md_info,
		1);
	if (ret != 0)
	{
		ESP_LOGE(TAG, "Failed to setup MD context");
		mbedtls_md_free(&md_ctx);
		mbedtls_ctr_drbg_free(&ctr_drbg);
		mbedtls_entropy_free(&entropy);
		return false;
	}

	ESP_LOGD(TAG, "password: %s", password);
	ESP_LOGD(TAG, "salt: %s", salt);

	if (!salt || !password)
	{
		ESP_LOGE(TAG, "Salt or password is empty");
		mbedtls_md_free(&md_ctx);
		mbedtls_ctr_drbg_free(&ctr_drbg);
		mbedtls_entropy_free(&entropy);
		return false;
	}

	ret = mbedtls_pkcs5_pbkdf2_hmac(
		&md_ctx,
		(const unsigned char *)password,
		strlen(password),
		salt,
		sizeof(salt),
		ITERATIONS,
		KEY_SIZE,
		key);
	if (ret != 0)
	{
		ESP_LOGE(TAG, "Key derivation failed with error: %d", ret);
		mbedtls_md_free(&md_ctx);
		mbedtls_ctr_drbg_free(&ctr_drbg);
		mbedtls_entropy_free(&entropy);
		return false;
	}

	ESP_LOGI(TAG, "Key created");
	for (size_t i = 0; i < KEY_SIZE; ++i)
	{
		ESP_LOGD(TAG, "Key byte %d: 0x%02x", i, key[i]);
	}

	mbedtls_md_free(&md_ctx);
	mbedtls_ctr_drbg_free(&ctr_drbg);
	mbedtls_entropy_free(&entropy);

	File file = SPIFFS.open(KEY_FILE_PATH, FILE_WRITE);
	if (!file)
	{
		ESP_LOGE(TAG, "Failed to open file for writing");
		return false;
	}

	if (file.write(key, KEY_SIZE) != KEY_SIZE)
	{
		ESP_LOGE(TAG, "Failed to write key to SPIFFS");
		file.close();
		return false;
	}

	ESP_LOGI(TAG, "Key stored in SPIFFS");
	file.close();

	ESP_LOGI(TAG, "Encoding salt to Base32");
	uint8_t *encoded_salt = NULL;
	Base32 base32;
	int encoded_salt_len = base32.toBase32(salt, sizeof(salt), (uint8_t *&)encoded_salt, true);
	encoded_salt[encoded_salt_len] = '\0';
	ESP_LOGD(TAG, "Base32 Encoded Salt: %s", reinterpret_cast<char *>(encoded_salt));

	ESP_LOGI(TAG, "Storing salt in config.yml");
	Configuration config = Configuration::load();
	config.encryption.salt = String(reinterpret_cast<char *>(encoded_salt));
	ESP_LOGD(TAG, "Base32 encoded salt %s", config.encryption.salt.c_str());
	config.save();

	ESP_LOGI(TAG, "derived key generated successfully");

	return true;
}

bool load_key(uint8_t *key)
{
	ESP_LOGI(TAG, "loading key from SPIFFS");
	File file = SPIFFS.open(KEY_FILE_PATH, FILE_READ);
	if (!file)
	{
		ESP_LOGD(TAG, "Failed to open key file");
		return false;
	}

	if (file.read(key, KEY_SIZE) != KEY_SIZE)
	{
		ESP_LOGD(TAG, "Failed to read the key");
		file.close();
		return false;
	}

	file.close();
	ESP_LOGI(TAG, "Key loaded successfully");
	return true;
}

bool decrypt_text(const char *input, char *output, size_t output_len)
{
	size_t input_len = strlen(input);

	mbedtls_aes_context aes;
	mbedtls_aes_init(&aes);

	if (mbedtls_aes_setkey_dec(&aes, _key, KEY_SIZE * 8) != 0)
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
	mbedtls_aes_context aes;
	mbedtls_aes_init(&aes);

	if (mbedtls_aes_setkey_enc(&aes, _key, KEY_SIZE * 8) != 0)
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

void init_encryption(const char *encoded_salt)
{
	ESP_LOGI(TAG, "initializing encryption");

	size_t encoded_salt_length = strlen(encoded_salt);
	// NOTE: if the salt isn't empty it must be decoded, and the key loaded from SPIFFS
	if (encoded_salt_length)
	{
		ESP_LOGD(TAG, "decoding salt: %s", encoded_salt);
		uint8_t *decoded_salt = new uint8_t[SALT_SIZE];
		memset(decoded_salt, 0, SALT_SIZE);
		Base32 base32;
		int decoded_salt_length = base32.fromBase32(
			reinterpret_cast<byte *>(const_cast<char *>(encoded_salt)),
			encoded_salt_length,
			decoded_salt);
		memset(_salt, 0, SALT_SIZE);
		memcpy(_salt, decoded_salt, decoded_salt_length);
		delete[] decoded_salt;
		for (size_t i = 0; i < SALT_SIZE; ++i)
		{
			ESP_LOGD(TAG, "byte %d: 0x%02x", i, _salt[i]);
		}
		ESP_LOGD(TAG, "salt decoded successfully");

		load_key(_key);
		for (size_t i = 0; i < KEY_SIZE; ++i)
		{
			ESP_LOGD(TAG, "byte %d: 0x%02x", i, _key[i]);
		}
	}

	ESP_LOGI(TAG, "encryption initialized");
}