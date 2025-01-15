import {
  object,
  array,
  string,
  number,
  boolean,
  type InferType,
  ValidationError,
} from "yup";
import { AxiosError, type AxiosInstance, type AxiosResponse } from "axios";
import { createAxiosInstance, type RetryConfig } from "@/utils/axios";

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
          return value ? /^[A-Z2-7]+=*$/.test(value.toUpperCase()) : false;
        }
      ),
    group: number()
      .typeError("Group must be a number")
      .integer("Group must be an integer.")
      .required("Group is required.")
      .strict(),
  })
);

export class ESP32MFAAuthenticatorClientValidationError extends Error {
  errors: string[];

  constructor(errors: string[]) {
    super("Validation error occurred");
    this.name = "ESP32MFAAuthenticatorClientValidationError";
    this.errors = errors;
  }
}

export interface ESP32MFAAuthenticationClientResponse extends AxiosResponse {}
export class ESP32MFAAuthenticatorClientError extends AxiosError {
  constructor(
    message: string,
    public code: string,
    public status: number,
    public request?: any,
    public response?: ESP32MFAAuthenticationClientResponse
  ) {
    super(message);
    this.name = "ESP32MFAAuthenticatorClientError";
    this.code = code;
    this.status = status;
    this.request = request;
    this.response = response;

    if (Error.captureStackTrace) {
      Error.captureStackTrace(this, ESP32MFAAuthenticatorClientError);
    }
  }
}

export interface Config {
  wifi: {
    ssid: String;
    password: String;
  };
  authentication: {
    pin: {
      hash: String;
      key: String;
    };
    unlock_attempts: Number;
  };
  display: {
    sleep_timeout: Number;
  };
  touch: {
    calibrate: Boolean;
  };
}

export interface Service {
  name: string;
  totp?: string;
  secret?: string;
  group: number;
}

export default class ESP32MFAAuthenticatorClient {
  private client: AxiosInstance;

  constructor(baseURL?: string) {
    this.client = createAxiosInstance(
      baseURL || "/api/v1",
      {
        "Content-Type": "application/json",
        // NOTE: this token is generated and replaced in the server
        "x-csrf-token": "{{CSRF_TOKEN}}",
      },
      5000,
      5
    );
  }

  public async login(username: string, password: string) {
    try {
      const response = await this.client.post("/auth/login", {
        username,
        password,
      });
      return response.data;
    } catch (error) {
      console.error("Error while loging in: ", error);
      if (error instanceof AxiosError) {
        throw new ESP32MFAAuthenticatorClientError(
          error.message,
          error.code ?? "UNKNOWN",
          error.status,
          error.request,
          error.response
        );
      }
      throw error;
    }
  }

  public async logout() {
    try {
      const response = await this.client.post("/auth/logout");
      return response.data;
    } catch (error) {
      console.error("Error while loging out: ", error);
      if (error instanceof AxiosError) {
        throw new ESP32MFAAuthenticatorClientError(
          error.message,
          error.code ?? "UNKNOWN",
          error.status,
          error.request,
          error.response
        );
      }
      throw error;
    }
  }

  public async validateSession() {
    try {
      const response = await this.client.post("/auth/validate");
      return response.data;
    } catch (error) {
      console.error("Error while validating session: ", error);
      if (error instanceof AxiosError) {
        throw new ESP32MFAAuthenticatorClientError(
          error.message,
          error.code ?? "UNKNOWN",
          error.status,
          error.request,
          error.response
        );
      }
      throw error;
    }
  }

  public async updateConfig(config: Config): Promise<Config> {
    try {
      console.debug("Validating configuration");
      await CONFIG_PUT_SCHEMA.validate(config, { abortEarly: false });
      console.debug("Updating configuration");
      const response = await this.client.put("/config", { config });
      return response.data as Config;
    } catch (error) {
      console.error("Error while updating configuration:", error);
      if (error instanceof ValidationError) {
        console.error("Validation error:", error.errors);
        throw new ESP32MFAAuthenticatorClientValidationError(error.errors);
      } else if (error instanceof AxiosError) {
        throw new ESP32MFAAuthenticatorClientError(
          error.message,
          error.code ?? "UNKNOWN",
          error.status,
          error.request,
          error.response
        );
      }

      throw error;
    }
  }

  public async fetchConfig(): Promise<Config> {
    try {
      console.debug("Fetching Configuration");
      const response = await this.client.get("/config");
      return response.data as Config;
    } catch (error) {
      console.error("Error while fetching configuration:", error);
      if (error instanceof AxiosError) {
        throw new ESP32MFAAuthenticatorClientError(
          error.message,
          error.code ?? "UNKNOWN",
          error.status,
          error.request,
          error.response
        );
      }
      throw error;
    }
  }

  public async fetchServices(): Promise<Service[]> {
    try {
      console.debug("Fetching Services");
      const response = await this.client.get("/services");
      return response.data as Service[];
    } catch (error) {
      console.error("Error while fetching services:", error);
      if (error instanceof AxiosError) {
        throw new ESP32MFAAuthenticatorClientError(
          error.message,
          error.code ?? "UNKNOWN",
          error.status,
          error.request,
          error.response
        );
      }
      throw error;
    }
  }

  public async updateServices(services: Service[]): Promise<Service[]> {
    try {
      console.debug("Validating services");
      await SERVICES_PUT_SCHEMA.validate(services, { abortEarly: false });
      console.debug("Updating services");
      const response = await this.client.put("/services");
      return response.data as Service[];
    } catch (error) {
      console.error("Error while updating services:", error);
      if (error instanceof ValidationError) {
        console.error("Validation error:", error.errors);
        throw new ESP32MFAAuthenticatorClientValidationError(error.errors);
      } else if (error instanceof AxiosError) {
        throw new ESP32MFAAuthenticatorClientError(
          error.message,
          error.code ?? "UNKNOWN",
          error.status,
          error.request,
          error.response
        );
      }

      throw error;
    }
  }

  public async newService(services: Service[]): Promise<Service[]> {
    try {
      console.debug("Validating services");
      await SERVICES_POST_SCHEMA.validate(services, { abortEarly: false });
      console.debug("Creating services");
      const response = await this.client.post("/services", { services });
      return response.data as Service[];
    } catch (error) {
      console.error("Error while creating service:", error);
      if (error instanceof ValidationError) {
        console.error("Validation error:", error.errors);
        throw new ESP32MFAAuthenticatorClientValidationError(error.errors);
      } else if (error instanceof AxiosError) {
        throw new ESP32MFAAuthenticatorClientError(
          error.message,
          error.code ?? "UNKNOWN",
          error.status,
          error.request,
          error.response
        );
      }

      throw error;
    }
  }
}
