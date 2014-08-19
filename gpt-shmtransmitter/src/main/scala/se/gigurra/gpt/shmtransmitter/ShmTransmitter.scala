package falcon.shmdistributor

import scala.collection.JavaConversions.asScalaBuffer
import scala.collection.JavaConversions.bufferAsJavaList
import scala.collection.mutable.ArrayBuffer
import scala.collection.mutable.HashMap

import se.culvertsoft.mnet.NodeSettings
import se.culvertsoft.mnet.backend.WebsockBackendSettings
import se.culvertsoft.mnet.client.MNetClient
import se.gigurra.gpt.common.NetworkNames
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

    val cfg = ReadConfigFile[ShmTransmitterCfg]("config.json").getOrElse(new ShmTransmitterCfg)

    val nodeSettings = new NodeSettings().setName(NetworkNames.SHM_TRANSMITTER)
    val wsSettings = new WebsockBackendSettings().unsetListenPort()
    wsSettings.getConnectTo().addAll(cfg.getTargets.map(se.gigurra.gpt.common.NetworkAddr2Url.apply))

    val client = new MNetClient(wsSettings, nodeSettings).start()
    val shms = openShms(cfg.getShms)

    while (true) {

      // Read shms
      for (shm <- shms) {
        val readBuf = readBuffers(shm)
        shm.read(readBuf, readBuf.length)
      }

      // Send shms
      for (route <- client.getRoutes) {
        if (route.isConnected && !route.hasBufferedData && route.name == NetworkNames.SHM_RECEIVER) {
          for (shm <- shms) {
            route.send(Serializer.writeBinary(new ShmMsg()
              .setData(readBuffers(shm))
              .setName(shm.name)
              .setSize(shm.size))
              .setSenderId(client.id))
          }
        }
      }

      Thread.sleep(15)

    }

  }

}
