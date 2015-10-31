package se.gigurra.gpt.displaystransmitter

import java.awt.image.{BufferedImage, DataBufferInt}
import java.net.ConnectException
import java.util

import org.libjpegturbo.turbojpeg.{TJ, TJCompressor}
import se.culvertsoft.mnet.client.MNetClient
import se.gigurra.gpt.common._
import se.gigurra.gpt.model.displays.common.StreamMsg
import se.gigurra.gpt.model.displays.transmitter.StreamTransmitterCfg

import scala.collection.JavaConversions._

case class State(
  sourceImg: BufferedImage = null,
  srcImgData: Array[Int] = null,
  compressBuf: Array[Byte] = null,
  frameNumber: Int = 0,
  nextSendTime: Double = 0.0)

object DisplaysTransmitter {

  val DEFAULT_SETTINGS_FILE_NAME = "gpt-displaystransmitter-cfg.json"
  val compressor = new TJCompressor
  var state = State()

  def readShm(shm: FalconTexturesShm): Unit = {
    shm.textureData.get(state.srcImgData).reset()
  }

  def main(args: Array[String]) {

    println("Starting gpt DisplaysTransmitter")
    println("Reading settings from " + DEFAULT_SETTINGS_FILE_NAME + "...")
    val settings = ReadConfigFile[StreamTransmitterCfg](DEFAULT_SETTINGS_FILE_NAME).getOrElse(new StreamTransmitterCfg)
    SaveConfigFile(DEFAULT_SETTINGS_FILE_NAME, settings)
    println("Settings:")
    println(settings)
    println("Starting export")

    val clients = createClients(settings)
    val shm = FalconTexturesShm()

    try {
      while (true) {
        if (shm.hasData) {
          reInit(shm.resolution, settings)
          throttle(settings)
          readShm(shm)
          broadcast(createMsg(), clients)
        }
        else {
          println("Waiting for BMS ..")
          Thread.sleep(2000)
        }
      }
    } catch {
      case ex: Throwable =>
        println("DisplayTransmitter crashed..")
        ex.printStackTrace()
        clients.foreach(_.stop())
    }

    println("Exiting DisplaysTransmitter")

  }

  case class Resolution(width: Int, height: Int, bytesPerPixel: Int) {
    def frameByteSize: Int = width * height * bytesPerPixel
  }

  case class FalconTexturesShm() extends SharedMemory(
    name = "FalconTexturesSharedMemoryArea",
    szIfNew = 10 * 1024 + 2400 * 2400 * 4,
    allowNew = true) {
    require(valid, s"Unable to open or create shared memory $name")

    val raw = getByteBuffer().get
    val textureData = raw.asIntBuffer()

    textureData.position(32)
    textureData.mark()

    def hasData = width > 0 && height > 0 && width <= 2048 && height <= 2048
    def width: Int = raw.getInt(4 * 4)
    def height: Int = raw.getInt(3 * 4)
    def resolution: Resolution = Resolution(width, height, bytesPerPixel)
    def bitsPerPixel = raw.getInt(5 * 4) / width * 8
    def bytesPerPixel = bitsPerPixel / 8
    def frameByteSize = width * height * bytesPerPixel
  }

  def createClients(settings: StreamTransmitterCfg): scala.Seq[_root_.se.culvertsoft.mnet.client.MNetClient] = {
    settings.getTargets.map(t => new MNetClient(NetworkNames.DISP_TRANSMITTER, t.getIp, t.getPort) {
      override def handleError(error: Exception, source: Object): Unit = {
        error match {
          case error: ConnectException => println(s"Texture stream: Unable to connect to ${t.getIp}:${t.getPort}.. retrying..")
          case error => error.printStackTrace()
        }
      }
    }.start())
  }

  def reInit(resolution: Resolution, settings: StreamTransmitterCfg): Unit = {
    if (state.sourceImg == null ||
      state.sourceImg.getWidth != resolution.width ||
      state.sourceImg.getHeight != resolution.height) {

      println(s"BMS detected or export resolution changed ${resolution} .. Adapting ..")
      require(resolution.bytesPerPixel == 4, "Shm is not RBGA format!")

      state = state.copy(sourceImg = new BufferedImage(resolution.width, resolution.height, BufferedImage.TYPE_INT_RGB))
      state = state.copy(srcImgData = state.sourceImg.getRaster.getDataBuffer.asInstanceOf[DataBufferInt].getData)
      state = state.copy(compressBuf = new Array[Byte](2 * resolution.frameByteSize))

      compressor.setJPEGQuality((settings.getJpegQual * 100.0f).toInt)
      compressor.setSubsamp(TJ.SAMP_422)
      compressor.setSourceImage(state.sourceImg, 0, 0, resolution.width, resolution.height)

      state = state.copy(nextSendTime = time)


      println("Resumed texture export!")
    }
  }

  def createMsg(): StreamMsg = {

    state = state.copy(frameNumber = state.frameNumber + 1)

    compressor.compress(state.compressBuf, 0)
    new StreamMsg()
      .setData(util.Arrays.copyOf(state.compressBuf, compressor.getCompressedSize))
      .setFrameNbr(state.frameNumber)
      .setWidth(state.sourceImg.getWidth)
      .setHeight(state.sourceImg.getHeight)
  }

  def broadcast(msg: StreamMsg, clients: Seq[MNetClient]): Unit = {
    for {
      client <- clients
      route <- client.getRoutes
      if route.isConnected && route.name == NetworkNames.DISP_RECEIVER
    } {
      route.send(
        Serializer.writeBinary(msg)
          .setTargetId(route.endpointId)
          .setSenderId(client.id))
    }
  }

  def time = System.nanoTime() / 1e9

  def throttle(settings: StreamTransmitterCfg): Unit = {
    if (settings.hasMaxFps && settings.getMaxFps > 0) {
      while(time < state.nextSendTime)
        Thread.sleep(1)
      state.copy(nextSendTime = state.nextSendTime + 1.0 / settings.getMaxFps)
    }
  }

}
