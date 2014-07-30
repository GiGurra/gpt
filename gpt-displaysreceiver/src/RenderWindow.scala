package ymfde

import java.awt.Component
import java.awt.Frame
import java.awt.Graphics
import java.awt.Graphics2D
import java.awt.Rectangle
import java.awt.RenderingHints
import java.awt.event.WindowEvent
import java.awt.event.WindowListener
import java.awt.image.BufferedImage

import javax.swing.SwingUtilities

class RenderWindow(
    windowTitle: String,
    windowSz: Rectangle,
    ata: DoubleRectangle,
    border: Boolean,
    aot: Boolean,
    swapChain: TripleBuffer[BufferedImage]) {
    private val rs = new RenderSurface()
    private val frame = new Frame()
    @volatile private var updateIssued = false
    init()

    private def init() {
        frame.setSize(windowSz.width, windowSz.height)
        frame.setLocation(windowSz.x, windowSz.y)
        frame.add(rs)
        if (!border) {
            frame.setUndecorated(true)
        }
        if (aot) {
            frame.setAlwaysOnTop(true)
        }
        frame.setTitle(windowTitle)
        frame.setVisible(true)
        frame.addWindowListener(new WindowListener() {
            override def windowOpened(e: WindowEvent) {}
            override def windowIconified(e: WindowEvent) {}
            override def windowDeiconified(e: WindowEvent) {}
            override def windowDeactivated(e: WindowEvent) {}
            override def windowClosed(e: WindowEvent) {}
            override def windowActivated(e: WindowEvent) {}
            override def windowClosing(e: WindowEvent) {
                RenderWindow.this.frame.dispose()
            }
        })
    }

    class RenderSurface extends Component {

        override def update(g: Graphics) {
            paint(g)
        }

        override def paint(g: Graphics) {
            updateIssued = false
            val image = swapChain.getFrontBuffer()
            if (image != null && g != null) {
                val wTex = image.getWidth()
                val hTex = image.getHeight()
                val x0 = (0.5 + ata.x * wTex).toInt
                val x1 = x0 + (ata.w * wTex).toInt
                val y0 = (0.5 + ata.y * hTex).toInt
                val y1 = y0 + (ata.h * hTex).toInt
                val g2d = g.asInstanceOf[Graphics2D]
                g2d.setRenderingHint(RenderingHints.KEY_INTERPOLATION, RenderingHints.VALUE_INTERPOLATION_BILINEAR)
                g2d.drawImage(image, 0, 0, getWidth(), getHeight(), x0, y0, x1, y1, null)
            }
        }
    }

    def issueUpdate() {
        if (!updateIssued) {
            updateIssued = true
            SwingUtilities.invokeLater(new Runnable() {

                override def run() {
                    if (rs.isVisible()) {
                        val g = rs.getGraphics()
                        if (g != null) {
                            rs.update(g)
                            g.dispose()
                        }
                    }
                }
            })
        }
    }

    def isVisible() = {
        frame.isVisible()
    }

    def title() = {
        frame.getTitle()
    }

    def x() = {
        if (frame.isUndecorated()) {
            frame.getX()
        } else {
            frame.getX() - frame.getInsets().left
        }
    }

    def y() = {
        if (frame.isUndecorated()) {
            frame.getY()
        } else {
            frame.getY() - frame.getInsets().top
        }
    }

    def w() = {
        if (frame.isUndecorated()) {
            frame.getWidth()
        } else {
            frame.getWidth() + frame.getInsets().left + frame.getInsets().right
        }
    }

    def h() = {
        if (frame.isUndecorated()) {
            frame.getHeight()
        } else {
            frame.getHeight() + frame.getInsets().top + frame.getInsets().bottom
        }
    }

    def isAlive() = {
        isVisible()
    }

    def killWindow() {
        frame.dispose()
    }

}