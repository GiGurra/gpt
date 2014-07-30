package ymfde

import java.nio.ByteBuffer

import yodautils.crc.Crc32Impl

object JpgMsg {

    val JPG_MSG_SIZE = 1056
    val DATA_BLOCK_SIZE = 1024

    def apply(_src: Array[Byte], checkChecksum: Boolean): Option[JpgMsg] = {

        val srcBytes = _src

        if (srcBytes.length < JPG_MSG_SIZE) {
            return None
        }

        val bbIn = ByteBuffer.wrap(srcBytes)

        val key = bbIn.getInt()
        val pad = bbIn.getInt()
        val size = bbIn.getInt()
        val imgId = bbIn.getInt()
        val partId = bbIn.getInt()
        val totalFrameParts = bbIn.getInt()
        val dataBytesOffs = bbIn.position
        bbIn.position(dataBytesOffs + DATA_BLOCK_SIZE)
        val nSignBytes = bbIn.getInt()
        val checksum = bbIn.getInt()

        if (nSignBytes <= 0 || nSignBytes > DATA_BLOCK_SIZE) {
            return None
        } else if (checkChecksum && Crc32Impl.calcCrc(bbIn, 1056 - 4) != checksum) {
            return None
        }

        return Some(new JpgMsg(
            key,
            pad,
            size,
            imgId,
            partId,
            totalFrameParts,
            srcBytes,
            dataBytesOffs,
            nSignBytes,
            checksum))

    }

}

class JpgMsg(
    val key: Int,
    val pad: Int,
    val size: Int,
    val imgId: Int,
    val partId: Int,
    val totalFrameParts: Int,
    val srcBytes: Array[Byte],
    val dataBytesOffs: Int,
    val nSignificantBytes: Int,
    val checksum: Int)
