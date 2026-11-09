# Python 3 + Pillow: the pictures for Steam's library, for ROTKPlus added as a non-Steam game (on a PC or a Steam
# Machine / Steam Deck). From the project's own art (launcher/art: the banner with the four controllers, the emblem,
# the icon) and the Cinzel font; nothing of the game. The game's name is only written, as text.
# Output in launcher/art/steam:
#   capsule_600x900.png   the library's cover (portrait)
#   wide_920x430.png      the wide cover (recent games, Big Picture)
#   hero_1920x620.png     the background at the top of the game's page
#   logo.png              the emblem, transparent, over the background
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
emblem = Image.open(os.path.join(ART, "rotk_cover.png")).convert("RGBA")          # the CO-OP VERSUS emblem
icon = Image.open(os.path.join(ART, "icon_rotkplus_1024.png")).convert("RGBA")


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

# the cover, portrait: the emblem large, the game's name and what ROTKPlus adds under it
w, h = 600, 900
im = vignette(backdrop(w, h))
e = emblem.copy()
e.thumbnail((520, 520), Image.LANCZOS)
im.paste(e, ((w - e.width) // 2, 70), e)
d = ImageDraw.Draw(im)
text_spaced(d, w / 2, 612, "THE LORD OF THE RINGS", 22, SOFT, 5)
text_centred(d, w / 2, 650, "The Return of the King", 40, SILVER)
rule(d, w / 2, 724, 170, (120, 136, 165))
text_centred(d, w / 2, 748, "PC edition with ROTKPlus", 26, SOFT)
text_spaced(d, w / 2, 800, "CO-OP  ·  VERSUS  ·  4 PLAYERS", 19, (232, 186, 46), 3)
frame(d, w, h)
im.save(os.path.join(OUT, "capsule_600x900.png"), optimize=True)

# the wide cover: the banner itself (the four controllers around the emblem)
cover(banner, 920, 430).save(os.path.join(OUT, "wide_920x430.png"), optimize=True)

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

# the logo over the background: the emblem, transparent
e = emblem.copy()
e.thumbnail((640, 640), Image.LANCZOS)
e.save(os.path.join(OUT, "logo.png"), optimize=True)

# the icon
i = icon.copy()
i.thumbnail((256, 256), Image.LANCZOS)
i.save(os.path.join(OUT, "icon_256.png"), optimize=True)

for f in sorted(os.listdir(OUT)):
    print(f, Image.open(os.path.join(OUT, f)).size)
