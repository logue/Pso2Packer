using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Forms;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using MahApps.Metro.Controls;

namespace IceBrowser {
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : MetroWindow {
        private WavePlayer wave;
        private TreeNode m_Root;
        private IceResourceSet m_Pack;
        private IceResource Res;

        public MainWindow () {
            InitializeComponent ();
        }
        /***********************************************************************************************/
        private void InsertFileNode (uint id) {
            IceResource pr = m_Pack.GetFileByIndex (id);
            if (pr != null) {
                string filePath = pr.GetName ();
                string[] paths = filePath.Split ('\\');
                TreeNode node = m_Root;
                for (uint i = 0; i < paths.Length; ++i) {
                    int index = node.Nodes.IndexOfKey (paths[i]);
                    if (index < 0) {
                        // Add( name , text)
                        string Ext = System.IO.Path.GetExtension (@paths[i]);
                        if (Ext != "") {
                            switch (Ext) {
                                default : if (@paths[i] == "vf.dat") {
                                    node = node.Nodes.Add (paths[i], paths[i], 3, 3);
                                    PreviewById (id);
                                }
                                else {
                                    node = node.Nodes.Add (paths[i], paths[i], 1, 1);
                                }
                                break;
                                case ".txt":
                                        node = node.Nodes.Add (paths[i], paths[i], 2, 2);
                                    break;
                                case ".xml":
                                        case ".trn":
                                        node = node.Nodes.Add (paths[i], paths[i], 3, 3);
                                    break;
                                case ".jpg":
                                        case ".psd":
                                        case ".bmp":
                                        case ".dds":
                                        case ".gif":
                                        case ".png":
                                        node = node.Nodes.Add (paths[i], paths[i], 4, 4);
                                    break;
                                case ".ttf":
                                        case ".ttc":
                                        node = node.Nodes.Add (paths[i], paths[i], 5, 5);
                                    break;
                                case ".wav":
                                        case ".mp3":
                                        node = node.Nodes.Add (paths[i], paths[i], 6, 6);
                                    break;
                            }
                        } else {
                            node = node.Nodes.Add (paths[i], paths[i]);
                        }
                        node.Tag = id;
                    } else {
                        node = node.Nodes[index];
                    }
                }
            }
        }
        private void UnpackById (uint id) {
            Res = m_Pack.GetFileByIndex (id);
            if (Res != null) {
                w.UnpackFile (Res);
            }
        }
        private void UnpackByName (string name) {
            Res = m_Pack.GetFileByName (name);
            if (Res != null) {
                w.UnpackFile (Res);
            }
        }
        private void PreviewById (uint id) {
            PackResource Res = m_Pack.GetFileByIndex (id);
            Status.Text = Properties.Resources.Str_LoadingPreview;
            this.Update ();
            if (Res != null) {
                PicturePanel.Hide ();
                hexBox.ResetText ();
                TextView.Hide ();
                pPlay.Hide ();
                String InternalName = Res.GetName ();
                string Ext = System.IO.Path.GetExtension (@InternalName);
                // loading file content.
                byte[] buffer = new byte[Res.GetSize ()];
                Res.GetData (buffer);
                Res.Close ();
                switch (Ext) {
                    case ".dds":
                    case ".jpg":
                    case ".gif":
                    case ".png":
                    case ".bmp":
                        string Info = "";
                        if (Ext == ".dds") {
                            Bitmap bmp = DDSDataToBMP (buffer);
                            Info = "DDS (Direct Draw Surfice)";
                            PictureView.Image = bmp;
                        } else {
                            switch (Ext) {
                                case ".jpg":
                                    Info = "JPEG";
                                    break;
                                case ".gif":
                                    Info = "GIF";
                                    break;
                                case ".bmp":
                                    Info = "Bitmap";
                                    break;
                                case ".png":
                                    Info = "PNG (Portable Network Graphic)";
                                    break;
                            }
                            var ms = new MemoryStream (buffer);
                            PictureView.Image = Image.FromStream (ms);
                            ms.Dispose ();
                        }
                        PictureView.Update ();
                        Status.Text = String.Format ("{0} Image file. ({1} x {2})", Info, PictureView.Width, PictureView.Height);
                        PictureView.SizeMode = PictureBoxSizeMode.AutoSize;
                        PicturePanel.AutoScroll = true;
                        PicturePanel.Update ();
                        PicturePanel.Show ();
                        break;
                    case ".xml":
                    case ".html":
                    case ".txt":
                    case ".trn":
                        string text = Encoding.Unicode.GetString (buffer);
                        TextView.Clear ();
                        TextView.Text = text;
                        TextView.Update ();
                        TextView.Show ();
                        Status.Text = String.Format ("Ascii file.");
                        break;
                    case ".wav":
                    case ".mp3":
                        pPlay.Show ();
                        // http://msdn.microsoft.com/en-us/library/ms143770%28v=VS.100%29.aspx
                        this.wave = new WavePlayer (buffer);
                        this.wave.Play ();
                        Status.Text = "Sound file.";
                        break;
                    default:
                        if (InternalName == "vf.dat") {
                            TextView.Clear ();
                            TextView.Text = Encoding.ASCII.GetString (buffer);
                            TextView.Update ();
                            TextView.Show ();
                            Status.Text = "Version infomation.";
                        } else {
                            DynamicByteProvider d = new DynamicByteProvider (buffer);
                            hexBox.ByteProvider = d;
                            Status.Text = "Unknown file.";
                        }
                        break;
                }
            }
            this.Update ();
        }
        /// <summary>
        /// Converts an in-memory image in DDS format to a System.Drawing.Bitmap
        /// object for easy display in Windows forms.
        /// </summary>
        /// <param name="DDSData">Byte array containing DDS image data</param>
        /// <returns>A Bitmap object that can be displayed</returns>
        public static Bitmap DDSDataToBMP (byte[] DDSData) {
            // Create a DevIL image "name" (which is actually a number)
            int img_name;
            Il.ilGenImages (1, out img_name);
            Il.ilBindImage (img_name);
            // Load the DDS file into the bound DevIL image
            Il.ilLoadL (Il.IL_DDS, DDSData, DDSData.Length);
            // Set a few size variables that will simplify later code
            int ImgWidth = Il.ilGetInteger (Il.IL_IMAGE_WIDTH);
            int ImgHeight = Il.ilGetInteger (Il.IL_IMAGE_HEIGHT);
            Rectangle rect = new Rectangle (0, 0, ImgWidth, ImgHeight);
            // Convert the DevIL image to a pixel byte array to copy into Bitmap
            Il.ilConvertImage (Il.IL_BGRA, Il.IL_UNSIGNED_BYTE);
            // Create a Bitmap to copy the image into, and prepare it to get data
            Bitmap bmp = new Bitmap (ImgWidth, ImgHeight);
            BitmapData bmd =
                bmp.LockBits (rect, ImageLockMode.WriteOnly, PixelFormat.Format32bppArgb);
            // Copy the pixel byte array from the DevIL image to the Bitmap
            Il.ilCopyPixels (0, 0, 0,
                Il.ilGetInteger (Il.IL_IMAGE_WIDTH),
                Il.ilGetInteger (Il.IL_IMAGE_HEIGHT),
                1, Il.IL_BGRA, Il.IL_UNSIGNED_BYTE,
                bmd.Scan0);
            // Clean up and return Bitmap
            Il.ilDeleteImages (1, ref img_name);
            bmp.UnlockBits (bmd);
            return bmp;
        }
        /// <summary>
        /// Playing sound via winmm.dll
        /// </summary>
        /// <param name="buffer">Byte array containing wave data</param>
        // http://dobon.net/vb/dotnet/programing/playembeddedwave.html
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
                [System.Runtime.InteropServices.DllImport ("winmm.dll")]
            private static extern bool PlaySound (
                IntPtr pszSound, IntPtr hmod, PlaySoundFlags fdwSound);
            private System.Runtime.InteropServices.GCHandle gcHandle;
            private byte[] waveBuffer = null;
            public WavePlayer (byte[] buffer) {
                waveBuffer = buffer;
                gcHandle = System.Runtime.InteropServices.GCHandle.Alloc (
                    buffer, System.Runtime.InteropServices.GCHandleType.Pinned);
            }
            public void Play () {
                // Play wave asyncrous
                PlaySound (this.gcHandle.AddrOfPinnedObject (), IntPtr.Zero,
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
    public class Model_TreeViewItem : TreeViewItem {
        public DirectoryInfo _Directory { get; set; }
        private bool _Expanded { get; set; } = false;
        public ReactiveProperty<Model_TreeViewItem> _SelectionItem { get; set; } = new ReactiveProperty<Model_TreeViewItem> ();

        public Model_TreeViewItem (string path) {
            this._Directory = new DirectoryInfo (path);
            if (_Directory.GetDirectories ().Count () > 0) {
                this.Items.Add (new TreeViewItem ());
                this.Expanded += Model_TreeViewItem_Expanded;
            }
            this.Header = CreateHeader ();
            this.Selected += Model_TreeViewItem_Selected;
        }

        private void Model_TreeViewItem_Expanded (object sender, RoutedEventArgs e) {
            if (!_Expanded) {
                this.Items.Clear ();
                foreach (DirectoryInfo dir in _Directory.GetDirectories ()) {
                    if (dir.Attributes == FileAttributes.Directory) {
                        this.Items.Add (new Model_TreeViewItem (dir.FullName));
                    }
                }
                _Expanded = true;
            }
        }

        private StackPanel CreateHeader () {
            StackPanel sp = new StackPanel () { Orientation = Orientation.Horizontal };
            sp.Children.Add (new Image () {
                Source = new BitmapImage (new Uri (@"Resources\Folder.ico", UriKind.Relative)),
                    Width = 15,
                    Height = 18,
            });
            sp.Children.Add (new TextBlock () { Text = _Directory.Name });
            return sp;
        }

        private void Model_TreeViewItem_Selected (object sender, RoutedEventArgs e) {
            _SelectionItem.Value = (this.IsSelected) ? this : (Model_TreeViewItem) e.Source;
        }
    }
}
