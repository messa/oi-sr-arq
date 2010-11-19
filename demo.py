#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Selective Repeat ARQ simulation.
"""

WINDOW_SIZE = 5
NETWORK_DELAY_TIME = 3
NETWORK_ERROR_RATE = 0.2
ACK_WAIT_TIMEOUT = 10


class Context:
    """
    Provides information about time (for simulating network delays, timeouts etc.),
    also handles logging.
    """

    def __init__(self):
        self.time = 0
        self.lastTimeLogged = None

    def get_time(self):
        return self.time

    def tick(self):
        self.time += 1

    def log(self, s):
        if self.lastTimeLogged == self.get_time():
            t = ""
        else:
            t = self.get_time()
            self.lastTimeLogged = t
        print "%4s  %s" % (t, s)


class Network:
    """
    Simulates physical network.
    """

    def __init__(self, context):
        import random
        self.context = context
        self.frames = list()
        self.random = random.Random(42)

    def send(self, src, target, frame):
        """
        Send a frame to target.
        """
        if self.random.uniform(0, 1) < NETWORK_ERROR_RATE:
            self.context.log("DROPPED - %s sends %s to %s" % (src, frame, target))
        else:
            self.context.log("%s sends %s to %s" % (src, frame, target))
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
            if timeSent >= self.context.get_time() - NETWORK_DELAY_TIME:
                continue
            if self.random.uniform(0, 1) < 0.5:
                continue  # simulated additional network delay
            received.append(frame)
        if received:
            self.frames = [(timeSent, target, frame)  # delete received frames from
                for (timeSent, target, frame) in self.frames  # self.frames
                    if frame not in received]
            self.context.log("%s received %s" % (side, \
                ", ".join(repr(frame) for frame in received)))
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
        self.receiverWaitingSeq = 0  # seq number that the receiver is waiting for
                        # - sequence number of the earliest frame it has not received
        self.nextSeq = 0  # seq number that will be assigned to the next sent message

        self.ackFramesReceived = 0  # for stats
        self.messageFramesSent = 0

    def add_messages(self, messages):
        self.messagesToSend.extend(messages)

    def _next_seq(self):
        seq = self.nextSeq
        self.nextSeq += 1
        return seq

    def run(self):
        resend = False

        # process frames (ACKs) from receiver
        frames = self.network.recv(self.side)
        for _, waitingSeq in frames:
            self.ackFramesReceived += 1
            self.context.log("Sender: got ACK with waitingSeq=%r; our " \
                "receiverWaitingSeq was %r" % (waitingSeq, self.receiverWaitingSeq))
            if waitingSeq == self.receiverWaitingSeq:
                # acked something that was acked already
                if self.window.has(self.receiverWaitingSeq):
                    self.context.log("Sender: resending")
                    resend = True
            else:
                while waitingSeq > self.receiverWaitingSeq:
                    self.context.log("Sender: removing message %r seq=%r from window" \
                        % (self.window.get(self.receiverWaitingSeq)["message"],
                           self.receiverWaitingSeq))
                    self.window.pop(self.receiverWaitingSeq)
                    self.receiverWaitingSeq += 1

        # is it too long that the not acknowledged frame was sent?
        if not resend and \
            self.window.has(self.receiverWaitingSeq) \
            and self.window.get(self.receiverWaitingSeq)["lastSent"] < \
                self.context.get_time() - ACK_WAIT_TIMEOUT:
            self.context.log("Sender: timeout expired; resending")
            resend = True

        # fill the window with messages to send
        if not resend and not self.window.full() and self.messagesToSend:
            message = self.messagesToSend.pop(0)
            seq = self._next_seq()
            frame = (seq, message)
            self.network.send(self.side, self.targetSide, frame)
            self.messageFramesSent += 1
            self.window.add(seq, {
                "lastSent": self.context.get_time(),
                "message": message,
            })
            return


        if resend and self.window.has(self.receiverWaitingSeq):
            data = self.window.get(self.receiverWaitingSeq)
            frame = (self.receiverWaitingSeq, data["message"])
            data["lastSent"] = self.context.get_time()
            self.network.send(self.side, self.targetSide, frame)
            self.messageFramesSent += 1


class Receiver:

    def __init__(self, context, network, side, senderSide):
        self.context = context
        self.network = network
        self.side = side
        self.senderSide = senderSide
        self.receivedMessages = list()

        self.window = Window()
        self.waitingForSeq = 0

        self.ackFramesSent = 0  # for stats
        self.messageFramesReceived = 0

    def get_received(self):
        return tuple(self.receivedMessages)

    def run(self):
        sendAck = False

        # process incoming message frames
        for seq, message in self.network.recv(self.side):
            self.messageFramesReceived += 1
            sendAck = True
            if seq >= self.waitingForSeq:
                # add message to window (if not already there)
                if not self.window.has(seq):
                    self.window.add(seq, message)
                    self.context.log("Receiver: message %r (seq %r) added to window" \
                        % (message, seq))
                else:
                    self.context.log("Receiver: message %r (seq %r) already in " \
                        "window (ignoring)" % (message, seq))
            else:
                self.context.log("Receiver: message %r (seq %r) already accepted "\
                    "(ignoring)" % (message, seq))

        # remove frames from window
        while self.window.has(self.waitingForSeq):
            message = self.window.pop(self.waitingForSeq)
            self.receivedMessages.append(message)
            self.context.log("Receiver: message %r (seq %r) accepted (removed " \
                "from window)" % (message, self.waitingForSeq))
            self.waitingForSeq += 1
            assert sendAck == True

        if sendAck:
            frame = ("ACK", self.waitingForSeq)
            self.network.send(self.side, self.senderSide, frame)
            self.ackFramesSent += 1


def main():
    import optparse
    op = optparse.OptionParser()
    op.add_option("-v", "--verbose", dest="verbose", default=False, action="store_true")
    (options, args) = op.parse_args()

    context = Context()
    network = Network(context=context)
    sender = Sender(context=context, network=network, side="Sender", targetSide="Receiver")
    receiver = Receiver(context=context, network=network, side="Receiver", senderSide="Sender")

    messages = ("AAA", "BBB", "CCC", "DDD", "EEE", "FFF", "GGG", "HHH")
    sender.add_messages(messages)

    while True:
        # cooperative multitasking :)
        sender.run()
        receiver.run()

        if options.verbose and context.lastTimeLogged != context.get_time():
            if receiver.get_received() != messages:
                context.log(".")

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
    print ""
    print "Stats:"
    print "  Messages transferred: %s" % len(received)
    print "  ACK frames sent: %s, received: %s, loss: %.2f %%" % \
        (receiver.ackFramesSent, sender.ackFramesReceived, \
         100 - 100.0 * sender.ackFramesReceived / receiver.ackFramesSent)
    print "  Message frames sent: %s, received: %s, loss: %.2f %%" % \
        (sender.messageFramesSent, receiver.messageFramesReceived, \
         100 - 100.0 * receiver.messageFramesReceived / sender.messageFramesSent)
    print "  Duplicate message frames sent: %s (%.2f %%), received: %s (%.2f %%)" % \
        (sender.messageFramesSent - len(messages),
         100 - 100.0 * len(messages) / sender.messageFramesSent,
         receiver.messageFramesReceived - len(messages),
         100 - 100.0 * len(messages) / receiver.messageFramesReceived)


if __name__ == "__main__":
    main()

