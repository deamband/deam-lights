// sum.test.js
import { expect, test } from "vitest";
import { getOwnIP } from "./helpers";

test("finds IP automatically", () => {
  const ip = getOwnIP({ family: "IPv4" });

  expect(ip).toBeTruthy();
});
