package falcon.shmdistributor

import java.nio.ByteBuffer

import scala.Array.canBuildFrom
import scala.collection.mutable.ArrayBuffer

import shmutils.SharedMemory
import yodautils.misc.CallResult
import yodautils.nioNet.ConnStatus
import yodautils.nioNet.P2pReconnectingTcpClient
import yodautils.parser.ByteContainer
import yodautils.parser.StandardMessage
import yodautils.strings.StringUtils
import yodautils.xml.XmlBuilder

class ShmTransmitter {

    // Settings
    val cfg = new XmlBuilder(StringUtils.textFile2String("config.xml")).toMap()
    val areasString = cfg.get("areas").toString()
    val withoutBrackets = areasString.trim().substring(1, areasString.trim().length() - 1)
    val areas = withoutBrackets.split(",").map(_.trim())
    val target = cfg.get("target").toString().trim()
    val connection = new P2pReconnectingTcpClient(target, 8053)
    val shms = openShms(areas)

    def openShm(name: String) = {
        new SharedMemory(name, 0, false)
    }

    def openShms(names: Array[String]) = {
        println(s"Opening ${names.length} shared memories:")
        val shms = new ArrayBuffer[SharedMemory]()
        for (s <- names) {
            print(s"Opening '$s'...")
            var shm: SharedMemory = openShm(s)
            while (!shm.valid) {
                shm = openShm(s)
                Thread.sleep(1000)
            }
            println(s"done! (${shm.size} bytes)")
            shms += shm
        }
        shms
    }

    def transmit(x: ByteContainer) {
        CallResult.catchAll(connection.transmit(x))
    }

    def shm2Message(shm: SharedMemory): Option[StandardMessage] = {
        shm.view.map { view =>
            val nameBytes = shm.name.getBytes()
            val bb = ByteBuffer.wrap(new Array[Byte](4 + nameBytes.length + shm.size.toInt))
            bb.putInt(nameBytes.length)
            bb.put(nameBytes)
            shm.read(bb.array, bb.position(), shm.size)
            bb.rewind()
            new StandardMessage(12345, bb.array(), false)
        }
    }

    def run() {

        var lastConnStatus = connection.getStatus()

        // Main loop
        while (true) {

            val connStatus = connection.cycle(System.currentTimeMillis())
            if (connStatus != lastConnStatus) {
                println("SimDistributor status changed to: " + connStatus)
                lastConnStatus = connStatus
            }

            if (connStatus == ConnStatus.CONNECTED && !connection.isChoked()) {
                for (shm <- shms) {
                    shm2Message(shm).foreach(transmit(_))
                }
            }

            Thread.sleep(if (connStatus == ConnStatus.CONNECTED) 19 else 500)

        }

    }

}

object ShmTransmitter {
    def main(args: Array[String]) {
        new ShmTransmitter().run()
    }
}
