package se.gigurra.gpt.common

import java.io.ByteArrayInputStream
import java.io.ByteArrayOutputStream

import scala.language.reflectiveCalls
import scala.reflect.ClassTag

import se.culvertsoft.mgen.javapack.classes.MGenBase
import se.culvertsoft.mgen.javapack.serialization.BinaryReader
import se.culvertsoft.mgen.javapack.serialization.BinaryWriter
import se.culvertsoft.mgen.javapack.serialization.BuiltInReader
import se.culvertsoft.mgen.javapack.serialization.BuiltInWriter
import se.culvertsoft.mgen.javapack.serialization.JsonReader
import se.culvertsoft.mgen.javapack.serialization.JsonWriter
import se.culvertsoft.mnet.DataMessage
import se.culvertsoft.mnet.Message

object Serializer {

  private val buffer = new ByteArrayOutputStream
  private val jsonWriter = new JsonWriter(buffer, Common.reg)
  private val binaryWriter = new BinaryWriter(buffer, Common.reg)
  private val jsonReader = new JsonReader(new ByteArrayInputStream(Array[Byte]()), Common.reg)
  private val binaryReader = new BinaryReader(new ByteArrayInputStream(Array[Byte]()), Common.reg)

  private def write(o: MGenBase, writer: BuiltInWriter): Array[Byte] = {
    writer.writeObject(o)
    val out = buffer.toByteArray()
    buffer.reset()
    out
  }

  private def read[T <: MGenBase: ClassTag](data: Array[Byte], reader: BuiltInReader { def setInput(s: java.io.InputStream): BuiltInReader }): T = {
    reader.setInput(new ByteArrayInputStream(data))
    reader.readObject(scala.reflect.classTag[T].runtimeClass.asInstanceOf[Class[T]])
  }

  def writeJsonRaw(o: MGenBase): Array[Byte] = synchronized { write(o, jsonWriter) }
  def writeBinaryRaw(o: MGenBase): Array[Byte] = synchronized { write(o, binaryWriter) }

  def readJsonRaw[T <: MGenBase: ClassTag](data: Array[Byte]): T = synchronized { read[T](data, jsonReader) }
  def readBinaryRaw[T <: MGenBase: ClassTag](data: Array[Byte]): T = synchronized { read[T](data, binaryReader) }

  def writeJson(o: MGenBase): Message = synchronized {
    new DataMessage().setStringData(jsonWriter.writeObjectToString(o))
  }

  def writeBinary(o: MGenBase): Message = synchronized {
    new DataMessage().setBinaryData(write(o, binaryWriter))
  }

  def read[T <: MGenBase: ClassTag](container: Message): Option[T] = synchronized {
    container match {
      case container: DataMessage =>
        if (container.hasBinaryData) {
          binaryReader.setInput(new ByteArrayInputStream(container.getBinaryData)).readObject() match {
            case x: T => Some(x)
            case _ => None
          }
        } else if (container.hasStringData) {
          jsonReader.readObject(container.getStringData) match {
            case x: T => Some(x)
            case _ => None
          }
        } else {
          None
        }
      case _ => None
    }
  }

}