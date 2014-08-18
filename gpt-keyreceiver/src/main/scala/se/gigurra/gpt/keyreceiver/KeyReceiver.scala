package se.gigurra.gpt.keyreceiver;

import com.sun.jna.platform.win32.User32
import com.sun.jna.platform.win32.WinDef.DWORD
import com.sun.jna.platform.win32.WinDef.WORD
import com.sun.jna.platform.win32.WinUser.INPUT
import com.sun.jna.platform.win32.WinUser.KEYBDINPUT

import se.culvertsoft.mnet.Message
import se.culvertsoft.mnet.api.Connection
import se.culvertsoft.mnet.api.Route
import se.culvertsoft.mnet.backend.WebsockBackendSettings
import se.culvertsoft.mnet.client.MNetClient
import se.gigurra.gpt.common.Serializer
import se.gigurra.gpt.model.keys.common.KeyMessage

object KeyReceiver {

  def main(args: Array[String]) {

    val server = new MNetClient(new WebsockBackendSettings().setListenPort(8052)) {

      override def handleMessage(msg: Message, connection: Connection, route: Route) {

        Serializer.read[KeyMessage](msg) match {
          case Some(msg) =>
            try {
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
          case _ =>
        }

      }

      start()
    }
  }

}
