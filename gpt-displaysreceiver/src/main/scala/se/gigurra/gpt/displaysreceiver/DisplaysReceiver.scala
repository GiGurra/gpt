package se.gigurra.gpt.displaysreceiver

import java.awt.Rectangle
import java.awt.image.BufferedImage
import java.awt.image.DataBufferInt

import scala.collection.JavaConversions.asScalaBuffer
import scala.collection.JavaConversions.bufferAsJavaList
import scala.collection.mutable.ArrayBuffer

import org.libjpegturbo.turbojpeg.TJDecompressor

import se.culvertsoft.mnet.Message
import se.culvertsoft.mnet.NodeSettings
import se.culvertsoft.mnet.api.Connection
import se.culvertsoft.mnet.api.Route
import se.culvertsoft.mnet.backend.WebsockBackendSettings
import se.culvertsoft.mnet.client.MNetClient
import se.gigurra.gpt.common.NetworkNames
import se.gigurra.gpt.common.ReadConfigFile
import se.gigurra.gpt.common.SaveConfigFile
import se.gigurra.gpt.common.Serializer
import se.gigurra.gpt.common.SharedMemory
import se.gigurra.gpt.model.displays.common.StreamMsg
import se.gigurra.gpt.model.displays.receiver.StreamReceiverCfg

object DisplaysReceiver {

  val tjDec = new TJDecompressor
  val DEFAULT_SETTINGS_FILE_NAME = "displays_receiver_cfg.json"

  val windows = new ArrayBuffer[RenderWindow]
  val swapChain = new TripleBuffer[BufferedImage](new Array[BufferedImage](3), false)
  var imgWidth = 0
  var imgHeight = 0
  var lastTcpMsgAt = -10.0

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

  def main(args: Array[String]) {

    print("Reading settings from " + DEFAULT_SETTINGS_FILE_NAME + "...")
    val settings = ReadConfigFile[StreamReceiverCfg](DEFAULT_SETTINGS_FILE_NAME).getOrElse(new StreamReceiverCfg)
    SaveConfigFile(DEFAULT_SETTINGS_FILE_NAME, settings)

    print("Starting displays...")

    for (display <- settings.getDisplays()) {
      println(s"Checking ${display.getName}")
      if (display.getActive()) {
        println("Active")
        val xScr = display.getTarget().getX().toInt
        val yScr = display.getTarget().getY().toInt
        val wScr = display.getTarget().getWidth().toInt
        val hScr = display.getTarget().getHeight().toInt
        val xTex = display.getSource().getX()
        val yTex = display.getSource().getY()
        val wTex = display.getSource().getWidth()
        val hTex = display.getSource().getHeight()
        val border = display.getBorder()
        val aot = display.getAlwaysOnTop()
        val window = new RenderWindow(
          display.getName(),
          new Rectangle(xScr, yScr, wScr, hScr),
          new DoubleRectangle(xTex, yTex, wTex, hTex),
          border,
          aot,
          swapChain)
        windows.add(window)
      }
    }
    println("ok")

    // Start listening on TCP
    val server = new MNetClient(
      new WebsockBackendSettings().setListenPort(8051),
      new NodeSettings().setName(NetworkNames.DISP_RECEIVER)) {

      override def handleMessage(msg_in: Message, connection: Connection, route: Route) {
        println(s"$this got ${msg_in._typeName} from $connection")
        Serializer.read[StreamMsg](msg_in) match {
          case Some(msg) =>
            tjDec.setJPEGImage(msg.getData(), msg.getData().length)
            ensureBiSize(tjDec.getWidth(), tjDec.getHeight())
            swapChain.paint(tjDec.decompress(_, 0))
            issueRedrawWindows()
          case _ => println("But was either not a DataMessage or had no binary data")
        }
      }

      start()
    }

    var sm = if (settings.getUseShm) new SharedMemory(settings.getShmName, 0, false) else null
    while (allVisible(windows)) {

      if (System.nanoTime() / 1e9 - lastTcpMsgAt < 2.0) {
        Thread.sleep(100)
      } else {

        if (settings.getUseShm) {
          if (sm.valid) {

            ensureBiSize(settings.getShmWidth, settings.getShmHeight)

            swapChain.paint { backBuffer =>
              val trgData = backBuffer.getRaster().getDataBuffer().asInstanceOf[DataBufferInt].getData()
              sm.flush()
              sm.read(trgData, settings.getShmWidth * settings.getShmHeight)
            }

            issueRedrawWindows()
            Thread.sleep(19)

          } else {
            sm = new SharedMemory(settings.getShmName, 0, false)
            Thread.sleep(100)
          }
        }

      }

    }

    // Kill recv server
    server.stop()

    // Save xml
    print("Closing displays and saving settings to " + DEFAULT_SETTINGS_FILE_NAME + "...")
    for (rw <- windows) {
      val wset = settings.getDisplays().find(_.getName() == rw.title).get
      wset.getTarget().setX(rw.x)
      wset.getTarget().setY(rw.y)
      wset.getTarget().setWidth(rw.w)
      wset.getTarget().setHeight(rw.h)
      rw.killWindow()
    }

    SaveConfigFile(DEFAULT_SETTINGS_FILE_NAME, settings)
    println("ok")

  }

}
