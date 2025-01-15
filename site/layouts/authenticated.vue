<template>
  <div>
    <Navbar />
    <Toast ref="toast" />
    <slot />
  </div>
</template>

<script lang="ts" setup>
import Toast from "@/components/toast.vue";
import { ref, provide } from "vue";

const toast = ref(null);

const showToast = (message: string, type: string = "bg-success") => {
  if (toast.value) {
    toast.value.show(message, type);
  }
};

provide("showToast", showToast);
</script>

<style></style>
