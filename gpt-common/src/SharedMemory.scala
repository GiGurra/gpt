package shmutils

import com.sun.jna.Native
import com.sun.jna.Pointer
import com.sun.jna.platform.win32.BaseTSD.SIZE_T
import com.sun.jna.platform.win32.Kernel32
import com.sun.jna.platform.win32.WinBase
import com.sun.jna.platform.win32.WinDef.DWORD
import com.sun.jna.platform.win32.WinNT
import com.sun.jna.platform.win32.WinNT.HANDLE

object SharedMemory {

    Native.register("Kernel32")

    val VIEW_ACCESS = WinNT.SECTION_MAP_READ | WinNT.SECTION_MAP_WRITE
    val PAGE_ACCESS = WinNT.PAGE_READWRITE
    val INVALID_HANDLE_VALUE = WinBase.INVALID_HANDLE_VALUE

    @native def OpenFileMappingA(access: DWORD, bInheritHandle: Boolean, s: String): Long
    @native def VirtualQuery(p: Pointer, infoOut: MEMORY_BASIC_INFORMATION, sz: SIZE_T): Long
    @native def FlushViewOfFile(p: Pointer, n: SIZE_T): Int

    private def validate[T](handle: T, fGetPtr: T => Pointer): Option[T] = {
        if (Pointer.nativeValue(fGetPtr(handle)) != 0) {
            Some(handle)
        } else {
            None
        }
    }

    def openHandle(name: String, sz: Int, mayCreateNew: Boolean): Option[HANDLE] = {
        if (mayCreateNew) {
            validate(
                Kernel32.INSTANCE.CreateFileMapping(INVALID_HANDLE_VALUE, null, PAGE_ACCESS, 0, sz, name),
                (h: HANDLE) => h.getPointer())
        } else {
            validate(
                new HANDLE(new Pointer(OpenFileMappingA(new DWORD(VIEW_ACCESS), false, name))),
                (h: HANDLE) => h.getPointer())
        }
    }

    def findSz(ptr: Pointer): Int = {
        if (ptr != null) {
            val info = new MEMORY_BASIC_INFORMATION()
            VirtualQuery(ptr, info, new SIZE_T(info.size()))
            info.RegionSize.intValue()
        } else {
            0
        }
    }

    def mapView(handle: HANDLE): Option[Pointer] = {
        validate(
            Kernel32.INSTANCE.MapViewOfFile(handle, VIEW_ACCESS, 0, 0, 0),
            (p: Pointer) => p)
    }

}

class SharedMemory(
    val name: String, szIfNew: Int, allowNew: Boolean) {
    val handle: Option[HANDLE] = SharedMemory.openHandle(name, szIfNew, allowNew)
    val view: Option[Pointer] = handle.flatMap(SharedMemory.mapView(_))
    val size: Int = view.map(SharedMemory.findSz(_)).getOrElse(0)
    private var _valid: Boolean = handle.isDefined && view.isDefined && size > 0

    if (!valid) {
        close()
    }

    def valid() = {
        _valid
    }

    def close() {
        _valid = false
        view.foreach(Kernel32.INSTANCE.UnmapViewOfFile(_))
        handle.foreach(Kernel32.INSTANCE.CloseHandle(_))
    }

    def flush() {
        view.foreach(SharedMemory.FlushViewOfFile(_, new SIZE_T(size)))
    }

    def write(data: Array[Byte], offs: Int, n: Int) {
        view.foreach(_.write(0, data, offs, math.min(n, size)))
    }

    def read(ints: Array[Int], n: Int) {
        view.foreach(_.read(0, ints, 0, n))
    }

    def read(bytes: Array[Byte], n: Int) {
        read(bytes, 0, n)
    }

    def read(bytes: Array[Byte], offs: Int, n: Int) {
        view.foreach(_.read(0, bytes, offs, n))
    }

    def copy(): Array[Byte] = {
        val out = new Array[Byte](size.toInt)
        read(out, math.min(out.length, size))
        out
    }

    def getPtr() = {
        view
    }

    def getByteBuffer() = {
        getPtr().map(_.getByteBuffer(0, size))
    }

    override def toString(): String = {
        s"super.toString(), size: $size"
    }

}
