/*
// Thread for spamming the new shm
new Thread() {
  override def run(): Unit = {

    val shm = new SharedMemory("FalconTexturesSharedMemoryArea", 10*1024 + 2400 * 2400 * 4, true)

    val buffer = shm.getByteBuffer().get
    val height = buffer.getInt(3*4)
    val width = buffer.getInt(4*4)
    val bitsPerPixel = buffer.getInt(5*4)/width * 8
    val bytesPerPixel = bitsPerPixel / 8
    require(bitsPerPixel == 32, "Shm is not RBGA format!")

    val srcImg = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB)
    val srcImgData = srcImg.getRaster.getDataBuffer.asInstanceOf[DataBufferInt].getData

    val frameByteSize = 2 * width * height * bytesPerPixel
    val trgData = new Array[Byte](frameByteSize)

    println(s"height $height")
    println(s"width $width")
    println(s"bitsPerPixel $bitsPerPixel")
    println(s"frameByteSize $frameByteSize")


    handleSize(width, height)

    buffer.position(128)
    val shmAsIntBuffer = buffer.asIntBuffer()

    tjCom.setJPEGQuality(70)
    tjCom.setSubsamp(TJ.SAMP_422)
    tjCom.setSourceImage(srcImg, 0, 0, width, height)
    handleSize(width, height)

    var lastSize = -1

    while (windows.forall(_.isAlive)) {


      val t0 = System.currentTimeMillis()
      shmAsIntBuffer.get(srcImgData, 0, srcImgData.length)
      shmAsIntBuffer.position(0)
      val tAfterCopyIntoImg = System.currentTimeMillis()
      val dtCopyIntoImg = tAfterCopyIntoImg - t0
      tjCom.compress(trgData, 0)
      if (lastSize != tjCom.getCompressedSize) {

        tjDec.setSourceImage(trgData, tjCom.getCompressedSize)
        swapChain.paint(tjDec.decompress(_, 0))
        windows.foreach(_.issueUpdate())

        lastSize = tjCom.getCompressedSize
      } else {
        Thread.sleep(1)
      }

    }
  }
  start()
}
*/