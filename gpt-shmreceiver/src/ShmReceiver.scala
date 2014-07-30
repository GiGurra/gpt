package falcon.shmsubscriber

import java.io.Serializable
import java.nio.ByteBuffer
import java.util.ArrayList

import scala.collection.JavaConversions.asScalaBuffer
import scala.collection.mutable.HashMap

import shmutils.SharedMemory
import yodautils.concurrent.Callback
import yodautils.nioNet.TcpServer
import yodautils.nioNet.rulesets.P2pServerRuleset
import yodautils.parser.CRC32Message
import yodautils.parser.ParsedMessage
import yodautils.parser.StandardMessage

class ShmReceiver {

    private val sharedMems = new HashMap[String, SharedMemory]

    private val shmMsgCallback = new Callback() {

        override def receiveCallback(src: Object, data: Serializable) {

            for (
                msg <- data.asInstanceOf[ArrayList[ParsedMessage]]
                    .map(_.asStandardMsg)
                    .filter(_.getType() == 12345)
            ) {

                val smAllBytes = msg.getBytes()
                val allBytes = ByteBuffer.wrap(smAllBytes)
                allBytes.position(StandardMessage.HEADER_SIZE)

                val shmNameBytes = new Array[Byte](allBytes.getInt())
                allBytes.get(shmNameBytes)

                val shmName = new String(shmNameBytes)
                val dataOffs = allBytes.position()
                val shmSize = allBytes.remaining() - CRC32Message.CRC32_CHECKSUM_SIZE

                val shm = sharedMems.getOrElseUpdate(shmName, new SharedMemory(shmName, shmSize, true))
                if (shm.valid) {
                    shm.write(smAllBytes, dataOffs, shmSize)
                    shm.flush()
                }

            }

        }

    }

    private val server = new TcpServer(8053, new P2pServerRuleset(shmMsgCallback))

    def run() {
        server.run()
    }

}

object ShmReceiver {
    def main(args: Array[String]) {
        new ShmReceiver().run()
    }
}
