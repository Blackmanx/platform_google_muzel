/*
 * Copyright (C) 2024 The Android Open Source Project
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

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

public class Main {
  static volatile boolean name_was_set = false;
  static final String BIRTH_NAME = "birth name";
  static final String NEW_NAME = "new name";
  static final CountDownLatch child_started = new CountDownLatch(1);

  private static class Child implements Runnable {
    @Override
    public void run() {
      String bname = Thread.currentThread().getName();
      if (!name_was_set && !bname.equals(BIRTH_NAME)) {
        System.err.println("Wrong birth name: " + bname);
      }
      child_started.countDown();
      while (!name_was_set) {
        try {
          Thread.sleep(10);
        } catch (InterruptedException e) {
          System.out.println("Unexpected interrupt in child");
        }
      }
      System.out.println("Name was set");
      System.out.println("Final child Java name: " + Thread.currentThread().getName());
      System.out.print("Final child pthread name: ");
      printPthreadName();
    }
  }

  public static void main(String[] args) {
    System.loadLibrary(args[0]);
    System.out.print("Main Started; java name: ");
    System.out.println(Thread.currentThread().getName());
    System.out.print("Pthread name: ");
    printPthreadName();
    Thread t = new Thread(new Child(), BIRTH_NAME);
    System.out.print("Child's Java name: ");
    System.out.println(t.getName());
    t.start();
    try {
      if (!child_started.await(2, TimeUnit.SECONDS)) {
        System.out.println("Latch wait timed out");
      }
    } catch (InterruptedException e) {
      System.out.println("Unexpected interrupt in parent");
    }
    System.out.println("Setting name from " + Thread.currentThread().getName());
    t.setName(NEW_NAME);
    if (!t.getName().equals(NEW_NAME)) {
      System.err.println("Wrong new name from main thread: " + t.getName());
    }
    name_was_set = true;
    try {
      t.join();
    } catch (InterruptedException e) {
      System.out.println("Unexpected interrupt in join()");
    }
    System.out.println("Final parent Java name: " + Thread.currentThread().getName());
    System.out.print("Final parent pthread name: ");
    printPthreadName();
  }

  private static native void printPthreadName();
}
