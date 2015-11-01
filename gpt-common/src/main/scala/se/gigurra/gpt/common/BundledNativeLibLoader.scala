package se.gigurra.gpt.common

import java.io.File
import java.nio.file.{Files, Paths}

import com.google.common.io.ByteStreams
import com.sun.jna.Platform

object BundledNativeLibLoader {

  def os = Platform.getOSType match {
    case Platform.WINDOWS => "windows"
    case _ => "unknown_os"
  }

  def ext = Platform.getOSType match {
    case Platform.WINDOWS => "dll"
    case _ => "unknown_os_lib_ext"
  }

  def bits = if (Platform.is64Bit) "64" else "32"

  def load(lib: String): Unit = {

    val fileName = s"${lib}_${os}_${bits}.${ext}"
    val outPath = Paths.get(new File(tempDir, fileName).getAbsoluteFile.getPath)

    for (is <- resource.managed(getClass.getClassLoader.getResourceAsStream(fileName))) {
      val libBytes = ByteStreams.toByteArray(is)
      Files.write(outPath, libBytes)
    }

    System.load(outPath.toString)

  }

  def tempDir = new File(System.getProperty("java.io.tmpdir"))

}
