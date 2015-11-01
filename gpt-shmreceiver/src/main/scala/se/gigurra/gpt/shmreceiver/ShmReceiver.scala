package se.gigurra.gpt.shmreceiver

import se.culvertsoft.mnet.{Message, NodeSettings}
import se.culvertsoft.mnet.api.{Connection, Route}
import se.culvertsoft.mnet.backend.WebsockBackendSettings
import se.culvertsoft.mnet.client.MNetClient
import se.gigurra.gpt.common.{NetworkNames, Serializer, SharedMemory}
import se.gigurra.gpt.model.shm.common.ShmMsg

import scala.collection.mutable.HashMap

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
