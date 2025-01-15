<template>
  <div class="container mt-5 mb-5">
    <div class="row justify-content-center">
      <div class="col-md-6">
        <div class="d-flex justify-content-between align-items-center mb-4">
          <h1 class="mb-0">Services</h1>
          <div>
            <button class="btn btn-primary me-1" @click="showModal = true">
              New
            </button>
          </div>
        </div>
        <Totps></Totps>
      </div>
    </div>

    <div class="modal fade show" tabindex="-1" role="dialog" v-if="showModal">
      <div class="modal-dialog" role="document">
        <div class="modal-content">
          <div class="modal-header">
            <h5 class="modal-title">New Service</h5>
            <button
              type="button"
              class="btn-close"
              @click="showModal = false"
              aria-label="Close"
            ></button>
          </div>
          <div class="modal-body">
            <ServiceForm @close-modal="showModal = false" />
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script lang="ts" setup>
import Totps from "~/components/totps.vue";
import ServiceForm from "~/components/service-form.vue";

definePageMeta({
  middleware: ["auth"],
  layout: "authenticated",
});

const showModal = ref(false);
</script>

<style>
.modal.fade.show {
  display: block; /* Ensure the modal is visible */
  background-color: rgba(0, 0, 0, 0.5); /* Modal backdrop */
}
</style>
