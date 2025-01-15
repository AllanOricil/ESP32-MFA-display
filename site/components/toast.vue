<template>
  <div
    v-if="visible"
    class="toast align-items-center border-0 show"
    role="alert"
    aria-live="assertive"
    aria-atomic="true"
    @transitionend="onTransitionEnd"
  >
    <div class="d-flex">
      <div class="toast-body" :class="toastClass">
        {{ message }}
      </div>
      <button
        type="button"
        class="btn-close me-2 m-auto"
        @click="hideToast"
        aria-label="Close"
      ></button>
    </div>
  </div>
</template>

<script lang="ts" setup>
import { ref, watchEffect } from "vue";

const visible = ref(false);
const message = ref("");
const toastClass = ref("");

const show = (msg: string, type: "success" | "error" = "success") => {
  message.value = msg;
  toastClass.value = type === "success" ? "bg-success" : "bg-error";
  visible.value = true;
  setTimeout(() => {
    visible.value = false;
  }, 3000);
};

const hideToast = () => {
  visible.value = false;
};

const onTransitionEnd = () => {
  if (!visible.value) {
    message.value = "";
    toastClass.value = "";
  }
};

defineExpose({
  show,
});
</script>

<style scoped>
.toast {
  position: fixed;
  top: 70px;
  right: 20px;
  z-index: 9999;
  min-width: 250px;
  opacity: 0;
  visibility: hidden;
  transition:
    opacity 0.5s ease,
    visibility 0.5s ease;
}

.toast.show {
  opacity: 1;
  visibility: visible;
}

.toast-body {
  color: white;
}

.bg-success {
  background-color: #28a745 !important;
}

.bg-error {
  background-color: #dc3545 !important;
}

.bg-info {
  background-color: #17a2b8 !important;
}

.bg-warning {
  background-color: #ffc107 !important;
}
</style>
