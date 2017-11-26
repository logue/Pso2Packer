using System;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media.Imaging;
using MahApps.Metro.Controls;
using MahApps.Metro.IconPacks;
using Reactive.Bindings;
using S16.Drawing;
using WpfHexaEditor.Core;
using WpfHexaEditor.Core.Bytes;

namespace IceBrowser {
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : MetroWindow {
        private Utility.WavePlayer wave;
        //private IceResourceSet m_Pack;
        //private IceResource Res;

        public MainWindow () {
            InitializeComponent ();
        }
        /***********************************************************************************************/

        /// <summary>
        /// プレビューを表示
        /// </summary>
        /// <param name="id"></param>
        private void PreviewById (uint id) {
            /*
            PackResource Res = m_Pack.GetFileByIndex (id);
            Status.Content = "Now Loading...";
            if (Res != null) {
                PicturePanel.Visibility = Visibility.Hidden;
                TextView.Visibility = Visibility.Hidden;
                SoundView.Visibility = Visibility.Hidden;
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
                            DDSImage dds = new DDSImage (buffer);
                            Info = "DDS (Direct Draw Surfice)";
                            PictureView.Source = Utility.ToImageSource (dds.BitmapImage);
                            dds.Dispose ();
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
                            BitmapImage bitmapImage = new BitmapImage ();
                            MemoryStream ms = new MemoryStream (buffer);
                            bitmapImage.StreamSource = ms;
                            PictureView.Source = bitmapImage;
                            ms.Dispose ();
                        }
                        Status.Content = String.Format ("{0} Image file. ({1} x {2})", Info, PictureView.Width, PictureView.Height);
                        break;
                    case ".xml":
                    case ".html":
                    case ".txt":
                    case ".trn":
                        string text = Encoding.Unicode.GetString (buffer);
                        TextView.Text = text;
                        Status.Content = String.Format ("Ascii file.");
                        break;
                    case ".wav":
                    case ".mp3":
                        SoundView.Visibility = Visibility.Visible;
                        // http://msdn.microsoft.com/en-us/library/ms143770%28v=VS.100%29.aspx
                        this.wave = new Utility.WavePlayer (buffer);
                        this.wave.Play ();
                        Status.Content = "Sound file.";
                        break;
                    default:
                        Status.Content = "Unknown file.";
                        break;
                }
            }
            */
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
        /// <summary>
        /// フォルダが開いたときにアイコン変更（多分使わない）
        /// </summary>
        private void IconChange () {
            StackPanel sp = (StackPanel) this.Header;
            PackIconFontAwesome Icon = (PackIconFontAwesome) sp.Children[0];
            if (this.IsExpanded) {
                Icon = new PackIconFontAwesome () { Kind = PackIconFontAwesomeKind.FolderOutlinepenOutline };
            } else {
                Icon = new PackIconFontAwesome () { Kind = PackIconFontAwesomeKind.FolderOutlinepenOutline };
            }
        }
        /// <summary>
        /// 項目にアイコンを追加
        /// </summary>
        /// <returns></returns>
        private StackPanel CreateHeader () {
            StackPanel sp = new StackPanel () { Orientation = Orientation.Horizontal };
            PackIconFontAwesomeKind Icon;
            string Ext = System.IO.Path.GetExtension (_Directory.Name);
            switch (Ext) {
                default : Icon = PackIconFontAwesomeKind.Folder;
                break;
                case ".txt":
                        Icon = PackIconFontAwesomeKind.FileTextOutline;
                    break;
                case ".xml":
                        case ".trn":
                        Icon = PackIconFontAwesomeKind.FileCodeOutline;
                    break;
                case ".jpg":
                        case ".psd":
                        case ".bmp":
                        case ".dds":
                        case ".gif":
                        case ".png":
                        Icon = PackIconFontAwesomeKind.FileImageOutline;
                    break;
                case ".ttf":
                        case ".ttc":
                        Icon = PackIconFontAwesomeKind.Font;
                    break;
                case ".wav":
                        case ".mp3":
                        Icon = PackIconFontAwesomeKind.FileAudioOutline;
                    break;
            }
            sp.Children.Add (new PackIconFontAwesome () {
                Kind = Icon,
            });
            sp.Children.Add (new TextBlock () { Text = _Directory.Name });
            return sp;
        }

        private void Model_TreeViewItem_Selected (object sender, RoutedEventArgs e) {
            _SelectionItem.Value = (this.IsSelected) ? this : (Model_TreeViewItem) e.Source;
        }
    }
}
