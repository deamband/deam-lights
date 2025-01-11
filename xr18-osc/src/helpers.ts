import { NetworkInterfaceInfoIPv4, networkInterfaces } from "os";

export interface GetOwnInterfacesOptions {
  family: "IPv4" | "IPv6";
}

export function getOwnInterfaces(options: GetOwnInterfacesOptions): NetworkInterfaceInfoIPv4[] {
  const interfaceMap = networkInterfaces();

  const interfaces = Object.entries(interfaceMap).flatMap(([name, addresses]) => {
    if (!addresses) return [];

    return addresses
      .filter((address): address is NetworkInterfaceInfoIPv4 => address.family === options.family)
      .map((address) => ({ ...address, name }));
  });

  return interfaces;
}

export function getOwnIP(options: GetOwnInterfacesOptions): string {
  const interfaces = getOwnInterfaces(options);

  interfaces.sort((a, b) => {
    if (a.internal) return 1;
    if (b.internal) return -1;

    return a.netmask < b.netmask ? 1 : -1;
    return 0;
  });

  return interfaces[0].address;
}

export class Logger {
  constructor(private readonly prefix: string, private logLevel: "log" | "debug" | "error" | "none" = "log") {}

  log(message: string, ...args: unknown[]) {
    if (!["log", "debug"].includes(this.logLevel)) return;
    this.writeMessage("log", message, ...args);
  }

  debug(message: string, ...args: unknown[]) {
    if (!["debug"].includes(this.logLevel)) return;
    this.writeMessage("debug", message, ...args);
  }

  error(message: string, ...args: unknown[]) {
    if (!["error", "log", "debug"].includes(this.logLevel)) return;
    this.writeMessage("log", message, ...args);
  }

  writeMessage(method: "log" | "debug" | "error", message: string, ...args: unknown[]) {
    return console[method](`[${this.prefix}]: ${message}`, ...args);
  }
}

export function dbToPercentage(d: number) {
  /*
          Function courtesy of Patrick‐Gilles Maillot (see their X32 Documentation)
          “d” represents the dB float data. d:[-90, +10]
          “f” represents OSC float data. f: [0.0, 1.0]
       */
  let f = 0.0;
  if (d <= -90) f = 0.0;
  else if (d < -60) f = (d + 90) / 480;
  else if (d < -30) f = (d + 70) / 160;
  else if (d < -10) f = (d + 50) / 80;
  else if (d <= 10) f = (d + 30) / 40;
  /*
          f is now a fudged linear value between 0.0 and 1.0 for decibel values from -90 to 10.
          0.75 = 0dB, so given our highest values are 0 we want to scale it again slightly to give us a 0.0 to 1.0 value for -90dB to +0 dB
          0.375 should now be 0.5 for example
      */
  return f / 0.75;
}
