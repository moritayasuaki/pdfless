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
  poppler::page *m_cur;
  int m_page;
  double m_offx, m_offy, m_scale;
  poppler::image m_img;
  bool m_reverse, m_help;

public:
  pdfless(const char *path) {
    using namespace std;
    using namespace poppler;
    m_path = path;
    m_doc = document::load_from_file(path);
    m_aa = aa_autoinit(&aa_defparams);
    if (!m_aa)
      cerr << "aalib init failed" << endl, exit(1);
    m_reverse = false;
    m_page = 0;
    m_cur = m_doc->create_page(m_page);
    if (!m_cur)
      cerr << "pdf have no page" << endl, exit(1);
    reinit();
  }
  ~pdfless(void) {
    aa_close(m_aa);
    delete m_doc;
  }
  void left(void) {
    m_offx -= 18;
    normalize();
  }
  void right(void) {
    m_offx += 18;
    normalize();
  }
  void up(void) {
    m_offy -= 18;
    normalize();
  }
  void down(void) {
    m_offy += 18;
    normalize();
  }
  void prev_page(void) {
    --m_page;
    if (m_page < 0)
      m_page = 0;
    if (m_cur)
      delete m_cur;
    m_cur = m_doc->create_page(m_page);
    reinit();
    normalize();
    update_image();
  }
  void next_page(void) {
    m_page++;
    if (m_page >= m_doc->pages())
      m_page = m_doc->pages() - 1;
    if (m_cur)
      delete m_cur;
    m_cur = m_doc->create_page(m_page);
    reinit();
    normalize();
    update_image();
  }
  void zoom_in(void) {
    double pre = m_scale;
    m_scale *= 1.25;
    if (m_scale > 32)
      m_scale = 32;
    if (m_scale < 1)
      m_scale = 1;
    m_offx *= m_scale / pre;
    m_offy *= m_scale / pre;
    normalize();
    update_image();
  }
  void zoom_out(void) {
    double pre = m_scale;
    m_scale /= 1.25;
    if (m_scale > 32)
      m_scale = 32;
    if (m_scale < 1)
      m_scale = 1;
    m_offx *= m_scale / pre;
    m_offy *= m_scale / pre;
    normalize();
    update_image();
  }
  void update_image(void) {
    using namespace poppler;
    page_renderer pr;
    aa_resize(m_aa);
    double base;
    if (aa_imgwidth(m_aa) * m_cur->page_rect().height() <
        aa_imgheight(m_aa) * m_cur->page_rect().width())
      base = aa_imgwidth(m_aa) / m_cur->page_rect().width();
    else
      base = aa_imgheight(m_aa) / m_cur->page_rect().height();
    m_img = pr.render_page(m_cur, m_scale * base * 144, m_scale * base * 72, -1,
                           -1, -1, -1);
  }
  void reinit(void) {
    using namespace poppler;
    page_renderer pr;
    m_scale = 1.0;
    double base;
    if (aa_imgwidth(m_aa) * m_cur->page_rect().height() <
        aa_imgheight(m_aa) * m_cur->page_rect().width()) {
      base = aa_imgwidth(m_aa) / m_cur->page_rect().width();
      m_img = pr.render_page(m_cur, m_scale * base * 144, m_scale * base * 72,
                             -1, -1, -1, -1);
      m_offx = aa_imgwidth(m_aa) / 2;
      m_offy = m_img.height() / 2;
    } else {
      base = aa_imgheight(m_aa) / m_cur->page_rect().height();
      m_img = pr.render_page(m_cur, m_scale * base * 144, m_scale * base * 72,
                             -1, -1, -1, -1);
      m_offx = m_img.width() / 2;
      m_offy = aa_imgheight(m_aa) / 2;
    }
  }
  void normalize(void) {
    int iw = aa_imgwidth(m_aa);
    int ih = aa_imgheight(m_aa);
    if (m_offx < 0)
      m_offx = 0;
    if (m_offx >= m_img.width())
      m_offx = m_img.width() - 1;
    if (m_offy < 0)
      m_offy = 0;
    if (m_offy > m_img.height())
      m_offy = m_img.height() - 1;
  }

  void reverse(void) { m_reverse = !m_reverse; }
  int render(void) {
    using namespace poppler;
    page_renderer pr;
    aa_resize(m_aa);
    int iw = aa_imgwidth(m_aa);
    int ih = aa_imgheight(m_aa);
    int bw = m_img.bytes_per_row();
    image::format_enum fmt = m_img.format();
    unsigned char *dat = (unsigned char *)m_img.data();
    for (int iy = 0; iy < ih; iy++)
      for (int ix = 0; ix < iw; ix++) {
        int tx = ix + m_offx - iw / 2;
        int ty = iy + m_offy - ih / 2;
        if (tx < 0 || tx >= m_img.width() || ty < 0 || ty >= m_img.height()) {
          aa_putpixel(m_aa, ix, iy, 0);
        } else {
          int b;
          switch (fmt) {
          case image::format_mono:
            b = dat[bw * ty + tx];
            break;
          case image::format_rgb24:
            b = brightness(dat[bw * ty + 3 * tx + 0], dat[bw * ty + 3 * tx + 1],
                           dat[bw * ty + 3 * tx + 2]);
            break;
          case image::format_argb32:
            b = brightness(dat[bw * ty + 4 * tx + 0], dat[bw * ty + 4 * tx + 1],
                           dat[bw * ty + 4 * tx + 2]);
            break;
          case image::format_invalid:
          default:
            return -1;
          }
          aa_putpixel(m_aa, ix, iy, b);
        }
      }
    aa_fastrender(m_aa, 0, 0, aa_scrwidth(m_aa), aa_scrheight(m_aa));
    aa_printf(m_aa, 0, aa_scrheight(m_aa) - 1, AA_NORMAL,
              "zoom = %f%% x = %f%% y = %f%% ", m_scale * 100,
              m_offx / m_img.width() * 100, m_offy / m_img.height() * 100);
    if (m_help) {
      aa_printf(m_aa, 0, 0, AA_NORMAL, "j : down ");
      aa_printf(m_aa, 0, 1, AA_NORMAL, "k : up ");
      aa_printf(m_aa, 0, 2, AA_NORMAL, "l : right ");
      aa_printf(m_aa, 0, 3, AA_NORMAL, "h : left ");
      aa_printf(m_aa, 0, 4, AA_NORMAL, "d <space> : next page ");
      aa_printf(m_aa, 0, 5, AA_NORMAL, "u : prev page ");
      aa_printf(m_aa, 0, 6, AA_NORMAL, "+ : zoom in ");
      aa_printf(m_aa, 0, 7, AA_NORMAL, "- : zoom out ");
      aa_printf(m_aa, 0, 8, AA_NORMAL, "r : reverse color ");
      aa_printf(m_aa, 0, 9, AA_NORMAL, "q : quit ");
    }
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
  void help(void) { m_help = !m_help; }
};

int main(int argc, char **argv) {
  using namespace std;
  if (argc < 2)
    cerr << "Please input pdf file" << endl, exit(1);
  pdfless pl(argv[1]);
  while (1) {
    if (pl.render())
        exit(1);
    pl.flush();
    switch (getchar()) {
    case 'j':
      pl.down();
      break;
    case 'd':
    case ' ':
      pl.next_page();
      break;
    case 'u':
      pl.prev_page();
      break;
    case 'k':
      pl.up();
      break;
    case 'h':
      pl.left();
      break;
    case 'l':
      pl.right();
      break;
    case '+':
      pl.zoom_in();
      break;
    case '-':
      pl.zoom_out();
      break;
    case 'r':
      pl.reverse();
      break;
    case '?':
      pl.help();
      break;
    case 'q':
      return 0;
    }
  }
}
