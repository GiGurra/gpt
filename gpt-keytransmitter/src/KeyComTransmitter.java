package keyboardhook;

import static com.sun.jna.platform.win32.WinUser.WH_KEYBOARD_LL;
import yodautils.keyboard.KeyboardMsg;
import yodautils.nioNet.ConnStatus;
import yodautils.nioNet.P2pReconnectingTcpClient;
import yodautils.nioNet.ReconnectingTcpClient;
import yodautils.nioNet.rulesets.JsonMessage;
import yodautils.strings.StringUtils;
import yodautils.xml.XmlBuilder;

import com.sun.jna.platform.win32.Kernel32;
import com.sun.jna.platform.win32.User32;
import com.sun.jna.platform.win32.WinDef.HMODULE;
import com.sun.jna.platform.win32.WinDef.LRESULT;
import com.sun.jna.platform.win32.WinDef.WPARAM;
import com.sun.jna.platform.win32.WinUser.HHOOK;
import com.sun.jna.platform.win32.WinUser.KBDLLHOOKSTRUCT;
import com.sun.jna.platform.win32.WinUser.LowLevelKeyboardProc;
import com.sun.jna.platform.win32.WinUser.MSG;

public class KeyComTransmitter {

	public static volatile boolean quit = false;

	public static void main(String[] args) throws Exception {

		final HMODULE hm = Kernel32.INSTANCE.GetModuleHandle(null);
		final String targetAddr = (String) new XmlBuilder(StringUtils.textFile2String("config.xml")).toMap().get("target");
		final ReconnectingTcpClient rtc = new P2pReconnectingTcpClient(targetAddr, 8052);

		new Thread(new Runnable() {
			public void run() {
				while (!quit) {
					synchronized (rtc) {
						try {
							if (rtc.cycle(System.currentTimeMillis()) == ConnStatus.CONNECTED) {
								rtc.transmit(new JsonMessage(new KeyboardMsg(-1, -1, -1, 1)));
							}
						} catch (Throwable t) { // Disconnect
							t.printStackTrace();
						}
					}
					try {
						Thread.sleep(500);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}
		}).start();

		final LowLevelKeyboardProc lpfn = new LowLevelKeyboardProc() {
			public LRESULT callback(int nCode, WPARAM wPar, KBDLLHOOKSTRUCT lp) {
				synchronized (rtc) {
					if (rtc.getStatus() == ConnStatus.CONNECTED) {
						try {
							rtc.transmit(new JsonMessage(new KeyboardMsg(wPar.intValue(), lp.vkCode, lp.scanCode, lp.flags)));
						} catch (Throwable t) { // disonnected
							t.printStackTrace();
						}
					}
				}
				return User32.INSTANCE.CallNextHookEx(null, nCode, wPar, lp.getPointer());
			}
		};

		final HHOOK hHook = User32.INSTANCE.SetWindowsHookEx(WH_KEYBOARD_LL, lpfn, hm, 0);
		if (hHook == null) {
			System.err.println("Failed to create keyboard hook, bailing!");
			quit = true;
		}

		final MSG msg = new MSG();
		while (!quit) {
			final int result = User32.INSTANCE.GetMessage(msg, null, 0, 0);
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

	}
}
