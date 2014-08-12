package se.gigurra.gpt.common

import java.nio.file.Files
import java.nio.file.Paths

object WriteFile {
  def apply(fName: String, data: Array[Byte]) {
    Files.write(Paths.get(fName), data)
  }
}