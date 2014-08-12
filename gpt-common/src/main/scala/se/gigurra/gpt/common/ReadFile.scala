package se.gigurra.gpt.common

import java.nio.file.Files
import java.nio.file.Paths

object ReadFile {
  def apply(fName: String): Array[Byte] = {
    Files.readAllBytes(Paths.get(fName))
  }
}