package se.gigurra.gpt.keyreceiver;

import java.io.ByteArrayInputStream

import com.sun.jna.platform.win32.User32
import com.sun.jna.platform.win32.WinDef.DWORD
import com.sun.jna.platform.win32.WinDef.WORD
import com.sun.jna.platform.win32.WinUser.INPUT
import com.sun.jna.platform.win32.WinUser.KEYBDINPUT

import se.culvertsoft.mgen.javapack.serialization.JsonReader
import se.gigurra.gpt.model.ClassRegistry
import se.gigurra.gpt.model.keys.common.KeyMessage
import se.gigurra.libgurra.net.types.BasicTcpServer
import se.gigurra.libgurra.parsing.types.BasicMessage

object KeyReceiver {

  val classRegistry = new ClassRegistry

  def main(args: Array[String]) {
    new BasicTcpServer(8052) {
      override protected def handleMessage(c: Client, msg: BasicMessage) {

        val reader = new JsonReader(new ByteArrayInputStream(msg.data), classRegistry)

        try {
          val msg = reader.readObject(classOf[KeyMessage])
          if (msg.hasValue()) {
            val inp = new INPUT();
            inp.input.setType(classOf[KEYBDINPUT]);
            inp.`type` = new DWORD(INPUT.INPUT_KEYBOARD);
            var inFlags = msg.getFlags
            if ((inFlags & 0x80) == 0x80) {
              inFlags ^= 0x80;
              inFlags |= 0x02;
            }
            inp.input.ki.dwFlags = new DWORD(inFlags);
            inp.input.ki.time = new DWORD(0);
            inp.input.ki.wScan = new WORD(msg.getScanCode);
            inp.input.ki.wVk = new WORD(msg.getVCode);
            User32.INSTANCE.SendInput(new DWORD(1), Array(inp), inp.size());
          }
        } catch {
          case e: Exception =>
            System.err.println("Bad data in stream:");
            e.printStackTrace();
        }
      }
    }.run();
  }

}
