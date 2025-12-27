/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package libcore.java.net;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.io.Closeable;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.SocketException;
import java.nio.channels.AsynchronousCloseException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SocketChannel;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicLong;

import org.junit.Assume;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

/**
 * Test that Socket.close called on another thread interrupts a thread that's blocked doing
 * network I/O.
 */
@RunWith(JUnit4.class)
public class ConcurrentCloseTest {
    private static final InetSocketAddress UNREACHABLE_ADDRESS
            = new InetSocketAddress("192.0.2.0", 80); // RFC 5737

    @Test
    public void test_accept() throws Exception {
        ServerSocket ss = new ServerSocket(0);
        Killer<ServerSocket> killer = new Killer<>(ss).startAndWaitForStarted();
        try {
            System.err.println("accept...");
            killer.startClose();
            Socket s = ss.accept();
            fail("accept returned " + s + "!");
        } catch (SocketException expected) {
            assertEquals("Socket closed", expected.getMessage());
        }
    }

    @Test
    public void test_connect() throws Exception {
        Socket s = new Socket();
        Killer<Socket> killer = new Killer<>(s).startAndWaitForStarted();
        try {
            System.err.println("connect...");
            killer.startClose();
            s.connect(UNREACHABLE_ADDRESS);
            fail("connect returned: " + s + "!");
        } catch (SocketException expected) {
            checkIfNetworkUnavailable(expected);
            assertEquals("Socket closed", expected.getMessage());
        }
    }

    @Test
    public void test_connect_timeout() throws Exception {
        Socket s = new Socket();
        Killer<Socket> killer = new Killer<>(s).startAndWaitForStarted();
        try {
            System.err.println("connect (with timeout)...");
            killer.startClose();
            s.connect(UNREACHABLE_ADDRESS, 3600 * 1000);
            fail("connect returned: " + s + "!");
        } catch (SocketException expected) {
            checkIfNetworkUnavailable(expected);
            assertEquals("Socket closed", expected.getMessage());
        }
    }

    @Test
    public void test_connect_nonBlocking() throws Exception {
        SocketChannel s = SocketChannel.open();
        Killer<Socket> killer = new Killer<>(s.socket()).startAndWaitForStarted();
        try {
            System.err.println("connect (non-blocking)...");
            s.configureBlocking(false);
            killer.startClose();
            s.connect(UNREACHABLE_ADDRESS);
            while (!s.finishConnect()) {
                // Spin like a mad thing!
            }
            fail("connect returned: " + s + "!");
        } catch (SocketException expected) {
            checkIfNetworkUnavailable(expected);
            assertEquals("Socket closed", expected.getMessage());
        } catch (AsynchronousCloseException alsoOkay) {
            // See below.
        } catch (ClosedChannelException alsoOkay) {
            // For now, I'm assuming that we're happy as long as we get any reasonable exception.
            // It may be that we're supposed to guarantee only one or the other.
        }
    }

    @Test
    public void test_read() throws Exception {
        SilentServer ss = new SilentServer();
        Socket s = new Socket();
        s.connect(ss.getLocalSocketAddress());
        Killer<Socket> killer = new Killer<>(s).startAndWaitForStarted();
        try {
            System.err.println("read...");
            killer.startClose();
            int i = s.getInputStream().read();
            fail("read returned: " + i);
        } catch (SocketException expected) {
            assertEquals("Socket closed", expected.getMessage());
        }
        ss.close();
    }

    @Test
    public void test_read_multiple() throws Throwable {
        SilentServer ss = new SilentServer();
        final Socket s = new Socket();
        s.connect(ss.getLocalSocketAddress());

        // We want to test that we unblock *all* the threads blocked on a socket, not just one.
        // We know the implementation uses the same mechanism for all blocking calls, so we just
        // test read(2) because it's the easiest to test. (recv(2), for example, is only accessible
        // from Java via a synchronized method.)
        final ArrayList<Thread> threads = new ArrayList<Thread>();
        final List<Throwable> thrownExceptions = new CopyOnWriteArrayList<Throwable>();
        int SIZE = 10;
        CountDownLatch startedLatch = new CountDownLatch(SIZE);
        for (int i = 0; i < SIZE; ++i) {
            Thread t = new Thread(() -> {
                startedLatch.countDown();
                try {
                    try {
                        System.err.println("read...");
                        int i1 = s.getInputStream().read();
                        fail("read returned: " + i1);
                    } catch (SocketException expected) {
                        assertEquals("Socket closed", expected.getMessage());
                    }
                } catch (Throwable ex) {
                    thrownExceptions.add(ex);
                }
            });
            threads.add(t);
        }
        for (Thread t : threads) {
            t.start();
        }
        Killer<Socket> killer = new Killer<>(s).startAndWaitForStarted();
        assertTrue(startedLatch.await(2, TimeUnit.SECONDS));
        killer.startClose();

        for (Thread t : threads) {
            t.join();
        }
        for (Throwable exception : thrownExceptions) {
            throw exception;
        }

        ss.close();
    }

    @Test
    public void test_recv() throws Exception {
        DatagramSocket s = new DatagramSocket();
        byte[] buf = new byte[200];
        DatagramPacket p = new DatagramPacket(buf, 200);
        Killer<DatagramSocket> killer = new Killer<>(s).startAndWaitForStarted();
        try {
            System.err.println("receive...");
            killer.startClose();
            s.receive(p);
            fail("receive returned!");
        } catch (SocketException expected) {
            assertEquals("Socket closed", expected.getMessage());
        }
    }

    @Test
    public void test_write() throws Exception {
        final SilentServer ss = new SilentServer(128); // Minimal receive buffer size.

        // The test needs to send enough data to cause the write to block for long enough for
        // the socket close to occur. Start with a large enough factor and, if that doesn't
        // seem to be sufficient to cause the block, double it and try again. A factor of 2
        // used to be OK, then that needed to be increased to 4 and now this is also not
        // enough in some cases. (b/356850236)
        int bufferFactor = 4;

        while (true) {
            Socket s = new Socket();

            // Set the send buffer size really small, to ensure we block.
            int sendBufferSize = 1024;
            s.setSendBufferSize(sendBufferSize);
            sendBufferSize = s.getSendBufferSize(); // How big is the buffer really, Linux?

            sendBufferSize *= bufferFactor;

            s.connect(ss.getLocalSocketAddress());
            Killer<Socket> killer = new Killer<>(s);
            killer.startAndWaitForStarted();
            try {
                System.err.println("write...");
                // Write too much so the buffer is full and we block,
                // waiting for the server to read (which it never will).
                // If the asynchronous close fails, we'll see a test timeout here.
                byte[] buf = new byte[sendBufferSize];
                killer.startClose();
                s.getOutputStream().write(buf);
                if (killer.wasDefinitelyKilled()) {
                    fail("Socket close happened before write completed successfully");
                } else {
                    // Increase the bytes sent to try and cause write() to block for enough time.
                    bufferFactor *= 2;
                }
            } catch (SocketException expected) {
                // We throw "Connection reset by peer", which I don't _think_ is a problem.
                // assertEquals("Socket closed", expected.getMessage());
                break;
            }
        }
        ss.close();
    }

    // This server accepts connections, but doesn't read or write anything.
    // It holds on to the Socket connecting to the client so it won't be GCed.
    // Call "close" to close both the server socket and its client connection.
    static class SilentServer {
        private final ServerSocket ss;
        private Socket client;

        public SilentServer() throws IOException {
            this(0);
        }

        public SilentServer(int receiveBufferSize) throws IOException {
            ss = new ServerSocket(0);
            if (receiveBufferSize != 0) {
                ss.setReceiveBufferSize(receiveBufferSize);
            }
            new Thread(new Runnable() {
                public void run() {
                    try {
                        client = ss.accept();
                    } catch (Exception ex) {
                        ex.printStackTrace();
                    }
                }
            }).start();
        }

        public SocketAddress getLocalSocketAddress() {
            return ss.getLocalSocketAddress();
        }

        public void close() throws IOException {
            client.close();
            ss.close();
        }
    }

    // This thread calls the "close" method on the supplied T after 2s.
    static class Killer<T extends Closeable> extends Thread {
        private final T s;
        private final AtomicLong killedTs = new AtomicLong(0);
        private final CountDownLatch mStartedLatch = new CountDownLatch(1);
        private final CountDownLatch mCloseLatch = new CountDownLatch(1);

        public Killer(T s) {
            this.s = s;
        }

        public void run() {
            try {
                System.err.println("close...");
                mStartedLatch.countDown();
                assertTrue(mCloseLatch.await(2, TimeUnit.SECONDS));
                // Thread.sleep() avoids closing the socket too early before the corresponding
                // socket operation e.g. read(), starts.
                Thread.sleep(5);
                s.close();
                killedTs.set(System.nanoTime());
            } catch (Exception ex) {
                ex.printStackTrace();
            }
        }

        public Killer<T> startAndWaitForStarted() throws InterruptedException {
            start();
            boolean result = mStartedLatch.await(2, TimeUnit.SECONDS);
            assertTrue(result);
            return this;
        }

        public void startClose() {
            mCloseLatch.countDown();
        }

        public boolean wasDefinitelyKilled() {
            final long minThresholdNs = 500 * 1000 * 1000;
            final long now = System.nanoTime();
            final long killed = killedTs.get();
            return (killed > 0 && now - killed > minThresholdNs);
        }
    }

    private void checkIfNetworkUnavailable(SocketException ex) {
        // If the test environment does not have connectivity, the connect() will fail with
        // ENETUNREACH straight away. In this case, we should not fail the test.
        String msg = ex.getMessage();
        boolean isNetworkUnreachable = (msg != null) &&
            (msg.contains("ENETUNREACH") || msg.contains("Network is unreachable"));
        Assume.assumeFalse(
                "Connection failed due to unavailable connectivity in the test environment",
                isNetworkUnreachable);
    }
}
