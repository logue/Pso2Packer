using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IceBrowser {
    class MainWindowViewModel {
        public List<Model_TreeViewItem> VM { get; }
        public MainWindowViewModel () {
            VM = new List<Model_TreeViewItem> () {

                new Model_TreeViewItem (@"C:\Users\yoshiki\Desktop\てすと")
            };
        }
    }
}
