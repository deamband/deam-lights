import { getOwnIP, searchMix, XR18 } from "@deamband/xr18-osc";
import { dmxnet } from "dmxnet";
import { tap } from "rxjs/operators";

async function main() {
  const ip = getOwnIP({ family: "IPv4" });

  const mix = await searchMix({ localAddress: ip, log: true, logLevel: "debug" });

  const xr18 = new XR18(mix, { localAddress: ip });

  await xr18.connect();

  var artnet = new dmxnet({ log: { silent: true } });

  // TODO: update library to send only used channels, not all 512
  var sender = artnet.newSender({
    universe: 10,
  });

  xr18.meters
    .pipe(
      tap((levels) => {
        const level = levels["auxL"];
        process.stdout.write("".padEnd(100 * level, "#").padEnd(100, " ") + level + "\r");
      })
    )
    .subscribe((levels) => {
      Object.keys(levels).forEach((key, i) => {
        sender.prepChannel(i, levels[<keyof typeof levels>key] * 255);
      });

      sender.transmit();
    });

  // await xr18.close();
}

main();
