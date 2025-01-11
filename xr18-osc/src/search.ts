import { filter, firstValueFrom, timeout } from "rxjs";
import { Logger, getOwnIP } from "./helpers";
import { OscClient } from "./osc";
import { XInfo } from "./schema";
import { XR18 } from "./xr18";

export interface SearchMixOptions {
  localAddress: string;
  localPort?: number;

  broadcastAddress?: string;
  broadcastPort?: number;

  timeout?: number;

  log?: boolean;
  logLevel?: "log" | "debug" | "error";
}

export async function searchMix(options: SearchMixOptions): Promise<XInfo> {
  const logger = new Logger(XR18.name, options.log ? options.logLevel : "none");

  const localAddress = options.localAddress ?? getOwnIP({ family: "IPv4" });

  const broadcastAddress = options.broadcastAddress ?? localAddress.replace(/\d+$/, "255");

  const client = new OscClient({
    remoteAddress: broadcastAddress,
    localAddress: localAddress,
    log: options.log,
    logLevel: options.logLevel,
  });

  await client.start();

  await client.sendMessage("/xinfo");

  const xinfoMessage = await firstValueFrom(
    client.messages.pipe(filter((message) => message.address === "/xinfo")).pipe(timeout(options.timeout ?? 5000))
  );

  await client.close();

  const xinfo: XInfo = {
    ip: xinfoMessage.args[0] as string,
    name: xinfoMessage.args[1] as string,
    type: xinfoMessage.args[2] as string,
    version: xinfoMessage.args[3] as string,
  };

  logger.log(`Found ${xinfo.type} v${xinfo.version}: ${xinfo.name} (${xinfo.ip})`);

  return xinfo;
}
