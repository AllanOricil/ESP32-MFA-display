<template>
  <draggable v-model="services" class="row row-cols-2 g-2 align-items-stretch">
    <div class="col-6" v-for="service in services" :key="service.id">
      <Totp :service="service"></Totp>
    </div>
  </draggable>
</template>

<script lang="ts" setup>
import { ref, onBeforeMount, inject } from "vue";
import { VueDraggableNext } from "vue-draggable-next";
import Totp from "~/components/totp.vue";
import ESP32MFAAuthenticatorClient from "@/api/esp32-mfa-authenticator-client";

const name = "totps";

const services = ref([]);

const showToast = inject("showToast") as Function;

onBeforeMount(async () => {
  try {
    const client = new ESP32MFAAuthenticatorClient();
    services.value = await client.fetchServices();
  } catch (error) {
    console.error("Error while fetching services:", error);
    showToast("Could not load services");
  }
});
const draggable = VueDraggableNext;
</script>

<style></style>
