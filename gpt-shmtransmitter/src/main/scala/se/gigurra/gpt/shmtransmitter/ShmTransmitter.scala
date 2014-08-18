package falcon.shmdistributor

import scala.collection.JavaConversions.asScalaBuffer
import scala.collection.mutable.ArrayBuffer
import scala.collection.mutable.HashMap

import se.culvertsoft.mnet.client.MNetClient
import se.gigurra.gpt.common.ReadConfigFile
import se.gigurra.gpt.common.Serializer
import se.gigurra.gpt.common.SharedMemory
import se.gigurra.gpt.model.shm.common.ShmMsg
import se.gigurra.gpt.model.shm.transmitter.ShmTransmitterCfg

object ShmTransmitter {

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

    val settings = ReadConfigFile[ShmTransmitterCfg]("config.json").getOrElse(new ShmTransmitterCfg)

    val clients = settings.getTargets.map { t => new MNetClient(t.getIp, t.getPort) }
    clients.foreach(_.start())

    val shms = openShms(settings.getShms)

    while (true) {

      for (
        client <- clients;
        route <- client.getRoutes.filterNot(_.connection.hasBufferedData);
        shm <- shms
      ) {
        val readBuf = readBuffers(shm)
        val msg = new ShmMsg
        msg.setData(readBuf)
        msg.setName(shm.name)
        msg.setSize(shm.size)
        shm.read(readBuf, readBuf.length)
        client.broadcast(Serializer.writeJson(msg))
      }

      Thread.sleep(15)

    }

  }

}
