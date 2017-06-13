#include <aalib.h>
#include <iostream>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-image.h>
#include <poppler/cpp/poppler-page-renderer.h>
#include <poppler/cpp/poppler-page.h>

class pdfless {
private:
  const char *m_path;
  aa_context *m_aa;
  poppler::document *m_doc;
  int m_pages;
  bool m_reverse;

public:
  pdfless(const char *path) {
    using namespace std;
    m_path = path;
    m_doc = poppler::document::load_from_file(path);
    m_aa = aa_autoinit(&aa_defparams);
    if (!m_aa)
      cerr << "aalib init failed" << endl, exit(1);
    m_pages = m_doc->pages();
    m_reverse = false;
  }
  ~pdfless(void) {
    aa_close(m_aa);
    delete m_doc;
  }
  int render(int pagenum, int cx, int cy, double zoom) {
    using namespace poppler;
    int aw = aa_imgwidth(m_aa);
    int ah = aa_imgheight(m_aa);
    double mw = aa_mmwidth(m_aa);
    double mh = aa_mmheight(m_aa);
    page *page = m_doc->create_page(pagenum);
    double rw = page->page_rect().width();
    double rh = page->page_rect().height();
    double base = aw/rw * 30.0;
    page_renderer pr;
    image img = pr.render_page(page, 2 * base * zoom, base * zoom);
    int iw = img.width();
    int ih = img.height();
    int bw = img.bytes_per_row();
    int offx = (aw - iw + (int)(cx * 0.5 * base))/2;
    int offy = (ah - ih + (int)(cy * base))/2;
    image::format_enum fmt = img.format();
    unsigned char *dat = (unsigned char *)img.data();
    for (int ay = 0; ay < ah; ay++) {
      int iy = ay - offy;
      for (int ax = 0; ax < aw; ax++) {
        int ix = ax - offx;
        unsigned b = 0;
        if (ix < 0 || iw <= ix || iy < 0 || ih <= iy)
          b = 0;
        else
          switch (fmt) {
          case image::format_mono:
            b = dat[bw * iy + ix];
            break;
          case image::format_rgb24:
            b = brightness(dat[bw * iy + 3 * ix + 0], dat[bw * iy + 3 * ix + 1],
                           dat[bw * iy + 3 * ix + 2]);
            break;
          case image::format_argb32:
            b = brightness(dat[bw * iy + 4 * ix + 0], dat[bw * iy + 4 * ix + 1],
                           dat[bw * iy + 4 * ix + 2]);
            break;
          case image::format_invalid:
          default:
            return -1;
          }
        aa_putpixel(m_aa, ax, ay, b);
      }
    }
    aa_fastrender(m_aa, 0, 0, aa_scrwidth(m_aa),
              aa_scrheight(m_aa));
    delete page;
    return 0;
  }
  int clip(int c) {
    if (c >= 256)
      return 255;
    if (c < 0)
      return 0;
    return c;
  }
  unsigned brightness(unsigned r, unsigned g, unsigned b) {
    int l = clip((int)(0.2126 * r + 0.7152 * g + 0.0722 * b));
    if (m_reverse)
      return clip(250 - l);
    return l;
  }
  void flush(void) { aa_flush(m_aa); }
  void help(void) {
    aa_printf(m_aa, 0, 0, AA_NORMAL, "j : down\n");
    aa_printf(m_aa, 0, 1, AA_NORMAL, "k : up\n");
    aa_printf(m_aa, 0, 2, AA_NORMAL, "l : right\n");
    aa_printf(m_aa, 0, 3, AA_NORMAL, "h : left\n");
    aa_printf(m_aa, 0, 4, AA_NORMAL, "d <space> : next page\n");
    aa_printf(m_aa, 0, 5, AA_NORMAL, "u : prev page\n");
    aa_printf(m_aa, 0, 6, AA_NORMAL, "+ : zoom in\n");
    aa_printf(m_aa, 0, 7, AA_NORMAL, "- : zoom out\n");
    aa_printf(m_aa, 0, 8, AA_NORMAL, "r : reverse color\n");
    aa_printf(m_aa, 0, 9, AA_NORMAL, "q : quit\n");
    flush();
    getchar();
  }
  int run(double zoom) {
    using namespace poppler;
    int pagenum = 0;
    int cx = 0;
    int cy = 0;
    if (zoom > 100)
        zoom = 100;
    if (zoom < 1)
        zoom = 1;
    while (1) {
      int ret;
      aa_resize(m_aa);
      ret = render(pagenum, cx, cy, zoom);
      if (ret)
        return ret;
      flush();
      switch (getchar()) {
      case 'j':
        cy -= 1;
        break;
      case 'd':
      case ' ':
        if (pagenum < m_pages - 1)
          pagenum++;
        break;
      case 'u':
        if (0 < pagenum)
          --pagenum;
        break;
      case 'k':
        cy += 1;
        break;
      case 'h':
        cx += 4;
        break;
      case 'l':
        cx -= 4;
        break;
      case '+':
        if (zoom < 100)
          zoom *= 1.1;
        break;
      case '-':
        if (0.1 < zoom)
          zoom /= 1.1;
        break;
      case 'r':
        m_reverse = !m_reverse;
        break;
      case '?':
        help();
        break;
      case 'q':
        return 0;
      }
    }
  }
};

int main(int argc, char **argv) {
  using namespace std;
  if (argc < 2)
    cerr << "Please input pdf file" << endl, exit(1);
  double zoom = 1.0;
  pdfless pdfless(argv[1]);
  if (argc == 3)
    zoom = stod(argv[2]);
  return pdfless.run(zoom);
}
