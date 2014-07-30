package ymfde

import java.nio.ByteBuffer

trait JpgCallback {
    def handleFinishedJpgMsg(bb: ByteBuffer)
}

class JpegMsgCombiner(
    private val callback: JpgCallback) {
    private val bbFull = ByteBuffer.allocate(20 * 1024 * 1024)
    private var prevMsg: Option[JpgMsg] = None

    def reset() {
        bbFull.clear()
        prevMsg = None
    }

    def append(msg: JpgMsg) {
        bbFull.put(msg.srcBytes, msg.dataBytesOffs, JpgMsg.DATA_BLOCK_SIZE)
        prevMsg = Some(msg)
    }

    def handleNewSubImage(msg: JpgMsg) {

        prevMsg match {

            case Some(prevMsg) =>

                if (prevMsg.imgId == msg.imgId && prevMsg.partId + 1 == msg.partId) {

                    append(msg)

                    if (msg.partId + 1 == msg.totalFrameParts) {
                        callback.handleFinishedJpgMsg(bbFull)
                        reset()
                    }

                } else {
                    reset()
                }

            case _ =>
                reset()
                if (msg.partId == 0)
                    append(msg)

        }

    }

}
