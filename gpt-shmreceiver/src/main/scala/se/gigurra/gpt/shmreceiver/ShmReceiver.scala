package se.gigurra.gpt.shmreceiver

import java.io.ByteArrayInputStream

import scala.collection.mutable.HashMap

import se.culvertsoft.mgen.javapack.serialization.BinaryReader
import se.gigurra.gpt.common.SharedMemory
import se.gigurra.gpt.model.ClassRegistry
import se.gigurra.gpt.model.shm.common.ShmMsg
import se.gigurra.libgurra.net.types.BasicTcpServer
import se.gigurra.libgurra.parsing.types.BasicMessage

object ShmReceiver {

  val listenPort = 8053
  val timeOutSeconds = 5.0
  val maxSz = 5 * 1024 * 1024

  val classRegistry = new ClassRegistry
  val sharedMems = new HashMap[String, SharedMemory]

  def main(args: Array[String]) {
    new BasicTcpServer(listenPort, timeOutSeconds, maxSz, maxSz) {
      override def handleMessage(c: Client, _msg: BasicMessage) {
        val reader = new BinaryReader(new ByteArrayInputStream(_msg.data), classRegistry)
        val msg = reader.readObject(classOf[ShmMsg])

        val shm = sharedMems.getOrElseUpdate(msg.getName, new SharedMemory(msg.getName, msg.getSize, true))
        if (shm.valid) {
          shm.write(msg.getData, 0, msg.getData.length)
          shm.flush()
        }
      }
    }.start()
  }
}
