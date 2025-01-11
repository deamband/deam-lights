import { filter, map, Observable, share, skipWhile, timer } from "rxjs";
import { dbToPercentage, Logger } from "./helpers";
import { OscClient } from "./osc";
import { MeterLevels, XInfo } from "./schema";

export interface XR18Options {
  localAddress: string;
  localPort?: number;

  log?: boolean;
  logLevel?: "log" | "debug" | "error";

  metersInDecibels?: boolean;
}

export class XR18 {
  private readonly logger = new Logger(XR18.name, this.options.log ? this.options.logLevel : "none");

  private readonly client: OscClient;

  readonly meters = new Observable<MeterLevels>((subscriber) => {
    const metersSubscription = this.client.messages
      .pipe(filter((message) => message.address === "/meters/1"))
      .pipe(map((message) => this.parseMeters(message.args[0] as Uint8Array)))
      .subscribe((message) => {
        subscriber.next(message);
      });

    const intervalSubscription = timer(0, 9000)
      .pipe(skipWhile(() => !this.client.isConnected))
      .subscribe(() => this.client.sendMessage("/meters", { type: "s", value: "/meters/1" }));

    return () => {
      intervalSubscription.unsubscribe();
      metersSubscription.unsubscribe();
    };
  }).pipe(share());

  constructor(private readonly xinfo: XInfo, private readonly options: XR18Options) {
    this.client = new OscClient({
      remoteAddress: this.xinfo.ip,
      localAddress: this.options.localAddress,
      log: this.options.log,
      logLevel: this.options.logLevel,
    });
  }

  async connect() {
    await this.client.start();
  }

  async close() {
    await this.client.close();
  }

  private parseMeters(values: Uint8Array) {
    let thisBuffer = Buffer.from(values);

    let levels: MeterLevels = {
      //Output structure
      1: 0.0, //channel 1 - prefade
      2: 0.0, //channel 2 - prefade
      3: 0.0, //channel 3 - prefade
      4: 0.0, //channel 4 - prefade
      5: 0.0, //channel 5 - prefade
      6: 0.0, //channel 6 - prefade
      7: 0.0, //channel 7 - prefade
      8: 0.0, //channel 8 - prefade
      9: 0.0, //channel 9 - prefade
      10: 0.0, //channel 10 - prefade
      11: 0.0, //channel 11 - prefade
      12: 0.0, //channel 12 - prefade
      13: 0.0, //channel 13 - prefade
      14: 0.0, //channel 14 - prefade
      15: 0.0, //channel 15 - prefade
      16: 0.0, //channel 16 - prefade
      auxL: 0.0, //aux in channel - prefade (left)
      auxR: 0.0, //aux in channel - prefade (right)
      fx1PreL: 0.0, //Effect prefade
      fx1PreR: 0.0,
      fx2PreL: 0.0,
      fx2PreR: 0.0,
      fx3PreL: 0.0,
      fx3PreR: 0.0,
      fx4PreL: 0.0,
      fx4PreR: 0.0,
      bus1Pre: 0.0, //Bus prefade
      bus2Pre: 0.0,
      bus3Pre: 0.0,
      bus4Pre: 0.0,
      bus5Pre: 0.0,
      bus6Pre: 0.0,
      fx1SendPre: 0.0, //Effect send prefade
      fx2SendPre: 0.0,
      fx3SendPre: 0.0,
      fx4SendPre: 0.0,
      mainPostL: 0.0, //Main mix out postfade (left)
      mainPostR: 0.0, //Main mix out postfade (right)
      monL: 0.0, //Monitor out (left)
      monR: 0.0, //Monitor out (right)
    };

    Object.keys(levels).forEach((key, i) => {
      let value = thisBuffer.readIntLE(i * 2 + 4, 2); //Convert the two bytes into a signed int
      value = value / 256; //Convert to float as decibels

      if (!this.options.metersInDecibels) value = dbToPercentage(value);

      levels[<keyof typeof levels>key] = value;
    });
    return levels;
  }
}
