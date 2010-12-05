#!/usr/bin/env python

TEST_PORT = "9999"

import unittest
import select
import subprocess
from StringIO import StringIO


def _split_blocks(s, length=512):
    return [s[i:i+length] for i in range(0, len(s), length)]


class Communicator:

    def __init__(self):
        self._toRead = list()
        self._toWrite = list()
        self._readData = list()

    def read(self, pipe):
        self._toRead.append(pipe)
        self._readData.append(list())
        return self

    def write(self, pipe, data):
        self._toWrite.append((pipe, _split_blocks(data)))
        return self

    def run(self):
        while True:
            rList = list()
            wList = list()

            for pipe in self._toRead:
                if pipe is not None:
                    rList.append(pipe)

            for pipe, blocks in self._toWrite:
                wList.append(pipe)

            if not rList and not wList:
                break

            (r, w, x) = select.select(rList, wList, [])

            for n, pipe in enumerate(self._toRead):
                if pipe is not None and pipe in r:
                    data = pipe.read(512)
                    if not data:
                        self._toRead[n] = None
                    else:
                        self._readData[n].append(data)

            for pipe, blocks in self._toWrite:
                if pipe in w:
                    if blocks:
                        data = blocks.pop(0)
                        pipe.write(data)
                        pipe.flush()
                    else:
                        pipe.close()
                        self._toWrite = [(p, b) for p, b in self._toWrite if p is not pipe]

        self._readData = tuple("".join(items) for items in self._readData)
        if len(self._readData) == 1:
            return self._readData[0]
        else:
            return self._readData


class CommunicatorTests (unittest.TestCase):

    def test_split_blocks(self):
        self.assertEqual(_split_blocks(""), [])
        self.assertEqual(_split_blocks("", 1), [])
        self.assertEqual(_split_blocks("a", 1), ["a"])
        self.assertEqual(_split_blocks("abcd", 1), ["a", "b", "c", "d"])
        self.assertEqual(_split_blocks("", 3), [])
        self.assertEqual(_split_blocks("a", 3), ["a"])
        self.assertEqual(_split_blocks("abcd", 3), ["abc", "d"])
        self.assertEqual(_split_blocks("abcdef", 3), ["abc", "def"])
        self.assertEqual(_split_blocks("abcdefg", 3), ["abc", "def", "g"])

    def test_cat_short(self):
        data = "foobar"
        p = subprocess.Popen(["cat"], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        output = Communicator().write(p.stdin, data).read(p.stdout).run()
        self.assertEqual(output, data)

    def test_cat_long(self):
        data = 1024 * 1024 * "foobar"
        p = subprocess.Popen(["cat"], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        output = Communicator().write(p.stdin, data).read(p.stdout).run()
        self.assertEqual(output, data)

    def test_cat_cat_short(self):
        data = "foobar"
        p1 = subprocess.Popen(["cat"], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        p2 = subprocess.Popen(["cat"], stdin=p1.stdout, stdout=subprocess.PIPE, close_fds=True)
        output = Communicator().write(p1.stdin, data).read(p2.stdout).run()
        self.assertEqual(output, data)

    def test_cat_cat_long(self):
        data = 1024 * 1024 * "foobar"
        p1 = subprocess.Popen(["cat"], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        p2 = subprocess.Popen(["cat"], stdin=p1.stdout, stdout=subprocess.PIPE, close_fds=True)
        output = Communicator().write(p1.stdin, data).read(p2.stdout).run()
        self.assertEqual(output, data)


class TransferTests (unittest.TestCase):

    def test_one(self):
        data = "Hello"
        server = subprocess.Popen(["./server", TEST_PORT], stdout=subprocess.PIPE)
        client = subprocess.Popen(["./client", "127.0.0.1", TEST_PORT], stdin=subprocess.PIPE)
        output = Communicator().write(client.stdin, data).read(server.stdout).run()
        self.assertEqual(output, data)


if __name__ == "__main__":
    unittest.main(testRunner=unittest.TextTestRunner(verbosity=2))


