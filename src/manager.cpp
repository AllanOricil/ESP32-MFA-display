#include "manager.hpp"

static const char *TAG = "manager";

AsyncWebServer server(80);

// NOTE: we store the CSRF_TOKEN to validate it during requests made by the client
String CSRF_TOKEN = String(random(0xFFFFFFFF), HEX);

String get_cookie(AsyncWebServerRequest *request, const String &name)
{
	String cookies = request->header("Cookie");
	int start_pos = cookies.indexOf(name + "=");
	if (start_pos != -1)
	{
		start_pos += name.length() + 1;
		int end_pos = cookies.indexOf(";", start_pos);
		if (end_pos == -1)
		{
			end_pos = cookies.length();
		}
		return cookies.substring(start_pos, end_pos);
	}
	return "";
}

bool is_session_valid(AsyncWebServerRequest *request)
{
	String session_id = get_cookie(request, "esp32_mfa_authenticator_session_id");
	if (session_id != "")
	{
		return validate_session(session_id.c_str());
	}
	return false;
}

// NOTE: we are validating if the session belongs to the board
void is_authorized(
	AsyncWebServerRequest *request,
	const std::function<void(AsyncWebServerRequest *)> &handler)
{
	try
	{
		if (!is_session_valid(request))
		{
			request->send(401, "application/json", "{\"message\":\"session not found\"}");
			ESP_LOGI(TAG, "session not found");
			return;
		}

		handler(request);
	}
	catch (const std::exception &e)
	{
		ESP_LOGE(TAG, "error while validating the session: %s", e.what());
		request->send(500, "application/json", "{\"message\":\"something went wrong\"}");
	}
}

void is_csrf_valid(
	AsyncWebServerRequest *request,
	const std::function<void(AsyncWebServerRequest *)> &handler)
{
	try
	{
		String csrf_token = request->header("x-csrf-token");
		if (csrf_token != CSRF_TOKEN)
		{
			request->send(403, "application/json", "{\"message\":\"forbidden\"}");
			ESP_LOGI(TAG, "forbidden");
			return;
		}
		handler(request);
	}
	catch (const std::exception &e)
	{
		ESP_LOGE(TAG, "Error while validating the CSRF token: %s", e.what());
		request->send(500, "application/json", "{\"message\":\"something went wrong\"}");
	}
}

void init_manager(Configuration config, const char *local_network_ip)
{
	ESP_LOGI(TAG, "initializing manager server");

	if (
		!is_file_available(SPIFFS, "/index.html") ||
		!is_file_available(SPIFFS, "/esp32/login/index.html") ||
		!is_file_available(SPIFFS, "/esp32/services/index.html") ||
		!is_file_available(SPIFFS, "/esp32/settings/index.html") ||
		!is_file_available(SPIFFS, "/200.html") ||
		!is_file_available(SPIFFS, "/404.html") ||
		!is_file_available(SPIFFS, "/favicon.ico"))
	{
		ESP_LOGE(TAG, "one or more required files are missing. Server initializing aborted");
		return;
	}

	ESP_LOGD(TAG, "configuring routes");

	// NOTE: configuring page routes
	server.on(
		"/",
		HTTP_GET,
		[](AsyncWebServerRequest *request)
		{
			request->redirect("/esp32");
		});

	server.on(
		"/esp32",
		HTTP_GET,
		[](AsyncWebServerRequest *request)
		{
			request->send(SPIFFS, "/index.html", "text/html");
		});

	server.onNotFound(
		[](AsyncWebServerRequest *request)
		{
			request->send(SPIFFS, "/404.html", "text/html");
		});

	// NOTE: configuring assets routes
	server.serveStatic("/_nuxt/", SPIFFS, "/_nuxt/");

	// NOTE: here we add the csrf token in the client
	server.on(
		"/_nuxt/index.js",
		HTTP_GET,
		[](AsyncWebServerRequest *request)
		{
			File file = SPIFFS.open("/_nuxt/index.js", FILE_READ);
			if (!file)
			{
				request->send(500, "text/plain", "something went wrong");
				return;
			}

			String jsContent = file.readString();
			file.close();

			jsContent.replace("{{CSRF_TOKEN}}", CSRF_TOKEN);
			request->send(200, "application/javascript", jsContent);
		});

	server.on(
		"/favicon.ico",
		HTTP_GET,
		[](AsyncWebServerRequest *request)
		{
			request->send(SPIFFS, "/favicon.ico", "image/x-icon");
		});

	// NOTE: configuring api routes
	server.on(
		"/api/v1/auth/login",
		HTTP_POST,
		[](AsyncWebServerRequest *request) {},
		nullptr,
		[](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
		{
			try
			{
				String data_json = String((char *)data);
				StaticJsonDocument<256> doc;
				DeserializationError error = deserializeJson(doc, data_json);
				if (error)
				{
					ESP_LOGE(TAG, "JSON parsing failed: %s", error.c_str());
					request->send(400, "application/json", "{\"message\":\"invalid JSON\"}");
					return;
				}

				const char *username = doc["username"];
				const char *password = doc["password"];

				if (!username || !password)
				{
					ESP_LOGE(TAG, "missing username or password in JSON");
					request->send(400, "application/json", "{\"message\":\"username or password missing\"}");
					return;
				}

				session *session = authenticate(username, password);
				if (session != nullptr)
				{
					ESP_LOGI(TAG, "authentication successful, session token set");

					// NOTE: only this server can use this session token from js
					String cookie = "esp32_mfa_authenticator_session_id=" + String(session->session_id) + "; Expires=" + format_time_to_UTC_String(session->expiration) + "; Path=/; HttpOnly; SameSite=Strict";
					AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"message\":\"login successful\"}");
					response->addHeader("Set-Cookie", cookie);
					request->send(response);
				}
				else
				{
					ESP_LOGE(TAG, "invalid credentials");
					request->send(401, "application/json", "{\"message\":\"invalid credentials\"}");
				}
			}
			catch (const std::exception &e)
			{
				ESP_LOGE(TAG, "error while authenticating: %s", e.what());
				request->send(500, "application/json", "{\"message\":\"something went wrong\"}");
			}
		});

	server.on(
		"/api/v1/auth/logout",
		HTTP_POST,
		[](AsyncWebServerRequest *request)
		{
			try
			{
				destroy_session();
				String cookie = "esp32_mfa_authenticator_session_id=; Expires=0; Max-Age=0; Path=/; HttpOnly; SameSite=Strict";
				AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"message\":\"logout successful\"}");
				response->addHeader("Set-Cookie", cookie);
				request->send(response);
				ESP_LOGI(TAG, "logout successful, session token cleared");
			}
			catch (const std::exception &e)
			{
				ESP_LOGE(TAG, "error while logging out: %s", e.what());
				request->send(500, "application/json", "{\"message\":\"something went wrong\"}");
			}
		});

	server.on(
		"/api/v1/auth/validate",
		HTTP_POST,
		[](AsyncWebServerRequest *request)
		{
			try
			{
				String session_id = get_cookie(request, "esp32_mfa_authenticator_session_id");
				if (session_id != "")
				{
					if (validate_session(session_id.c_str()))
					{
						request->send(200, "application/json", "{\"message\":\"session valid\"}");
						ESP_LOGI(TAG, "session validated successfully");
					}
					else
					{
						request->send(401, "application/json", "{\"message\":\"session invalid\"}");
						ESP_LOGI(TAG, "session invalid");
					}
				}
				else
				{
					request->send(401, "application/json", "{\"message\":\"session not found\"}");
					ESP_LOGI(TAG, "session not found");
				}
			}
			catch (const std::exception &e)
			{
				ESP_LOGE(TAG, "error while validating session: %s", e.what());
				request->send(500, "application/json", "{\"message\":\"something went wrong\"}");
			}
		});

	server.on(
		"/api/v1/config",
		HTTP_GET,
		[config](AsyncWebServerRequest *request)
		{
			is_authorized(
				request,
				[config](AsyncWebServerRequest *request)
				{
					is_csrf_valid(
						request,
						[config](AsyncWebServerRequest *request)
						{
							try
							{
								request->send(200, "application/json", config.to_json_string(true));
								ESP_LOGI(TAG, "session is valid and can read config");
							}
							catch (const std::exception &e)
							{
								ESP_LOGE(TAG, "error while fetching config: %s", e.what());
								request->send(500, "application/json", "{\"message\":\"something went wrong\"}");
							}
						});
				});
		});

	server.on(
		"/api/v1/config",
		HTTP_PUT,
		[](AsyncWebServerRequest *request) {},
		nullptr,
		[config](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
		{
			is_authorized(
				request,
				[config, data](AsyncWebServerRequest *request)
				{
					is_csrf_valid(
						request,
						[config](AsyncWebServerRequest *request)
						{
							try
							{
								String data_json = String((char *)data);
								Configuration new_config = Configuration::parse(data_json);
								new_config.manager = config.manager;
								if (new_config.save())
								{
									ESP_LOGI(TAG, "config updated successfully");
									request->send(200, "application/json", "{\"message\":\"configuration updated\"}");
								}
							}
							catch (const std::exception &e)
							{
								ESP_LOGE(TAG, "error while updating config: %s", e.what());
								request->send(500, "application/json", "{\"message\":\"something went wrong\"}");
							}
						});
				});
		});

	ESP_LOGD(TAG, "server routes configured");

	// NOTE: these are some of helmet headers I got from one of many of my servers built with express
	DefaultHeaders::Instance().addHeader("X-Content-Type-Options", "nosniff");
	DefaultHeaders::Instance().addHeader("X-Frame-Options", "DENY");
	DefaultHeaders::Instance().addHeader("X-XSS-Protection", "1; mode=block");
	// TODO: add this if config.manager.baseURL is set with a reverse proxy one ? have to try
	// DefaultHeaders::Instance().addHeader("Strict-Transport-Security", "max-age=31536000; includeSubDomains; preload");
	DefaultHeaders::Instance().addHeader("Referrer-Policy", "no-referrer");
	DefaultHeaders::Instance().addHeader("Permissions-Policy", "geolocation=(), microphone=()");

	// TODO: allow configuring this using config.manager.baseURL
	DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", String("http://") + String(local_network_ip));
	DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, PUT, OPTIONS");
	DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type, Cookie");
	DefaultHeaders::Instance().addHeader("Access-Control-Allow-Credentials", "true");
	DefaultHeaders::Instance().addHeader("Access-Control-Max-Age", "3600");

	server.begin();

	ESP_LOGI(TAG, "server initialized");
}