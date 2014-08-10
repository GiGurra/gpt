package se.gigurra.gpt.keytransmitter

import java.io.ByteArrayOutputStream
import java.io.FileInputStream
import java.nio.ByteBuffer

import com.sun.jna.platform.win32.Kernel32
import com.sun.jna.platform.win32.User32
import com.sun.jna.platform.win32.WinDef.LRESULT
import com.sun.jna.platform.win32.WinDef.WPARAM
import com.sun.jna.platform.win32.WinUser.KBDLLHOOKSTRUCT
import com.sun.jna.platform.win32.WinUser.LowLevelKeyboardProc
import com.sun.jna.platform.win32.WinUser.MSG
import com.sun.jna.platform.win32.WinUser.WH_KEYBOARD_LL

import se.culvertsoft.mgen.javapack.serialization.JsonReader
import se.culvertsoft.mgen.javapack.serialization.JsonWriter
import se.gigurra.gpt.model.ClassRegistry
import se.gigurra.gpt.model.keys.common.KeyMessage
import se.gigurra.gpt.model.keys.transmitter.KeyTransmitterCfg
import se.gigurra.libgurra.concurrent.ThreadBase
import se.gigurra.libgurra.net.types.ManagedByteClient
import se.gigurra.libgurra.parsing.types.BasicMessage

object KeyTransmitter {

  @volatile var quit = false
  val classRegistry = new ClassRegistry
  val buffer = new ByteArrayOutputStream
  val serializer = new JsonWriter(buffer, classRegistry)

  def main(args: Array[String]) {

    val hm = Kernel32.INSTANCE.GetModuleHandle(null);
    val reader = new JsonReader(new FileInputStream("config.json"), classRegistry);
    val settings = reader.readObject(classOf[KeyTransmitterCfg]);
    val ip = settings.getTarget().getIp();
    val port = settings.getTarget().getPort();

    val client = new ManagedByteClient(ip, port /* 8052 */ ) {
      def send(msg: KeyMessage) {
        synchronized {
          serializer.writeObject(msg)
          val out = buffer.toByteArray()
          buffer.reset()
          send(ByteBuffer.wrap(BasicMessage.fromData(out).allBytes))
        }
      }
      start()
    }

    new ThreadBase {
      override def run() {
        while (!quit) {
          Thread.sleep(500)
          try {
            if (client.isConnected) {
              client.send(new KeyMessage)
            }
          } catch {
            case e: Exception => e.printStackTrace()
          }
        }
      }
      start()
    }

    val lpfn = new LowLevelKeyboardProc() {
      override def callback(nCode: Int, wPar: WPARAM, lp: KBDLLHOOKSTRUCT): LRESULT = {

        client.synchronized {
          val msg = new KeyMessage
          msg.setValue(wPar.intValue)
          msg.setFlags(lp.flags)
          msg.setScanCode(lp.scanCode)
          msg.setVCode(lp.vkCode)
          try {
            if (client.isConnected) {
              client.send(msg)
            }
          } catch {
            case e: Exception =>
              e.printStackTrace()
          }

          return User32.INSTANCE.CallNextHookEx(null, nCode, wPar, lp.getPointer())
        }

      }
    };

    val hHook = User32.INSTANCE.SetWindowsHookEx(WH_KEYBOARD_LL, lpfn, hm, 0);
    if (hHook == null) {
      System.err.println("Failed to create keyboard hook, bailing!");
      System.exit(1)
    }

    val msg = new MSG();
    while (!quit) {
      val result = User32.INSTANCE.GetMessage(msg, null, 0, 0);
      if (result == -1 || result == 0) {
        System.out.println("Exiting, GetMessage returned " + result);
        quit = true;
      } else {
        User32.INSTANCE.TranslateMessage(msg);
        User32.INSTANCE.DispatchMessage(msg);
      }
      Thread.sleep(1);
    }

    if (User32.INSTANCE.UnhookWindowsHookEx(hHook)) {
      System.out.println("Unhooked");
    }

    System.exit(0)

  }
}