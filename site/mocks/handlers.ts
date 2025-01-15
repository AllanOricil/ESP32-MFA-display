import { http, HttpResponse } from "msw";
import {
  object,
  array,
  string,
  number,
  boolean,
  type InferType,
  ValidationError,
} from "yup";

const MAX_SESSION_TIME = 5;

const CONFIG_PUT_SCHEMA = object({
  wifi: object({
    ssid: string().required("SSID is required"),
    password: string().required("WiFi password is required"),
  }),
  authentication: object({
    pin: object({
      pin: string().nullable().notRequired(),
      hash: string().nullable().notRequired(),
    }),
    unlock_attempts: number()
      .default(3)
      .typeError("Unlock attempts must be a number")
      .positive("Unlock attempts must be positive")
      .integer("Unlock attempts must be an integer")
      .strict(),
  }),
  display: object({
    sleep_timeout: number().nullable().notRequired(),
  }),
  touch: object({
    calibrate: boolean().default(false),
  }),
});

const SERVICES_PUT_SCHEMA = array().of(
  object({
    name: string().required("Name is required."),
    group: number()
      .typeError("Group must be a number")
      .integer("Group must be an integer.")
      .required("Group is required.")
      .strict(),
  })
);

const SERVICES_POST_SCHEMA = array().of(
  object({
    name: string()
      .required("Name is required.")
      .max(60, "Name must be 60 characters or fewer."),
    secret: string()
      .required("Secret is required.")
      .test(
        "is-multiple-of-8",
        "Secret length must be a multiple of 8.",
        (value) => {
          return value.length % 8 === 0;
        }
      )
      .test(
        "is-base32",
        "Secret must be a valid Base32-encoded string.",
        (value) => {
          return /^[A-Z2-7]+=*$/.test(value);
        }
      ),
    group: number()
      .typeError("Group must be a number")
      .integer("Group must be an integer.")
      .required("Group is required.")
      .strict(),
  })
);

export const handlers = [
  http.post("/api/v1/auth/login", async () => {
    const date = new Date();
    date.setTime(date.getTime() + MAX_SESSION_TIME * 60 * 1000);

    const response = new HttpResponse(
      JSON.stringify({ message: "login successful" }),
      {
        status: 200,
        headers: {
          "Content-Type": "application/json",
          "Set-Cookie": `esp32_mfa_authenticator_session_id=03f46f43-a4b0-4911-9bea-719614df1188; Path=/esp32; Expires=${date.toUTCString()}; Max-Age=${MAX_SESSION_TIME * 60};`,
        },
      }
    );

    return response;
  }),
  http.post("/api/v1/auth/logout", async () => {
    const response = new HttpResponse(
      JSON.stringify({ message: "logout successful" }),
      {
        status: 200,
        headers: {
          "Content-Type": "application/json",
          "Set-Cookie":
            "esp32_mfa_authenticator_session_id=; Path=/esp32; Expires=0; Max-Age=0;",
        },
      }
    );

    return response;
  }),
  http.post("/api/v1/auth/validate", async () => {
    return HttpResponse.json({ message: "session is valid" }, { status: 200 });
  }),
  http.get("/api/v1/config", async () => {
    return HttpResponse.json(
      {
        wifi: {
          ssid: "test",
          password: "*******",
        },
        authentication: {
          pin: {
            hash: "*******",
            key: "*******",
          },
          unlock_attempts: 3,
        },
        display: {
          sleep_timeout: 10,
        },
        touch: {
          calibrate: true,
        },
      },
      { status: 200 }
    );
  }),
  http.put("/api/v1/config", async () => {
    const config = await request.json();

    try {
      await CONFIG_PUT_SCHEMA.validate(config, { abortEarly: false });

      return HttpResponse.json(
        {
          message: "services updated",
        },
        { status: 200 }
      );
    } catch (error) {
      if (error instanceof ValidationError) {
        console.error("Validation error:", error.errors);
        throw { type: "ValidationError", errors: error.errors };

        return HttpResponse.json(
          {
            errors: error.errors,
            code: "INVALID_BODY",
          },
          { status: 400 }
        );
      }

      return HttpResponse.json(
        {
          message: "something went wrong",
        },
        { status: 500 }
      );
    }
  }),

  http.get("/api/v1/services", async () => {
    return HttpResponse.json(
      [
        {
          name: "a",
          totp: "******",
          group: 1,
        },
        {
          name: "b",
          totp: "******",
          group: 1,
        },
        {
          name: "c",
          totp: "******",
          group: 2,
        },
        {
          name: "d",
          totp: "******",
          group: 3,
        },
      ],
      {
        status: 200,
      }
    );
  }),

  http.put("/api/v1/services", async () => {
    return HttpResponse.json(
      {
        message: "services updated",
      },
      { status: 200 }
    );
  }),

  http.post("/api/v1/services", async ({ request }) => {
    const { services } = await request.json();

    try {
      await SERVICES_POST_SCHEMA.validate(services, { abortEarly: false });

      return HttpResponse.json(
        {
          message: "services updated",
        },
        { status: 200 }
      );
    } catch (error) {
      if (error instanceof ValidationError) {
        console.error("Validation error:", error.errors);
        throw { type: "ValidationError", errors: error.errors };

        return HttpResponse.json(
          {
            errors: error.errors,
            code: "INVALID_BODY",
          },
          { status: 400 }
        );
      }

      return HttpResponse.json(
        {
          message: "something went wrong",
        },
        { status: 500 }
      );
    }
  }),
];
