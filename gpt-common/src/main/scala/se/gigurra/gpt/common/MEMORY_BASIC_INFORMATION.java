package se.gigurra.gpt.common;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import com.sun.jna.platform.win32.BaseTSD.SIZE_T;
import com.sun.jna.platform.win32.WinDef.DWORD;

public class MEMORY_BASIC_INFORMATION extends Structure {

	public Pointer BaseAddress = null;
	public Pointer AllocationBase = null;
	public DWORD AllocationProtect = null;
	public SIZE_T RegionSize = null;
	public DWORD State = null;
	public DWORD Protect = null;
	public DWORD Type = null;

	@Override
	protected java.util.List<String> getFieldOrder() {
		final java.util.List<String> out = new java.util.ArrayList<String>();
		out.add("BaseAddress");
		out.add("AllocationBase");
		out.add("AllocationProtect");
		out.add("RegionSize");
		out.add("State");
		out.add("Protect");
		out.add("Type");
		return out;
	}

}