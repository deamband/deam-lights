import asyncio

from pythonosc.dispatcher import Dispatcher
from pythonosc.osc_message_builder import OscMessageBuilder
from pythonosc.osc_server import BlockingOSCUDPServer

from typing import Optional, Union


class OSCClientServer(BlockingOSCUDPServer):
    def __init__(self, address: str, dispatcher: Dispatcher):
        super().__init__(("", 0), dispatcher)
        self.xr_address = address

    def send_message(self, address: str, vals: Optional[Union[str, list]]):
        builder = OscMessageBuilder(address=address)
        vals = vals if vals is not None else []
        if not isinstance(vals, list):
            vals = [vals]
        for val in vals:
            builder.add_arg(val)
        msg = builder.build()
        self.socket.sendto(msg.dgram, (self.xr_address, 10024))


async def main():

    # TODO: get local subnet from current IP
    address = "192.168.31.255"

    dispatcher = Dispatcher()
    dispatcher.set_default_handler(print)

    server = OSCClientServer(address, dispatcher)

    # search for the XR18
    print(f"Sending /xinfo to {address}")
    server.send_message("/xinfo", None)

    # start meters monitoring
    server.send_message("/meters", [0])

    server.serve_forever()


if __name__ == "__main__":
    asyncio.run(main())
