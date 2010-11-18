#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Selective Repeat ARQ simulation.
"""

WINDOW_SIZE = 5
NETWORK_ERROR_RATE = 0.333


class Context:
    """
    Provides information about time (for simulating timeouts etc.).
    """

    def __init__(self):
        self.time = 0

    def get_time(self):
        return self.time

    def tick(self):
        self.time += 1

    def log(self, s):
        print "%4d  %s" % (self.get_time(), s)


class Network:
    """
    Simulates physical network.
    """

    def __init__(self, context):
        import random
        self.context = context
        self.frames = list()
        self.travelTime = 3
        self.random = random.Random(42)

    def send(self, src, target, frame):
        """
        Send a frame to target.
        """
        if self.random.uniform(0, 1) < NETWORK_ERROR_RATE:
            self.context.log("NETWORK: DROPPED - %s sends %s to %s" % (src, frame, target))
        else:
            self.context.log("NETWORK: %s sends %s to %s" % (src, frame, target))
            self.frames.append((self.context.get_time(), target, frame))

    def recv(self, side):
        """
        Are there any frames for this side?
        Returns list of frames.
        """
        received = list()
        for timeSent, target, frame in self.frames:
            if target != side:
                continue
            if timeSent >= self.context.get_time() - self.travelTime:
                continue
            received.append(frame)
        if received:
            self.frames = [(timeSent, target, frame)  # delete received frames from
                for (timeSent, target, frame) in self.frames  # self.frames
                    if frame not in received]
            self.context.log("NETWORK: %s received %s" % (side, received))
        return received


class Window:

    def __init__(self):
        self.size = WINDOW_SIZE
        self.data = dict()
        self.leastSeq = 0

    def full(self):
        return len(self.data) >= self.size

    def has(self, seq):
        return seq in self.data

    def add(self, seq, item):
        if self.full():
            raise Exception("Window is full")
        if seq in self.data:
            raise Exception("Already have this seq")
        if seq < self.leastSeq or seq >= self.leastSeq + self.size:
            raise Exception("Seq number out of bounds")
        self.data[seq] = item

    def pop(self, seq):
        if self.leastSeq != seq:
            raise Exception("Pop only from left")
        self.leastSeq += 1
        return self.data.pop(seq)

    def get(self, seq):
        try:
            return self.data[seq]
        except KeyError:
            raise Exception("%r not in %r" % (seq, self.data))


class Sender:

    def __init__(self, context, network, side, targetSide):
        self.context = context
        self.network = network
        self.side = side
        self.targetSide = targetSide
        self.messagesToSend = list()
        self.window = Window()
        self.ackSeq = -1
        self.nextSeq = 0
        self.timeout = 10

    def add_messages(self, messages):
        self.messagesToSend.extend(messages)

    def _next_seq(self):
        seq = self.nextSeq
        self.nextSeq += 1
        return seq

    def run(self):
        if not self.window.full() and self.messagesToSend:
            message = self.messagesToSend.pop(0)
            frame = {
                "seq": self._next_seq(),
                "message": message,
            }
            self.network.send(self.side, self.targetSide, frame)
            self.window.add(frame["seq"], {
                "firstSent": self.context.get_time(),
                "lastSent": self.context.get_time(),
                "message": message,
            })
            return

        resend = False

        frames = self.network.recv(self.side)
        for frame in frames:
            ack = frame["ack"]
            self.context.log("SENDER: got ack %s; ackSeq=%s" % (ack, self.ackSeq))
            if ack == self.ackSeq:
                # acked something that was acked already
                self.context.log("SENDER: resending seq %s" % (ack+1))
                resend = True
            else:
                while ack > self.ackSeq:
                    self.ackSeq += 1
                    self.window.pop(self.ackSeq)

        if not resend and \
            self.window.has(self.ackSeq+1) \
            and self.window.get(self.ackSeq+1)["lastSent"] < self.context.get_time() - self.timeout:

            resend = True

        if resend:
            data = self.window.get(self.ackSeq+1)
            frame = {
                "seq": self.ackSeq+1,
                "message": data["message"],
            }
            data["lastSent"] = self.context.get_time()
            self.network.send(self.side, self.targetSide, frame)


class Receiver:

    def __init__(self, context, network, side, senderSide):
        self.context = context
        self.network = network
        self.side = side
        self.senderSide = senderSide
        self.receivedMessages = list()
        self.window = Window()
        self.nextSeq = 0

    def get_received(self):
        return tuple(self.receivedMessages)

    def run(self):
        frames = self.network.recv(self.side)
        sendAck = False
        for frame in frames:
            sendAck = True
            if frame["seq"] >= self.nextSeq:
                self.window.add(frame["seq"], frame["message"])

        while self.window.has(self.nextSeq):
            self.receivedMessages.append(self.window.pop(self.nextSeq))
            self.nextSeq += 1
            assert sendAck == True

        if sendAck:
            frame = {
                "ack": self.nextSeq - 1,
            }
            self.network.send(self.side, self.senderSide, frame)


def main():
    context = Context()
    network = Network(context=context)
    sender = Sender(context=context, network=network, side="S", targetSide="R")
    receiver = Receiver(context=context, network=network, side="R", senderSide="S")

    messages = ("aaa", "bbb", "ccc", "ddd", "eee", "fff", "ggg", "hhh")
    sender.add_messages(messages)

    while True:
        # cooperative multitasking :)
        sender.run()
        receiver.run()
        context.tick()

        if context.get_time() > 10000:
            break

    received = receiver.get_received()
    print ""
    if received == messages:
        print "Transfer OK"
    else:
        print "Transfer failed"
        print "  Messages received:  %s" % str(received)
        print "  Should be received: %s" % str(messages)
        import sys
        sys.exit(1)



if __name__ == "__main__":
    main()

