#include "config.hpp"

static const char *TAG = "config";

String Configuration::to_json_string(bool safe) const
{
	StaticJsonDocument<512> doc;

	JsonObject _wifi = doc.createNestedObject("wifi");
	_wifi["ssid"] = wifi.ssid;
	_wifi["password"] = safe ? "*********" : wifi.password;

	JsonObject _authentication = doc.createNestedObject("authentication");
	JsonObject _pin = _authentication.createNestedObject("pin");
	_pin["hash"] = safe ? "*********" : authentication.pin.hash;
	_pin["key"] = safe ? "*********" : authentication.pin.key;
	_authentication["unlock_attempts"] = authentication.unlock_attempts;

	JsonObject _display = doc.createNestedObject("display");
	_display["sleep_timeout"] = display.sleep_timeout;

	JsonObject _touch = doc.createNestedObject("touch");
	_touch["calibrate"] = touch.calibrate;

	JsonObject _manager = doc.createNestedObject("manager");
	JsonObject _manager_authentication = _manager.createNestedObject("authentication");
	_manager_authentication["username"] = manager.authentication.username;
	_manager_authentication["password"] = safe ? "*********" : manager.authentication.password;
	_manager_authentication["key"] = safe ? "*********" : manager.authentication.key;
	_manager_authentication["session_length"] = manager.authentication.session_length;

	JsonObject _encryption = doc.createNestedObject("encryption");
	_encryption["salt"] = safe ? "*********" : encryption.salt;

	String json;
	serializeJson(doc, json);
	return json;
}

Configuration Configuration::load()
{
	ESP_LOGI(TAG, "opening config.yml file");
	File file = SD.open(CONFIG_FILE_PATH);
	if (!file)
	{
		ESP_LOGE(TAG, "error while opening config.yml file");
		throw std::runtime_error("Error opening config file");
	}

	ESP_LOGI(TAG, "reading config.yml file");
	Configuration config;
	YAMLNode root = YAMLNode::loadStream(file);
	file.close();

	if (root.isNull())
	{
		ESP_LOGE(TAG, "config.yml file is empty or has invalid format");
		throw std::runtime_error("Invalid config file format");
	}

	if (root["wifi"].isMap())
	{
		if (root["wifi"]["ssid"].isNull())
		{
			ESP_LOGE(TAG, "wifi.ssid is empty");
			throw std::runtime_error("wifi ssid must be defined");
		}
		config.wifi.ssid = root.gettext("wifi:ssid");

		if (root["wifi"]["password"].isNull())
		{
			ESP_LOGE(TAG, "wifi.password is empty");
			throw std::runtime_error("wifi password must be defined");
		}
		config.wifi.password = root.gettext("wifi:password");
	}

	if (root["authentication"].isMap())
	{
		if (!root["authentication"]["unlock_attempts"].isNull())
		{
			int unlock_attempts = string_2_int(root.gettext("authentication:unlock_attempts"));
			config.authentication.unlock_attempts = unlock_attempts > 0 ? unlock_attempts
																		: MAX_UNLOCK_ATTEMPTS;
		}

		if (root["authentication"]["pin"].isMap())
		{
			if (!root["authentication"]["pin"]["hash"].isNull())
				config.authentication.pin.hash = root.gettext("authentication:pin:hash");

			if (!root["authentication"]["pin"]["key"].isNull())
				config.authentication.pin.key = root.gettext("authentication:pin:key");
		}
	}

	if (root["display"].isMap())
	{
		if (!root["display"]["sleep_timeout"].isNull())
		{
			int sleep_timeout = string_2_int(root.gettext("display:sleep_timeout"));
			config.display.sleep_timeout = sleep_timeout > 0 ? sleep_timeout : SLEEP_TIMEOUT;
		}
	}

	if (root["touch"].isMap())
	{
		const char *calibrate = root.gettext("touch:calibrate");
		config.touch.calibrate = calibrate && (strcmp(calibrate, "true") == 0 || strcmp(calibrate, "1") == 0);
	}

	if (root["manager"].isMap())
	{
		if (root["manager"]["authentication"].isMap())
		{
			if (!root["manager"]["authentication"]["username"].isNull())
			{
				config.manager.authentication.username = root.gettext("manager:authentication:username");
			}

			if (!root["manager"]["authentication"]["password"].isNull())
			{
				config.manager.authentication.password = root.gettext("manager:authentication:password");
			}

			if (!root["manager"]["authentication"]["key"].isNull())
			{
				config.manager.authentication.key = root.gettext("manager:authentication:key");
			}

			if (!root["manager"]["authentication"]["session_length"].isNull())
			{
				int session_length = string_2_int(root.gettext("manager:authentication:session_length"));
				config.manager.authentication.session_length = session_length > 0 ? session_length : MAX_MANAGER_SESSION_LENGTH;
			}
		}
	}

	if (root["encryption"].isMap())
	{
		if (root["encryption"]["salt"].isNull())
		{
			ESP_LOGE(TAG, "encryption.salt is empty");
			throw std::runtime_error("encryption salt must be defined");
		}

		if (strlen(root.gettext("encryption:salt")) < 8)
		{
			ESP_LOGE(TAG, "encryption.salt must be at least 8 characters long");
			throw std::runtime_error("encryption salt must be at least 8 characters long");
		}

		config.encryption.salt = root.gettext("encryption:salt");
	}

	ESP_LOGI(TAG, "config loaded successfully");
	return config;
}

Configuration Configuration::parse(const String &json_string)
{
	Configuration config;
	StaticJsonDocument<512> doc;
	DeserializationError error = deserializeJson(doc, json_string);
	if (error)
	{
		ESP_LOGE(TAG, "failed to parse json %s", error.c_str());
		throw std::runtime_error("Failed to parse JSON.");
	}

	if (!doc.containsKey("wifi"))
	{
		ESP_LOGE(TAG, "wifi settings are required");
		throw std::runtime_error("wifi settings are required.");
	}

	config.wifi.password = doc["wifi"]["password"].as<String>();
	if (!config.wifi.password)
	{
		ESP_LOGE(TAG, "wifi password is required");
		throw std::runtime_error("wifi password is required.");
	}

	config.wifi.ssid = doc["wifi"]["ssid"].as<String>();
	if (!config.wifi.ssid)
	{
		ESP_LOGE(TAG, "wifi ssid is required");
		throw std::runtime_error("wifi ssid is required.");
	}

	if (doc.containsKey("authentication"))
	{

		int unlock_attempts = doc["authentication"]["unlock_attempts"].as<int>();
		if (unlock_attempts)
		{
			config.authentication.unlock_attempts = unlock_attempts;
		}
		else
		{
			ESP_LOGD(TAG, "using %d", MAX_UNLOCK_ATTEMPTS);
		}

		if (doc["authentication"].containsKey("pin"))
		{
			config.authentication.pin.hash = doc["authentication"]["pin"]["hash"].as<String>();
			if (!config.authentication.pin.hash)
			{
				ESP_LOGE(TAG, "authetication pin hash is required");
				throw std::runtime_error("authetication pin hash is required.");
			}

			config.authentication.pin.key = doc["authentication"]["pin"]["key"].as<String>();
			if (!config.authentication.pin.key)
			{
				ESP_LOGE(TAG, "authentication pin key is required");
				throw std::runtime_error("authentication pin key is required.");
			}
		}
		else
		{
			ESP_LOGD(TAG, "no pin settings were found");
		}
	}
	else
	{

		ESP_LOGD(TAG, "no authentication settings were found");
	}

	if (doc.containsKey("display"))
	{
		config.display.sleep_timeout = doc["display"]["sleep_timeout"].as<int>();
	}
	else
	{

		ESP_LOGD(TAG, "no display settings were found");
	}

	if (doc.containsKey("touch"))
	{
		config.touch.calibrate = doc["touch"]["calibrate"].as<bool>();
	}
	else
	{

		ESP_LOGD(TAG, "no touch settings were found");
	}

	return config;
}

bool Configuration::is_authentication_configured()
{
	return !authentication.pin.hash.isEmpty() && !authentication.pin.key.isEmpty();
}

bool Configuration::is_manager_configured()
{
	return !manager.authentication.username.isEmpty() && !manager.authentication.password.isEmpty() && !manager.authentication.key.isEmpty();
}

bool Configuration::save() const
{
	ESP_LOGI(TAG, "saving config to file");
	File file = SD.open(CONFIG_FILE_PATH, FILE_WRITE);
	if (!file)
	{
		ESP_LOGE(TAG, "error opening config.yml file for writting");
		return false;
	}

	String config_json = to_json_string(false);
	YAML::StringStream json_input_stream(config_json);
	YAMLNode root = YAMLNode::loadStream(json_input_stream);
	serializeYml(root.getDocument(), file, OUTPUT_YAML);
	file.close();
	ESP_LOGI(TAG, "config stored successfully");
	return true;
}
