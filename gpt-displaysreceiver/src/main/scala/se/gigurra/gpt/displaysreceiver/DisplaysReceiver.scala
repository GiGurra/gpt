package se.gigurra.gpt.displaysreceiver

import java.awt.Rectangle
import java.awt.image.BufferedImage
import java.awt.image.DataBufferInt
import java.io.ByteArrayInputStream
import java.io.FileInputStream
import java.io.FileOutputStream
import java.nio.file.Files
import java.nio.file.Paths

import scala.collection.JavaConversions.asScalaBuffer
import scala.collection.JavaConversions.bufferAsJavaList
import scala.collection.mutable.ArrayBuffer

import org.libjpegturbo.turbojpeg.TJDecompressor

import se.culvertsoft.mgen.javapack.serialization.BinaryReader
import se.culvertsoft.mgen.javapack.serialization.JsonReader
import se.culvertsoft.mgen.javapack.serialization.JsonWriter
import se.gigurra.gpt.common.SharedMemory
import se.gigurra.gpt.model.ClassRegistry
import se.gigurra.gpt.model.displays.common.StreamMsg
import se.gigurra.gpt.model.displays.receiver.StreamReceiverCfg
import se.gigurra.libgurra.net.types.BasicTcpServer
import se.gigurra.libgurra.parsing.types.BasicMessage

object DisplaysReceiver {

  val classRegistry = new ClassRegistry
  val tjDec = new TJDecompressor
  val DEFAULT_SETTINGS_FILE_NAME = "config.json"

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

  def main() {

    print("Reading settings from " + DEFAULT_SETTINGS_FILE_NAME + "...")
    val fileData = Files.readAllBytes(Paths.get(DEFAULT_SETTINGS_FILE_NAME))
    val jsonReader = new JsonReader(new FileInputStream(DEFAULT_SETTINGS_FILE_NAME), classRegistry)
    val settings = jsonReader.readObject(classOf[StreamReceiverCfg])

    print("Starting displays...")

    for (display <- settings.getDisplays()) {
      if (display.getActive()) {
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
        windows.add(new RenderWindow( //
          display.getName(), //
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
    val server = new BasicTcpServer(
      8051,
      5.0,
      1 * 1024 * 1024,
      1 * 1024 * 1024) {
      override protected def handleMessage(c: Client, _msg: BasicMessage) {
        val reader = new BinaryReader(new ByteArrayInputStream(_msg.data), classRegistry)
        val msg = reader.readObject(classOf[StreamMsg])
        tjDec.setJPEGImage(msg.getData(), msg.getData().length)
        ensureBiSize(tjDec.getWidth(), tjDec.getHeight())
        swapChain.paint(tjDec.decompress(_, 0))
        issueRedrawWindows()
      }
    }
    val listenerThread = server.start()

    if (settings.getUseShm()) {
      var sm = new SharedMemory(settings.getShmName, 0, false)
      while (allVisible(windows)) {

        if (System.nanoTime() / 1e9 - lastTcpMsgAt < 2.0) {
          Thread.sleep(100)
        } else {

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
    listenerThread.kill()
    listenerThread.join()

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

    new JsonWriter(new FileOutputStream(DEFAULT_SETTINGS_FILE_NAME), classRegistry).writeObject(settings)
    println("ok")

  }

}
