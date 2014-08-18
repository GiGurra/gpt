package se.gigurra.gpt.common

import java.io.IOException

import scala.reflect.ClassTag

import se.culvertsoft.mgen.javapack.classes.MGenBase

object ReadConfigFile {

  def apply[T <: MGenBase: ClassTag](fName: String): Option[T] = {
    try {
      Some(Serializer.readJsonRaw[T](ReadFile(fName)))
    } catch {
      case e: IOException => 
        None
    }
  }

}