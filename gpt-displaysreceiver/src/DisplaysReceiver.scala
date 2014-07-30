package ymfde

import java.awt.Rectangle
import java.awt.image.BufferedImage
import java.awt.image.DataBufferInt
import java.nio.ByteBuffer

import scala.collection.JavaConversions.asScalaSet
import scala.collection.JavaConversions.bufferAsJavaList
import scala.collection.mutable.ArrayBuffer

import org.libjpegturbo.turbojpeg.TJDecompressor

import shmutils.SharedMemory
import yodautils.nioNet.TcpServer
import yodautils.nioNet.rulesets.P2pServerRuleset
import yodautils.parser.ParsedMessage
import yodautils.strings.StringUtils
import yodautils.xml.XmlBuilder

object DisplaysReceiver {

    val tjDec = new TJDecompressor
    val DEFAULT_SETTINGS_FILE_NAME = "config.xml"
    val DISPLAYS_NAME = "Displays"
    val ACTIVE_STRING = "active"
    val X_TEX_STRING = "x_tex"
    val Y_TEX_STRING = "y_tex"
    val W_TEX_STRING = "w_tex"
    val H_TEX_STRING = "h_tex"
    val X_SCR_STRING = "x_scr"
    val Y_SCR_STRING = "y_scr"
    val W_SCR_STRING = "w_scr"
    val H_SCR_STRING = "h_scr"
    val windows = new ArrayBuffer[RenderWindow]
    val swapChain = new TripleBuffer[BufferedImage](new Array[BufferedImage](3), false)
    var imgWidth = 0
    var imgHeight = 0
    var lastTcpMsgAt = -10.0

    val jpgMsgCombiner = new JpegMsgCombiner(new JpgCallback {
        override def handleFinishedJpgMsg(bb: ByteBuffer) {
            lastTcpMsgAt = System.nanoTime() / 1e9
            tjDec.setJPEGImage(bb.array(), bb.position())
            ensureBiSize(tjDec.getWidth(), tjDec.getHeight())
            swapChain.paint(tjDec.decompress(_, 0))
            issueRedrawWindows()
        }
    })

    def ensureBiSize(w: Int, h: Int) {
        if (imgWidth != w || imgHeight != h) {
            imgWidth = w
            imgHeight = h
            swapChain.resetBuffers(
                new BufferedImage(w, h, BufferedImage.TYPE_INT_RGB),
                new BufferedImage(w, h, BufferedImage.TYPE_INT_RGB),
                new BufferedImage(w, h, BufferedImage.TYPE_INT_RGB))
        }
    }

    def issueRedrawWindows() {
        windows.foreach(_.issueUpdate())
    }

    def allVisible(windows: ArrayBuffer[RenderWindow]): Boolean = {
        windows.forall(_.isAlive)
    }

    def main() {

        print("Reading settings from " + DEFAULT_SETTINGS_FILE_NAME + "...")
        val settingsString = StringUtils.textFile2String(DEFAULT_SETTINGS_FILE_NAME)
        val xml = new XmlBuilder(settingsString.getBytes())
        val settings = xml.toMap()
        val shmName = settings.get("SHM").asInstanceOf[String]
        val shmTexW = settings.get("SHM_tex_w").asInstanceOf[String].toInt
        val shmTexH = settings.get("SHM_tex_h").asInstanceOf[String].toInt
        println("ok")

        print("Starting displays...")
        val displays = settings.get(DISPLAYS_NAME).asInstanceOf[java.util.Map[String, java.util.Map[String, String]]]
        for (displayName <- displays.keySet()) {
            val curDisplay = displays.get(displayName)
            if (curDisplay.get(ACTIVE_STRING).toBoolean) {
                val xScr = curDisplay.get(X_SCR_STRING).toInt
                val yScr = curDisplay.get(Y_SCR_STRING).toInt
                val wScr = curDisplay.get(W_SCR_STRING).toInt
                val hScr = curDisplay.get(H_SCR_STRING).toInt
                val xTex = curDisplay.get(X_TEX_STRING).toDouble
                val yTex = curDisplay.get(Y_TEX_STRING).toDouble
                val wTex = curDisplay.get(W_TEX_STRING).toDouble
                val hTex = curDisplay.get(H_TEX_STRING).toDouble
                val border = curDisplay.get("border").toBoolean
                val aot = curDisplay.get("alwaysOnTop").toBoolean
                windows.add(new RenderWindow( //
                    displayName, //
                    new Rectangle(xScr, yScr, wScr, hScr), //
                    new DoubleRectangle(xTex, yTex, wTex, hTex), //
                    border, //
                    aot, //
                    swapChain) //
                    )
            }
        }
        println("ok")

        // Start listening on TCP
        val tcpListener = new TcpServer(8051, new P2pServerRuleset(new P2pServerRuleset.MsgCallback {
            override def handle(pm: ParsedMessage) {
                JpgMsg(pm.getBytes(), false).foreach(jpgMsgCombiner.handleNewSubImage(_))
            }
        })).startOnNewThread

        // Also look in SHM
        var sm = new SharedMemory(shmName, 0, false)
        while (allVisible(windows)) {

            if (System.nanoTime() / 1e9 - lastTcpMsgAt < 2.0) {
                Thread.sleep(100)
            } else {

                if (sm.valid) {

                    ensureBiSize(shmTexW, shmTexH)

                    swapChain.paint { backBuffer =>
                        val trgData = backBuffer.getRaster().getDataBuffer().asInstanceOf[DataBufferInt].getData()
                        sm.flush()
                        sm.read(trgData, shmTexW * shmTexH)
                    }

                    issueRedrawWindows()
                    Thread.sleep(19)

                } else {
                    sm = new SharedMemory(shmName, 0, false)
                    Thread.sleep(100)
                }
            }

        }

        // Kill recv server
        tcpListener.killJoin()

        // Save xml
        print("Closing displays and saving settings to " + DEFAULT_SETTINGS_FILE_NAME + "...")
        for (rw <- windows) {
            val displays = settings.get(DISPLAYS_NAME).asInstanceOf[java.util.Map[String, java.util.Map[String, String]]]
            val curDisplay = displays.get(rw.title)
            curDisplay.put(X_SCR_STRING, rw.x.toString)
            curDisplay.put(Y_SCR_STRING, rw.y.toString)
            curDisplay.put(W_SCR_STRING, rw.w.toString)
            curDisplay.put(H_SCR_STRING, rw.h.toString)
            rw.killWindow()
        }
        StringUtils.string2File(DEFAULT_SETTINGS_FILE_NAME, new XmlBuilder().addMap(settings).toPrettyString())
        println("ok")

    }

}
