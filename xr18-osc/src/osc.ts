import { Subject } from "rxjs";
import { Logger } from "./helpers";
// @ts-ignore
import { UDPPort } from "osc";
import { IncomingMessage, MessageArg } from "./schema";

export interface OscClientOptions {
  remoteAddress: string;
  localAddress: string;
  localPort?: number;
  remotePort?: number;

  log?: boolean;
  logLevel?: "log" | "debug" | "error";
}

export class OscClient {
  private readonly logger = new Logger("OSC", this.options.log ? this.options.logLevel : "none");

  private udpPort?: UDPPort;

  readonly messages = new Subject<IncomingMessage>();

  private connected = false;

  constructor(private readonly options: OscClientOptions) {}

  get isConnected() {
    return this.connected;
  }

  async start() {
    await new Promise<void>((resolve, reject) => {
      const udpPort = new UDPPort({
        localAddress: this.options.localAddress,
        localPort: this.options.localPort ?? 0,
        remoteAddress: this.options.remoteAddress,
        remotePort: this.options.remotePort ?? 10024,
      });

      udpPort.on("ready", resolve);

      udpPort.on("message", this.handleMessage.bind(this));

      udpPort.on("error", (err: unknown) => reject(err));

      this.udpPort = udpPort;

      udpPort.open();
    });

    this.connected = true;
    this.logger.log("Connection established");
  }

  async sendMessage(path: string, ...args: MessageArg[]) {
    if (!this.connected) throw new Error("Client not connected");

    this.udpPort.send({ address: path, args });

    this.logger.debug(
      `< ${path}`,
      args.map((arg) => arg.value)
    );
  }

  async close() {
    this.connected = false;
    this.udpPort.close();
    this.messages.complete();

    this.logger.log("Connection closed");
  }

  private handleMessage(message: IncomingMessage, rinfo: RequestInfo) {
    this.logger.debug(`> ${message.address}`, message.args.join(" "));

    this.messages.next(message);
  }
}
