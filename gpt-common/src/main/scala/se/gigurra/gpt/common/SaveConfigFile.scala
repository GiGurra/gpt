package se.gigurra.gpt.common

import se.culvertsoft.mgen.javapack.classes.MGenBase

object SaveConfigFile {

  def apply[T <: MGenBase](fName: String, t: T) {
    WriteFile(fName, Serializer.writeJsonRaw(t))
  }

}