package se.gigurra.gpt.keyreceiver

import com.sun.jna.platform.win32.User32
import com.sun.jna.platform.win32.WinDef.{DWORD, WORD}
import com.sun.jna.platform.win32.WinUser.{INPUT, KEYBDINPUT}
import se.culvertsoft.mnet.{Message, NodeSettings}
import se.culvertsoft.mnet.api.{Connection, Route}
import se.culvertsoft.mnet.backend.WebsockBackendSettings
import se.culvertsoft.mnet.client.MNetClient
import se.gigurra.gpt.common.{NetworkNames, Serializer}
import se.gigurra.gpt.model.keys.common.KeyMessage

object KeyReceiver {

  def main(args: Array[String]) {

    val server = new MNetClient(
      new WebsockBackendSettings().setListenPort(8052),
      new NodeSettings().setName(NetworkNames.KEY_RECEIVER)) {

      override def handleMessage(msg: Message, connection: Connection, route: Route) {

        if (route != null && route.name == NetworkNames.KEY_TRANSMITTER) {

          Serializer.read[KeyMessage](msg) match {
            case Some(msg) =>
              if (msg.hasValue) {
                val inp = new INPUT()
                inp.input.setType(classOf[KEYBDINPUT])
                inp.`type` = new DWORD(INPUT.INPUT_KEYBOARD)
                var inFlags = msg.getFlags
                if ((inFlags & 0x80) == 0x80) {
                  inFlags ^= 0x80
                  inFlags |= 0x02
                }
                inp.input.ki.dwFlags = new DWORD(inFlags)
                inp.input.ki.time = new DWORD(0)
                inp.input.ki.wScan = new WORD(msg.getScanCode)
                inp.input.ki.wVk = new WORD(msg.getVCode)
                User32.INSTANCE.SendInput(new DWORD(1), Array(inp), inp.size())
              }
            case _ =>
          }

        }
      }

      start()
    }
  }

}
