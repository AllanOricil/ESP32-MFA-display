<template>
  <form @submit.prevent="onSubmit">
    <div class="mb-4">
      <label for="name" class="form-label">Name</label>
      <input
        type="text"
        id="name"
        class="form-control"
        v-model="formData.name"
        maxlength="60"
        required
      />
    </div>
    <div class="mb-4">
      <label for="secret" class="form-label">Secret</label>
      <input
        type="text"
        id="secret"
        class="form-control"
        v-model="formData.secret"
        required
        @input="validateSecret"
      />
      <small v-if="!isSecretValid" class="text-danger">
        Secret must be a multiple of 8 characters (e.g., 8, 16, 32).
      </small>
    </div>
    <div class="mb-4">
      <label for="group" class="form-label">Group</label>
      <input
        type="number"
        id="group"
        class="form-control"
        v-model.number="formData.group"
        min="0"
        required
      />
    </div>
    <div class="d-grid">
      <button type="submit" class="btn btn-light" :disabled="!isFormValid">
        Save
      </button>
    </div>

    <div v-if="errors.length" class="mt-3">
      <ul class="alert alert-danger mt-3">
        <li v-for="(error, index) in errors" :key="index">{{ error }}</li>
      </ul>
    </div>
  </form>
</template>

<script lang="ts" setup>
import { AxiosError } from "axios";
import { ref, computed, defineEmits, inject } from "vue";
import ESP32MFAAuthenticatorClient, {
  ESP32MFAAuthenticatorClientValidationError,
  ESP32MFAAuthenticatorClientError,
  Service,
} from "~/api/esp32-mfa-authenticator-client";

const formData = ref<Service>({
  name: "",
  secret: "",
  group: 1,
});

const showToast = inject("showToast") as Function;

const isSecretValid = ref(true);
const errors = ref<string[]>([]);

function validateSecret() {
  isSecretValid.value =
    formData.value.secret.length % 8 === 0 &&
    /^[A-Z2-7]+=*$/.test(formData.value.secret);
}

const isFormValid = computed(() => {
  return (
    formData.value.name.trim().length > 0 &&
    formData.value.secret.trim().length > 0 &&
    isSecretValid.value &&
    formData.value.group > 0
  );
});

const emit = defineEmits(["closeModal"]);
async function onSubmit() {
  errors.value = [];
  try {
    const client = new ESP32MFAAuthenticatorClient();
    await client.newService([formData.value]);
    emit("closeModal");
    showToast("New service created successfully");
  } catch (error) {
    console.error("Error while submiting service form:", error);

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
</script>

<style scoped></style>
