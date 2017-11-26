using System;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace IceBrowser {
    public static class Utility {
        [DllImport ("gdi32.dll", EntryPoint = "DeleteObject")]
        [
            return :MarshalAs (UnmanagedType.Bool)
        ]
        public static extern bool DeleteObject ([In] IntPtr hObject);
        /// <summary>
        /// Bitmap Image to ImageSource Conversion
        /// </summary>
        /// <param name="bmp">Bitmap</param>
        /// <returns>ImageSource</returns>
        /// <see cref="http://www.nuits.jp/entry/2016/10/17/181232"/>
        public static ImageSource ToImageSource (this Bitmap bmp) {
            var handle = bmp.GetHbitmap ();
            try {
                return System.Windows.Interop.Imaging.CreateBitmapSourceFromHBitmap (handle, IntPtr.Zero, Int32Rect.Empty, BitmapSizeOptions.FromEmptyOptions ());
            } finally { DeleteObject (handle); }
        }
        /// <summary>
        /// Sound Player by winmm.dll
        /// </summary>
        /// <see cref="http://dobon.net/vb/dotnet/programing/playembeddedwave.html"/>
        public class WavePlayer {
            public enum PlaySoundFlags : int {
                    SND_SYNC = 0x0000,
                    SND_ASYNC = 0x0001,
                    SND_NODEFAULT = 0x0002,
                    SND_MEMORY = 0x0004,
                    SND_LOOP = 0x0008,
                    SND_NOSTOP = 0x0010,
                    SND_NOWAIT = 0x00002000,
                    SND_ALIAS = 0x00010000,
                    SND_ALIAS_ID = 0x00110000,
                    SND_FILENAME = 0x00020000,
                    SND_RESOURCE = 0x00040004,
                    SND_PURGE = 0x0040,
                    SND_APPLICATION = 0x0080
                }
                [DllImport ("winmm.dll")]
            private static extern bool PlaySound (
                IntPtr pszSound, IntPtr hmod, PlaySoundFlags fdwSound);
            private GCHandle gcHandle;
            private byte[] waveBuffer = null;
            /// <summary>
            /// Playing sound via winmm.dll
            /// </summary>
            /// <param name="buffer">Byte array containing wave data</param>
            public WavePlayer (byte[] buffer) {
                waveBuffer = buffer;
                gcHandle = GCHandle.Alloc (
                    buffer, GCHandleType.Pinned);
            }
            /// <summary>
            /// Play wave asyncrous
            /// </summary>
            public void Play () {
                PlaySound (gcHandle.AddrOfPinnedObject (), IntPtr.Zero,
                    PlaySoundFlags.SND_MEMORY | PlaySoundFlags.SND_ASYNC);
            }
            /// <summary>
            /// Stop Wave
            /// </summary>
            public void Stop () {
                PlaySound (IntPtr.Zero, IntPtr.Zero, PlaySoundFlags.SND_PURGE);
                // free
                gcHandle.Free ();
                waveBuffer = null;
            }
        }
    }
}
