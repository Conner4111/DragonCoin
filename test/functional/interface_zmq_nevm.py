#!/usr/bin/env python3
# Copyright (c) 2015-2020 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test the ZMQ notification interface."""
import struct

from test_framework.address import ADDRESS_BCRT1_UNSPENDABLE, ADDRESS_BCRT1_P2WSH_OP_TRUE
from test_framework.blocktools import create_block, create_coinbase, add_witness_commitment
from test_framework.test_framework import SyscoinTestFramework
from test_framework.messages import CTransaction, hash256, FromHex, CNEVMBlock, CNEVMBlockConnect, uint256_from_str
from test_framework.util import (
    assert_equal,
    assert_raises_rpc_error,
    hex_str_to_bytes,
)
from io import BytesIO
from time import sleep
from threading import Thread
import random
# these would be handlers for the 3 types of calls from Syscoin on Geth
def receive_thread_nevmblock(self, subscriber, publisher):
    while True:
        try:
            self.log.info('receive_thread_nevmblock waiting to receive...')
            data = subscriber.receive()
            self.log.info('receive_thread_nevmblock received data')
            hashStr = hash256(str(random.randint(-0x80000000, 0x7fffffff)).encode())
            hashTopic = uint256_from_str(hashStr)
            nevmBlock = CNEVMBlock(hashTopic, hashStr, hashStr, subscriber.topic)
            publisher.send([subscriber.topic, nevmBlock.serialize()])
        except zmq.ContextTerminated:
            subscriber.close()
            publisher.close()
            break
        except zmq.ZMQError as e:
            self.log.warning('zmq error, socket closed unexpectedly.')
            raise e 

def receive_thread_nevmblockconnect(self, subscriber, publisher):
    while True:
        try:
            self.log.info('receive_thread_nevmblockconnect waiting to receive...')
            data = subscriber.receive()
            self.log.info('receive_thread_nevmblockconnect received data')
            evmBlockConnect = CNEVMBlockConnect()
            evmBlockConnect.deserialize(BytesIO(data))
            publisher.send([subscriber.topic, b"connected"])
        except zmq.ContextTerminated:
            subscriber.close()
            publisher.close()
            break
        except zmq.ZMQError as e:
            self.log.warning('zmq error, socket closed unexpectedly.')
            raise e 

def receive_thread_nevmblockdisconnect(self, subscriber, publisher):
    while True:
        try:
            self.log.info('receive_thread_nevmblockdisconnect waiting to receive...')
            data = subscriber.receive()
            self.log.info('receive_thread_nevmblockdisconnect received data')
            publisher.send([subscriber.topic, b"disconnected"])
        except zmq.ContextTerminated:
            subscriber.close()
            publisher.close()
            break
        except zmq.ZMQError as e:
            self.log.warning('zmq error, socket closed unexpectedly.')
            raise e 

# Test may be skipped and not have zmq installed
try:
    import zmq
except ImportError:
    pass
# this simulates the Geth node subscriber, subscribing to Syscoin publisher
class ZMQSubscriber:
    def __init__(self, socket, topic):
        self.socket = socket
        self.topic = topic

        self.socket.setsockopt(zmq.SUBSCRIBE, self.topic)

    # Receive message from publisher and verify that topic and sequence match
    def _receive_from_publisher_and_check(self):
        topic, body, seq = self.socket.recv_multipart()
        # Topic should match the subscriber topic.
        assert_equal(topic, self.topic)
        return body

    def receive(self):
        return self._receive_from_publisher_and_check()

    def close(self):
        self.socket.close()

# this simulates the Geth node publisher, publishing back to Syscoin subscriber
class ZMQPublisher:
    def __init__(self, socket):
        self.socket = socket

    # Send message to subscriber
    def _send_to_publisher_and_check(self, msg_parts):
        self.socket.send_multipart(msg_parts)

    def send(self, msg_parts):
        return self._send_to_publisher_and_check(msg_parts)

    def close(self):
        self.socket.close()

class ZMQTest (SyscoinTestFramework):
    def set_test_params(self):
        self.num_nodes = 2
        if self.is_wallet_compiled():
            self.requires_wallet = True
        # This test isn't testing txn relay/timing, so set whitelist on the
        # peers for instant txn relay. This speeds up the test run time 2-3x.
        self.extra_args = [["-whitelist=noban@127.0.0.1"]] * self.num_nodes

    def skip_test_if_missing_module(self):
        self.skip_if_no_py3_zmq()
        self.skip_if_no_syscoind_zmq()

    def run_test(self):
        self.ctx = zmq.Context()
        self.ctxpub = zmq.Context()
        try:
            self.test_basic()
        finally:
            # Destroy the ZMQ context.
            self.log.debug("Destroying ZMQ context")
            self.ctx.destroy(linger=None)
            self.ctxpub.destroy(linger=None)

    # Restart node with the specified zmq notifications enabled, subscribe to
    # all of them and return the corresponding ZMQSubscriber objects.
    def setup_zmq_test(self, services, servicessub, *, recv_timeout=60, sync_blocks=True):
        subscribers = []
        for topic, address in services:
            socket = self.ctx.socket(zmq.SUB)
            subscribers.append(ZMQSubscriber(socket, topic.encode()))
        self.extra_args[0] += ["-zmqpub%s=%s" % (topic, address) for topic, address in services]
        # publisher on Syscoin can have option to also be a subscriber on another address, related each publisher to a subscriber (in our case we have 3 publisher events that also would subscribe to events on the same topic)
        self.extra_args[0] += ["-zmqsubpub%s=%s" % (topic, address) for topic, address in servicessub]
        self.restart_node(0)

        for i, sub in enumerate(subscribers):
            sub.socket.connect(services[i][1])

        # set subscriber's desired timeout for the test
        for sub in subscribers:
            sub.socket.set(zmq.RCVTIMEO, recv_timeout*1000)

        return subscribers

    # setup the publisher socket to publish events back to Syscoin from Geth simulated publisher
    def setup_zmq_test_pub(self, address):
        socket = self.ctxpub.socket(zmq.PUB)
        publisher = ZMQPublisher(socket)
        publisher.socket.bind(address)
        return publisher

    def test_basic(self):
        self.log.info("restarting all nodes except node 0 in nevm mode...")
        for i in range(len(self.nodes)):
            if i > 0:
                self.restart_node(i, ["-enforcenevm", "-zmqpubrawtx=foo", "-zmqpubhashtx=bar"])
        addresspub = 'tcp://127.0.0.1:29434'
        address = 'tcp://127.0.0.1:29433'
        self.log.info("setup publisher...")
        publisher = self.setup_zmq_test_pub(addresspub)
        self.log.info("setup subscribers...")
        subs = self.setup_zmq_test([(topic, address) for topic in ["nevmconnect", "nevmdisconnect", "nevmblock"]], [(topic, addresspub) for topic in ["nevmconnect", "nevmdisconnect", "nevmblock"]])
        self.log.info("start node 0 in nevm mode...")
        # nevm enforcement starts at block 205 (we should be at 200 here), and also it needs this flag passed in on regtest mode to trigger enforcement of nevm
        self.extra_args[0] += ["-enforcenevm"]
        self.restart_node(0)
        self.connect_nodes(0, 1)
        self.sync_blocks()

        nevmblockconnect = subs[0]
        nevmblockdisconnect = subs[1]
        nevmblock = subs[2]

        num_blocks = 200
        self.log.info("Generate %(n)d blocks (and %(n)d coinbase txes)" % {"n": num_blocks})
        # start the threads to handle pub/sub of SYS/GETH communications
        Thread(target=receive_thread_nevmblock, args=(self, nevmblock,publisher,)).start()
        Thread(target=receive_thread_nevmblockconnect, args=(self, nevmblockconnect,publisher,)).start()
        Thread(target=receive_thread_nevmblockdisconnect, args=(self, nevmblockdisconnect,publisher,)).start()
        genhashes = self.nodes[0].generatetoaddress(num_blocks, ADDRESS_BCRT1_UNSPENDABLE)

        self.sync_all()

if __name__ == '__main__':
    ZMQTest().main()
