package keyboardcommandreceiver;

import yodautils.json.yoda.BaseBean;
import yodautils.keyboard.KeyboardMsg;
import yodautils.nioNet.TcpServer;
import yodautils.nioNet.rulesets.P2pServerRuleset;
import yodautils.parser.ParsedMessage;
import yodautils.parser.StandardMessage;

import com.sun.jna.platform.win32.User32;
import com.sun.jna.platform.win32.WinDef.DWORD;
import com.sun.jna.platform.win32.WinDef.WORD;
import com.sun.jna.platform.win32.WinUser.INPUT;
import com.sun.jna.platform.win32.WinUser.KEYBDINPUT;

public class KeyComReceiver {

	static final P2pServerRuleset.MsgCallback keybMsgCallback = new P2pServerRuleset.MsgCallback() {
		
		@Override
		public void handle(ParsedMessage pm) {

			final StandardMessage sm = new StandardMessage(pm.getBytes(), false);
			try {
				final BaseBean bean = sm.fromJsonToBean();
				if (bean instanceof KeyboardMsg) {
					final KeyboardMsg kbm = (KeyboardMsg) bean;
					if (kbm.vkey >= 0) {
						INPUT inp = new INPUT();
						inp.input.setType(KEYBDINPUT.class);
						inp.type = new DWORD(INPUT.INPUT_KEYBOARD);
						int inFlags = kbm.flags;
						if ((inFlags & 0x80) == 0x80) {
							inFlags ^= 0x80;
							inFlags |= 0x02;
						}
						inp.input.ki.dwFlags = new DWORD(inFlags);
						inp.input.ki.time = new DWORD(0);
						inp.input.ki.wScan = new WORD(kbm.scanCode);
						inp.input.ki.wVk = new WORD(kbm.vkey);
						User32.INSTANCE.SendInput(new DWORD(1), new INPUT[] { inp }, inp.size());
					}
				}
			} catch (Exception e) {
				System.err.println("Bad data in stream:");
				e.printStackTrace();
			}
		}
	};

	public static void main(String[] args) throws Exception {
		new TcpServer(8052, new P2pServerRuleset(keybMsgCallback)).run();
	}

}
