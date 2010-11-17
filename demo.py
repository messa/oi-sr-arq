#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Selective Repeat ARQ simulation.
"""

WINDOW_SIZE = 5



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
        self.context = context
        self.frames = list()
        self.travelTime = 3
        
    def send(self, src, target, frame):
        """
        Send a frame to target.
        """
        self.context.log("NETWORK: %s sends %s to %s" % (src, frame, target))
        self.frames.append((self.context.get_time(), target, frame))

    def recv(self, side):
        """
        Are there any frames for this side?
        Returns list of frames.
        """
        received = list()
        for data in self.frames:
            timeSent, target, frame = data
            if target != side:
                continue
            if timeSent >= self.context.get_time() - self.travelTime:
                continue
            received.append(frame)
            self.frames.remove(data)
        if received:
            self.context.log("NETWORK: %s received %s" % (side, received))
        return received


class Window:
    
    def __init__(self):
        self.size = WINDOW_SIZE
        self.data = list()
        
    def full(self):
        return len(self.data) >= self.size
        
    def add(self, item):
        if self.full():
            raise Exception("Window is full")
        self.data.append(item)


class Sender:
    
    def __init__(self, context, network, side, targetSide):
        self.context = context
        self.network = network
        self.side = side
        self.targetSide = targetSide
        self.messagesToSend = list()
        self.window = Window()
        self.nextSeq = 0
        
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
            self.window.add({
                "firstSent": self.context.get_time(),
                "lastSent": self.context.get_time(),
                "frame": frame,
            })
            return
            
        
            


class Receiver:
    
    def __init__(self, context, network, side):
        self.context = context
        self.network = network
        self.side = side
        self.receivedMessages = list()
        self.window = Window()
        
    def get_received(self):
        return tuple(self.receivedMessages)
        
    def run(self):
        frames = self.network.recv(self.side)
        for frame in frames:
            pass


def main():
    context = Context()
    network = Network(context=context)
    sender = Sender(context=context, network=network, side="A", targetSide="B")
    receiver = Receiver(context=context, network=network, side="B")

    messages = ("aaa", "bbb", "ccc", "ddd", "eee", "fff", "ggg", "hhh")
    sender.add_messages(messages)
    
    while context.get_time() < 1000:
        sender.run()
        receiver.run()
        context.tick()
    
    received = receiver.get_received()
    if received == messages:
        print "Everything OK"
    else:
        raise Exception("Messages received: %r, but should be received: %r" % (received, messages))
    


if __name__ == "__main__":
    main()

