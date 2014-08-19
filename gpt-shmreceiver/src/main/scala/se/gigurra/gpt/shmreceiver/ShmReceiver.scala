package se.gigurra.gpt.shmreceiver

import scala.collection.mutable.HashMap

import se.culvertsoft.mnet.Message
import se.culvertsoft.mnet.NodeSettings
import se.culvertsoft.mnet.api.Connection
import se.culvertsoft.mnet.api.Route
import se.culvertsoft.mnet.backend.WebsockBackendSettings
import se.culvertsoft.mnet.client.MNetClient
import se.gigurra.gpt.common.NetworkNames
import se.gigurra.gpt.common.Serializer
import se.gigurra.gpt.common.SharedMemory
import se.gigurra.gpt.model.shm.common.ShmMsg

object ShmReceiver {

  val listenPort = 8053
  val sharedMems = new HashMap[String, SharedMemory]

  def main(args: Array[String]) {

    new MNetClient(
      new WebsockBackendSettings().setListenPort(listenPort),
      new NodeSettings().setName(NetworkNames.SHM_RECEIVER)) {

      override def handleMessage(msg: Message, connection: Connection, route: Route) {

        if (route != null && route.name == NetworkNames.SHM_TRANSMITTER) {

          Serializer.read[ShmMsg](msg) match {
            case Some(msg) =>
              val shm = sharedMems.getOrElseUpdate(msg.getName, new SharedMemory(msg.getName, msg.getSize, true))
              if (shm.valid) {
                shm.write(msg.getData, 0, msg.getData.length)
                shm.flush()
              }
            case _ =>
          }
        }
      }

      start()
    }

  }
}
