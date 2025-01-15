<template>
  <form class="space-y-4" @submit.prevent="onSubmit">
    <div class="mb-4">
      <h4 class="mb-3">Wi-Fi</h4>

      <div>
        <label for="ssid" class="form-label">SSID</label>
        <input
          type="text"
          class="form-control"
          id="ssid"
          v-model="settings.wifi.ssid"
        />
      </div>
      <div>
        <label for="password" class="form-label">Password</label>
        <input
          type="password"
          class="form-control"
          id="password"
          v-model="settings.wifi.password"
        />
      </div>
    </div>

    <div class="mb-4">
      <h4 class="mb-3">Authentication</h4>
      <label for="unlock-attempts" class="form-label">Unlock Attempts</label>
      <input
        type="number"
        class="form-control"
        id="unlock-attempts"
        min="1"
        v-model.number="settings.authentication.unlock_attempts"
      />
      <div>
        <label for="pin-hash" class="form-label">Pin Hash</label>
        <input
          type="password"
          class="form-control"
          id="pin-hash"
          v-model="settings.authentication.pin.hash"
        />
      </div>
      <div>
        <label for="pin-key" class="form-label">Pin Key</label>
        <input
          type="password"
          class="form-control"
          id="pin-key"
          v-model="settings.authentication.pin.key"
        />
      </div>
    </div>

    <div class="mb-4">
      <h4 class="mb-3">Display</h4>

      <div>
        <label for="sleep-timeout" class="form-label">
          Sleep Timeout (seconds)
        </label>
        <input
          type="number"
          class="form-control"
          id="sleep-timeout"
          min="0"
          v-model.number="settings.display.sleep_timeout"
        />
      </div>
    </div>

    <div class="mb-2">
      <h4 class="mb-3">Touch</h4>

      <input
        type="checkbox"
        class="form-check-input"
        id="touch-calibrate"
        v-model="settings.touch.calibrate"
      />
      <label class="form-check-label ps-2" for="touch-calibrate"
        >Calibrate</label
      >
    </div>

    <div class="d-grid">
      <button type="submit" data-bs-theme="dark" class="btn btn-light">
        Save
      </button>
    </div>
  </form>
</template>

<script lang="ts" setup>
import jsYaml from "js-yaml";
import { ref, onBeforeMount, inject } from "vue";
import ESP32MFAAuthenticatorClient, {
  ESP32MFAAuthenticatorClientValidationError,
  ESP32MFAAuthenticatorClientError,
  type Config,
} from "~/api/esp32-mfa-authenticator-client";

const showToast = inject("showToast") as Function;

const settings = ref<Config>({
  wifi: {
    ssid: undefined,
    password: undefined,
  },
  authentication: {
    pin: {
      key: undefined,
      hash: undefined,
    },
    unlock_attempts: 3,
  },
  display: {
    sleep_timeout: 5,
  },
  touch: {
    calibrate: false,
  },
});

async function onSubmit(event: SubmitEvent) {
  try {
    const client = new ESP32MFAAuthenticatorClient();
    await client.updateConfig({
      wifi: {
        ssid: settings.value.wifi.ssid,
        password: settings.value.wifi.password,
      },
      authentication: {
        pin: {
          hash: settings.value.authentication.pin.hash || "",
          key: settings.value.authentication.pin.key || "",
        },
        unlock_attempts: settings.value.authentication.unlock_attempts || 3,
      },
      display: {
        sleep_timeout: settings.value.display.sleep_timeout || "",
      },
      touch: {
        calibrate: settings.value.touch.calibrate ? 1 : 0,
      },
    });
    showToast("Configuration updated successfully");
  } catch (error) {
    console.error("Error while submiting settings form:", error);
    // NOTE: client-side validation
    if (error instanceof ESP32MFAAuthenticatorClientValidationError) {
      if (error.errors) {
        errors.value.push(...error.errors);
      } else {
        errors.value.push("something unknown happened");
      }
    }
    // NOTE: server-side response error
    else if (error instanceof ESP32MFAAuthenticatorClientError) {
      if (error.response?.data.code === "INVALID_BODY") {
        errors.value.push(...error.response?.data?.errors);
      } else {
        errors.value.push("something unknown happened");
      }
    } else {
      errors.value.push("something unknown happened");
    }
  }
}

onBeforeMount(async () => {
  try {
    const client = new ESP32MFAAuthenticatorClient();
    settings.value = await client.fetchConfig();
  } catch (error) {
    showToast("Failed to load settings", "error");
  }
});
</script>

<style scoped>
.toast {
  position: fixed;
  top: 20px;
  right: 20px;
  z-index: 1000;
}

.toast-body {
  width: 90%;
}
</style>
