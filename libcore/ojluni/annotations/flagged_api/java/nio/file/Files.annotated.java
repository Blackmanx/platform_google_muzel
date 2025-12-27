/*
 * Copyright (c) 2007, 2020, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */


package java.nio.file;

@SuppressWarnings({"unchecked", "deprecation", "all"})
public final class Files {

Files() { throw new RuntimeException("Stub!"); }

public static long copy(java.io.InputStream in, java.nio.file.Path target, java.nio.file.CopyOption... options) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static long copy(java.nio.file.Path source, java.io.OutputStream out) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.nio.file.Path copy(java.nio.file.Path source, java.nio.file.Path target, java.nio.file.CopyOption... options) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.nio.file.Path createDirectories(java.nio.file.Path dir, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.nio.file.Path createDirectory(java.nio.file.Path dir, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.nio.file.Path createFile(java.nio.file.Path path, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.nio.file.Path createLink(java.nio.file.Path link, java.nio.file.Path existing) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.nio.file.Path createSymbolicLink(java.nio.file.Path link, java.nio.file.Path target, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.nio.file.Path createTempDirectory(java.lang.String prefix, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.nio.file.Path createTempDirectory(java.nio.file.Path dir, java.lang.String prefix, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.nio.file.Path createTempFile(java.lang.String prefix, java.lang.String suffix, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.nio.file.Path createTempFile(java.nio.file.Path dir, java.lang.String prefix, java.lang.String suffix, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static void delete(java.nio.file.Path path) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static boolean deleteIfExists(java.nio.file.Path path) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static boolean exists(java.nio.file.Path path, java.nio.file.LinkOption... options) { throw new RuntimeException("Stub!"); }

public static java.util.stream.Stream<java.nio.file.Path> find(java.nio.file.Path start, int maxDepth, java.util.function.BiPredicate<java.nio.file.Path,java.nio.file.attribute.BasicFileAttributes> matcher, java.nio.file.FileVisitOption... options) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.lang.Object getAttribute(java.nio.file.Path path, java.lang.String attribute, java.nio.file.LinkOption... options) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static <V extends java.nio.file.attribute.FileAttributeView> V getFileAttributeView(java.nio.file.Path path, java.lang.Class<V> type, java.nio.file.LinkOption... options) { throw new RuntimeException("Stub!"); }

public static java.nio.file.FileStore getFileStore(java.nio.file.Path path) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.nio.file.attribute.FileTime getLastModifiedTime(java.nio.file.Path path, java.nio.file.LinkOption... options) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.nio.file.attribute.UserPrincipal getOwner(java.nio.file.Path path, java.nio.file.LinkOption... options) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.util.Set<java.nio.file.attribute.PosixFilePermission> getPosixFilePermissions(java.nio.file.Path path, java.nio.file.LinkOption... options) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static boolean isDirectory(java.nio.file.Path path, java.nio.file.LinkOption... options) { throw new RuntimeException("Stub!"); }

public static boolean isExecutable(java.nio.file.Path path) { throw new RuntimeException("Stub!"); }

public static boolean isHidden(java.nio.file.Path path) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static boolean isReadable(java.nio.file.Path path) { throw new RuntimeException("Stub!"); }

public static boolean isRegularFile(java.nio.file.Path path, java.nio.file.LinkOption... options) { throw new RuntimeException("Stub!"); }

public static boolean isSameFile(java.nio.file.Path path, java.nio.file.Path path2) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static boolean isSymbolicLink(java.nio.file.Path path) { throw new RuntimeException("Stub!"); }

public static boolean isWritable(java.nio.file.Path path) { throw new RuntimeException("Stub!"); }

public static java.util.stream.Stream<java.lang.String> lines(java.nio.file.Path path) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.util.stream.Stream<java.lang.String> lines(java.nio.file.Path path, java.nio.charset.Charset cs) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.util.stream.Stream<java.nio.file.Path> list(java.nio.file.Path dir) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.nio.file.Path move(java.nio.file.Path source, java.nio.file.Path target, java.nio.file.CopyOption... options) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.io.BufferedReader newBufferedReader(java.nio.file.Path path) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.io.BufferedReader newBufferedReader(java.nio.file.Path path, java.nio.charset.Charset cs) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.io.BufferedWriter newBufferedWriter(java.nio.file.Path path, java.nio.charset.Charset cs, java.nio.file.OpenOption... options) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.io.BufferedWriter newBufferedWriter(java.nio.file.Path path, java.nio.file.OpenOption... options) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.nio.channels.SeekableByteChannel newByteChannel(java.nio.file.Path path, java.nio.file.OpenOption... options) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.nio.channels.SeekableByteChannel newByteChannel(java.nio.file.Path path, java.util.Set<? extends java.nio.file.OpenOption> options, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.nio.file.DirectoryStream<java.nio.file.Path> newDirectoryStream(java.nio.file.Path dir) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.nio.file.DirectoryStream<java.nio.file.Path> newDirectoryStream(java.nio.file.Path dir, java.lang.String glob) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.nio.file.DirectoryStream<java.nio.file.Path> newDirectoryStream(java.nio.file.Path dir, java.nio.file.DirectoryStream.Filter<? super java.nio.file.Path> filter) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.io.InputStream newInputStream(java.nio.file.Path path, java.nio.file.OpenOption... options) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.io.OutputStream newOutputStream(java.nio.file.Path path, java.nio.file.OpenOption... options) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static boolean notExists(java.nio.file.Path path, java.nio.file.LinkOption... options) { throw new RuntimeException("Stub!"); }

public static java.lang.String probeContentType(java.nio.file.Path path) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static byte[] readAllBytes(java.nio.file.Path path) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.util.List<java.lang.String> readAllLines(java.nio.file.Path path) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.util.List<java.lang.String> readAllLines(java.nio.file.Path path, java.nio.charset.Charset cs) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static <A extends java.nio.file.attribute.BasicFileAttributes> A readAttributes(java.nio.file.Path path, java.lang.Class<A> type, java.nio.file.LinkOption... options) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.util.Map<java.lang.String,java.lang.Object> readAttributes(java.nio.file.Path path, java.lang.String attributes, java.nio.file.LinkOption... options) throws java.io.IOException { throw new RuntimeException("Stub!"); }

@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_OPENJDK_21_V2_APIS)
public static java.lang.String readString(java.nio.file.Path path) throws java.io.IOException { throw new RuntimeException("Stub!"); }

@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_OPENJDK_21_V2_APIS)
public static java.lang.String readString(java.nio.file.Path path, java.nio.charset.Charset cs) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.nio.file.Path readSymbolicLink(java.nio.file.Path link) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.nio.file.Path setAttribute(java.nio.file.Path path, java.lang.String attribute, java.lang.Object value, java.nio.file.LinkOption... options) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.nio.file.Path setLastModifiedTime(java.nio.file.Path path, java.nio.file.attribute.FileTime time) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.nio.file.Path setOwner(java.nio.file.Path path, java.nio.file.attribute.UserPrincipal owner) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.nio.file.Path setPosixFilePermissions(java.nio.file.Path path, java.util.Set<java.nio.file.attribute.PosixFilePermission> perms) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static long size(java.nio.file.Path path) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.util.stream.Stream<java.nio.file.Path> walk(java.nio.file.Path start, int maxDepth, java.nio.file.FileVisitOption... options) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.util.stream.Stream<java.nio.file.Path> walk(java.nio.file.Path start, java.nio.file.FileVisitOption... options) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.nio.file.Path walkFileTree(java.nio.file.Path start, java.nio.file.FileVisitor<? super java.nio.file.Path> visitor) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.nio.file.Path walkFileTree(java.nio.file.Path start, java.util.Set<java.nio.file.FileVisitOption> options, int maxDepth, java.nio.file.FileVisitor<? super java.nio.file.Path> visitor) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.nio.file.Path write(java.nio.file.Path path, byte[] bytes, java.nio.file.OpenOption... options) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.nio.file.Path write(java.nio.file.Path path, java.lang.Iterable<? extends java.lang.CharSequence> lines, java.nio.charset.Charset cs, java.nio.file.OpenOption... options) throws java.io.IOException { throw new RuntimeException("Stub!"); }

public static java.nio.file.Path write(java.nio.file.Path path, java.lang.Iterable<? extends java.lang.CharSequence> lines, java.nio.file.OpenOption... options) throws java.io.IOException { throw new RuntimeException("Stub!"); }

@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_OPENJDK_21_V2_APIS)
public static java.nio.file.Path writeString(java.nio.file.Path path, java.lang.CharSequence csq, java.nio.charset.Charset cs, java.nio.file.OpenOption... options) throws java.io.IOException { throw new RuntimeException("Stub!"); }

@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_OPENJDK_21_V2_APIS)
public static java.nio.file.Path writeString(java.nio.file.Path path, java.lang.CharSequence csq, java.nio.file.OpenOption... options) throws java.io.IOException { throw new RuntimeException("Stub!"); }
}

