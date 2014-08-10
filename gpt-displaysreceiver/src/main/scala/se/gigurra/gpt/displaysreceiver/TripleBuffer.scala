package se.gigurra.gpt.displaysreceiver

class TripleBuffer[T](
    val buffers: Array[T],
    var newFrame: Boolean) {

    private def swap(i1: Int, i2: Int) {
        val t = buffers(i1)
        buffers(i1) = buffers(i2)
        buffers(i2) = t
    }

    def getFrontBuffer(): T = {
        synchronized {
            if (newFrame) {
                swap(1, 2)
                newFrame = false
            }
            buffers(2)
        }
    }

    def paint(fPaint: T => Unit) {
        val bb = synchronized(buffers(0))
        fPaint(bb)
        synchronized {
            swap(0, 1)
            newFrame = true
        }
    }

    def resetBuffers(a: T, b: T, c: T) {
        synchronized {
            buffers(0) = a
            buffers(1) = b
            buffers(2) = c
        }
    }

}
