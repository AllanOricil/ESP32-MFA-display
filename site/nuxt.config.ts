export default defineNuxtConfig({
  title: "esp32-mfa-authenticator",
  theme: {
    dark: true,
    colors: {
      primary: "#ff0000",
    },
  },
  buildId: process.env.NUXT_BUILD_ID,
  devtools: { enabled: true },
  ssr: false,
  vue: {
    compilerOptions: {
      isCustomElement: (tag) =>
        ["esp-web-install-button"].some((prefix) => tag.startsWith(prefix)),
    },
  },
  modules: ["@pinia/nuxt", "pinia-plugin-persistedstate/nuxt"],
  app: {
    head: {
      link: [
        {
          rel: "stylesheet",
          href: "https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css",
        },
      ],
      script: [
        {
          src: "https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js",
          tagPosition: "bodyClose",
        },
      ],
    },
  },
  vite: {
    build: {
      rollupOptions: {
        output: {
          // NOTE: https://github.com/rollup/rollup/issues/2756#issuecomment-951370238
          // NOTE: a single js because esp32 server can't handle serving multiple ones in parallel, consistently. It throws an exception or fail to serve a partition.
          manualChunks: () => "everything.js",
          // NOTE: I had to prefix _nuxt because the js was not appearing in .output/public/_nuxt and I need it to be in that location because of the csrf token injection process in the server
          entryFileNames: () => "_nuxt/index.js",
        },
      },
    },
  },
  compatibilityDate: "2025-01-01",
});
