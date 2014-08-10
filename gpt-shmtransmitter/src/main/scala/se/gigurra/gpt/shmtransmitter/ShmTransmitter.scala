package falcon.shmdistributor

import java.io.ByteArrayOutputStream
import java.io.FileInputStream
import java.nio.ByteBuffer

import scala.collection.JavaConversions.asScalaBuffer
import scala.collection.mutable.ArrayBuffer
import scala.collection.mutable.HashMap

import se.culvertsoft.mgen.javapack.serialization.BinaryWriter
import se.culvertsoft.mgen.javapack.serialization.JsonReader
import se.gigurra.gpt.common.SharedMemory
import se.gigurra.gpt.model.ClassRegistry
import se.gigurra.gpt.model.shm.common.ShmMsg
import se.gigurra.gpt.model.shm.transmitter.ShmTransmitterCfg
import se.gigurra.libgurra.net.types.ManagedByteClient
import se.gigurra.libgurra.parsing.types.BasicMessage

object ShmTransmitter {

  val classRegisty = new ClassRegistry
  val readBuffers = new HashMap[SharedMemory, Array[Byte]]

  def openShms(names: Seq[String]) = {

    def tryOpenShm(name: String) = {
      new SharedMemory(name, 0, false)
    }

    println(s"Opening ${names.length} shared memories:")
    val shms = new ArrayBuffer[SharedMemory]()
    for (s <- names) {
      print(s"Opening '$s'...")
      var shm = tryOpenShm(s)
      while (!shm.valid) {
        shm = tryOpenShm(s)
        Thread.sleep(1000)
      }
      println(s"done! (${shm.size} bytes)")
      shms += shm
      readBuffers.put(shm, new Array[Byte](shm.size))
    }
    shms
  }

  def main(args: Array[String]) {

    val reader = new JsonReader(new FileInputStream("config.json"), classRegisty)
    val settings = reader.readObject(classOf[ShmTransmitterCfg])
    val buffer = new ByteArrayOutputStream
    val serializer = new BinaryWriter(buffer, classRegisty)

    val clients = settings.getTargets.map { target =>
      new ManagedByteClient(target.getIp, target.getPort /* 8052 */ ) {
        def send(msg: ShmMsg) {
          synchronized {
            serializer.writeObject(msg)
            val out = buffer.toByteArray()
            buffer.reset()
            if (!isChoked)
              send(ByteBuffer.wrap(BasicMessage.fromData(out).allBytes))
          }
        }
        start()
      }
    }

    val shms = openShms(settings.getShms)

    while (true) {

      for (client <- clients) {

        for (shm <- shms) {

          try {
            if (client.isConnected) {
              val readBuf = readBuffers(shm)
              val msg = new ShmMsg
              msg.setData(readBuf)
              msg.setName(shm.name)
              msg.setSize(shm.size)
              shm.read(readBuf, readBuf.length)
              client.send(msg)
            }
          } catch {
            case e: Exception => e.printStackTrace()
          }
        }
      }

      Thread.sleep(15)

    }

  }

}
