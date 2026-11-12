# Python 3 + Pillow: the pictures for Steam's library, for ROTKPlus added as a non-Steam game (on a PC or a Steam
# Machine / Steam Deck). From the project's own art (launcher/art: the banner with the four controllers, the logo,
# the icon) and the Cinzel font; nothing of the game. The game's name is only written, as text.
# Output in launcher/art/steam:
#   capsule_600x900.png   the library's cover (portrait)
#   wide_920x430.png      the wide cover (recent games, Big Picture)
#   hero_1920x620.png     the background at the top of the game's page
#   logo.png              the logo, transparent, over the background
#   icon_256.png          the small icon
# usage: python tools/steam_artwork.py   (from the repository's folder)
import os
from PIL import Image, ImageDraw, ImageFilter, ImageFont, ImageEnhance

HERE = os.path.dirname(os.path.abspath(__file__))
ART = os.path.join(HERE, "..", "launcher", "art")
OUT = os.path.join(ART, "steam")
FONT = os.path.join(HERE, "..", "launcher", "fonts", "Cinzel.ttf")
NIGHT = (8, 11, 20)
SILVER = (214, 226, 246)
SOFT = (170, 190, 220)

banner = Image.open(os.path.join(ART, "banner_rotkplus.jpg")).convert("RGB")      # 1456 x 720
# since 1.5.1 the logo: ROTK, the star of four gems, PLUS (wide, transparent); the banner's old emblem in the middle is
# covered and the logo put in its place
emblem = Image.open(os.path.join(ART, "rotk_cover.png")).convert("RGBA")
emblem = emblem.crop(emblem.getchannel("A").point(lambda v: 255 if v > 8 else 0).getbbox())
icon = Image.open(os.path.join(ART, "icon_rotkplus_1024.png")).convert("RGBA")
# the four controllers of the banner (green, blue, orange, gold), each on its own
PADS = [banner.crop(b) for b in ((40, 88, 445, 372), (1012, 88, 1420, 372), (40, 385, 445, 668), (1012, 380, 1420, 668))]


def soft_paste(dst, src, xy, edge):
    """src pasted with its edges faded out over `edge` pixels"""
    m = Image.new("L", src.size, 0)
    ImageDraw.Draw(m).rectangle((edge, edge, src.width - edge, src.height - edge), fill=255)
    dst.paste(src, xy, m.filter(ImageFilter.GaussianBlur(edge / 2)))


def logo_on(im, width, cx, cy, glow=True):
    """the logo, `width` wide, centred on (cx, cy), with a dark halo under it"""
    e = emblem.resize((width, round(emblem.height * width / emblem.width)), Image.LANCZOS)
    if glow:
        g = Image.new("L", im.size, 0)
        ImageDraw.Draw(g).ellipse((cx - width * 0.62, cy - e.height * 0.95, cx + width * 0.62, cy + e.height * 0.95), fill=170)
        g = g.filter(ImageFilter.GaussianBlur(e.height * 0.45))
        im.paste(Image.new("RGB", im.size, NIGHT), (0, 0), g)
    im.paste(e, (round(cx - e.width / 2), round(cy - e.height / 2)), e)


def cover(im, w, h, cx=0.5, cy=0.5):
    """im scaled to fill w x h, cropped around (cx, cy)"""
    k = max(w / im.width, h / im.height)
    s = im.resize((round(im.width * k), round(im.height * k)), Image.LANCZOS)
    x = min(max(0, round(s.width * cx - w / 2)), s.width - w)
    y = min(max(0, round(s.height * cy - h / 2)), s.height - h)
    return s.crop((x, y, x + w, y + h))


def backdrop(w, h):
    """the banner's old map, blurred and darkened, as a background without the emblem in the way"""
    b = cover(banner, w, h).filter(ImageFilter.GaussianBlur(max(w, h) / 40))
    b = ImageEnhance.Brightness(b).enhance(0.45)
    night = Image.new("RGB", (w, h), NIGHT)
    return Image.blend(b, night, 0.35)


def vignette(im, strength=0.6):
    w, h = im.size
    m = Image.new("L", (w, h), 0)
    d = ImageDraw.Draw(m)
    for i in range(40):
        a = int(255 * strength * (1 - i / 40))
        d.rectangle((i * w // 90, i * h // 90, w - i * w // 90, h - i * h // 90), outline=a)
    m = m.filter(ImageFilter.GaussianBlur(min(w, h) / 12))
    dark = Image.new("RGB", (w, h), (0, 0, 0))
    return Image.composite(dark, im, m)


def text_spaced(d, cx, y, t, size, fill, gap):
    f = ImageFont.truetype(FONT, size)
    widths = [d.textlength(c, font=f) for c in t]
    x = cx - (sum(widths) + gap * (len(t) - 1)) / 2
    for c, w in zip(t, widths):
        d.text((x + 2, y + 2), c, font=f, fill=(0, 0, 0))
        d.text((x, y), c, font=f, fill=fill)
        x += w + gap


def text_centred(d, cx, y, t, size, fill):
    f = ImageFont.truetype(FONT, size)
    w = d.textlength(t, font=f)
    d.text((cx - w / 2 + 2, y + 2), t, font=f, fill=(0, 0, 0))
    d.text((cx - w / 2, y), t, font=f, fill=fill)


def gem(d, cx, cy, r, fill):
    d.polygon([(cx - r, cy), (cx, cy - r), (cx + r, cy), (cx, cy + r)], fill=fill)


def rule(d, cx, y, half, col):
    d.line((cx - half, y, cx - 14, y), fill=col, width=2)
    d.line((cx + 14, y, cx + half, y), fill=col, width=2)
    gem(d, cx, y, 6, SILVER)


def frame(d, w, h):
    d.rectangle((10, 10, w - 11, h - 11), outline=SOFT, width=2)
    d.rectangle((18, 18, w - 19, h - 19), outline=(90, 104, 130), width=1)
    for x, y in ((18, 18), (w - 19, 18), (18, h - 19), (w - 19, h - 19)):
        gem(d, x, y, 7, SILVER)


os.makedirs(OUT, exist_ok=True)

# the cover, portrait: the four controllers two by two, the logo under them, the game's name and what ROTKPlus adds
w, h = 600, 900
im = vignette(backdrop(w, h))
for k, p in enumerate(PADS):
    q = p.resize((262, round(p.height * 262 / p.width)), Image.LANCZOS)
    soft_paste(im, q, (36 + (k % 2) * 266, 52 + (k // 2) * 196), 10)
logo_on(im, 520, w / 2, 520)
d = ImageDraw.Draw(im)
text_spaced(d, w / 2, 640, "THE LORD OF THE RINGS", 22, SOFT, 5)
text_centred(d, w / 2, 676, "The Return of the King", 40, SILVER)
rule(d, w / 2, 748, 170, (120, 136, 165))
text_centred(d, w / 2, 768, "PC edition  ·  2003", 26, SOFT)
text_spaced(d, w / 2, 814, "CO-OP  ·  VERSUS  ·  4 PLAYERS", 19, (232, 186, 46), 3)
frame(d, w, h)
im.save(os.path.join(OUT, "capsule_600x900.png"), optimize=True)

# the wide cover: the banner (the four controllers), its old emblem covered by the blurred map and the logo in its place
b = banner.copy()
# (the blurred banner still showed the emblem: a dark plate in the map's own colour, with a little grain, instead)
base = banner.crop((0, 0, banner.width, 70)).resize((1, 1), Image.LANCZOS).getpixel((0, 0))
mid = Image.effect_noise((610, banner.height + 160), 18).convert("RGB")
mid = Image.blend(Image.new("RGB", mid.size, tuple(int(c * 0.55) for c in base)), mid, 0.05)
soft_paste(b, mid, (423, -80), 22)   # (taller than the banner: its fading edges are only at the sides)
logo_on(b, 560, banner.width / 2, banner.height * 0.47)
d = ImageDraw.Draw(b)
text_centred(d, banner.width / 2, 600, "The Return of the King", 46, SILVER)
cover(b, 920, 430).save(os.path.join(OUT, "wide_920x430.png"), optimize=True)

# the page's background: dark on the left, where Steam puts the logo; the four controllers on the right
w, h = 1920, 620
im = backdrop(w, h)
k = h / banner.height
left = banner.crop((0, 0, 448, banner.height))
right = banner.crop((banner.width - 440, 0, banner.width, banner.height))
pair = Image.new("RGB", (888, banner.height))
pair.paste(left, (0, 0))
pair.paste(right, (448, 0))
pair = pair.resize((round(888 * k), h), Image.LANCZOS)
mask = Image.new("L", pair.size, 255)
md = ImageDraw.Draw(mask)
for i in range(160):   # a soft left edge
    md.line((i, 0, i, h), fill=int(255 * i / 160))
x0 = w - pair.width - 30
im.paste(pair, (x0, 0), mask)
grad = Image.new("L", (w, h), 0)
gd = ImageDraw.Draw(grad)
for x in range(w):     # darker towards the left
    gd.line((x, 0, x, h), fill=int(150 * max(0.0, 1 - x / (w * 0.55))))
im = Image.composite(Image.new("RGB", (w, h), NIGHT), im, grad)
im.save(os.path.join(OUT, "hero_1920x620.png"), optimize=True)

# the logo over the background: transparent (Steam draws it on the left of the page's background)
e = emblem.copy()
e.thumbnail((900, 900), Image.LANCZOS)
e.save(os.path.join(OUT, "logo.png"), optimize=True)

# the icon
i = icon.copy()
i.thumbnail((256, 256), Image.LANCZOS)
i.save(os.path.join(OUT, "icon_256.png"), optimize=True)

for f in sorted(n for n in os.listdir(OUT) if n.endswith(".png")):   # (STEAM-ARTWORK.txt is there too)
    print(f, Image.open(os.path.join(OUT, f)).size)
