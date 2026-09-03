#!/usr/bin/env python
"""程序化生成 CoinFlip 全套 UI 素材（深空靛蓝 + 琥珀金风格）。

所有素材按 2x 分辨率渲染以保证清晰度，运行期由 Qt 平滑缩放。
修改任何设计参数后重新运行本脚本即可整体重新生成，保证风格统一。

用法: python scripts/generate_assets.py
"""

import math
import random
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter, ImageOps

RESOURCE_DIR = Path(__file__).resolve().parent.parent / "resource"

# ---- 色板 ------------------------------------------------------------
BG_TOP = (12, 16, 34)
BG_MID = (22, 28, 58)
BG_BOT = (10, 13, 28)

GOLD_LIGHT = (255, 216, 110)
GOLD = (245, 185, 59)
GOLD_DARK = (196, 132, 26)
GOLD_EDGE_DARK = (146, 96, 20)

SILVER_LIGHT = (244, 247, 252)
SILVER = (199, 207, 220)
SILVER_DARK = (137, 149, 170)
SILVER_EDGE_DARK = (104, 114, 134)

INK_TOP = (38, 47, 96)
INK_BOT = (18, 23, 48)


def lerp_color(c1, c2, t):
    return tuple(int(a + (b - a) * t) for a, b in zip(c1[:3], c2[:3]))


def radial_glow(size, color, max_alpha, gamma=1.4):
    """中心亮、边缘透明的径向光斑。"""
    grad = Image.radial_gradient("L")  # 0 中心 → 255 边角
    alpha = ImageOps.invert(grad).point(lambda v: int(max_alpha * (v / 255) ** gamma))
    glow = Image.new("RGBA", (256, 256), color + (0,))
    glow.putalpha(alpha)
    return glow.resize((size, size), Image.LANCZOS)


def vertical_gradient(size, stops):
    """多段垂直渐变。stops: [(pos0..1, color), ...]"""
    w, h = size
    column = Image.new("RGB", (1, h))
    px = column.load()
    for y in range(h):
        t = y / max(1, h - 1)
        for (p1, c1), (p2, c2) in zip(stops, stops[1:]):
            if p1 <= t <= p2:
                tt = (t - p1) / max(1e-6, p2 - p1)
                px[0, y] = lerp_color(c1, c2, tt)
                break
    return column.resize(size, Image.NEAREST).convert("RGBA")


def star_points(cx, cy, outer_r, inner_r, points=5, rotation=-90):
    pts = []
    for i in range(points * 2):
        r = outer_r if i % 2 == 0 else inner_r
        angle = math.radians(rotation + i * 180.0 / points)
        pts.append((cx + r * math.cos(angle), cy + r * math.sin(angle)))
    return pts


# ---- 背景 ------------------------------------------------------------
def make_background():
    w, h = 780, 1140  # 390x570 @2x
    img = vertical_gradient((w, h), [(0.0, BG_TOP), (0.5, BG_MID), (1.0, BG_BOT)]).convert("RGBA")

    # 金色主光斑（中部偏上，托住标题/棋盘视觉重心）
    img.alpha_composite(radial_glow(1100, GOLD, 26), (w // 2 - 550, 260))
    # 靛蓝冷光（左上，增加层次）
    img.alpha_composite(radial_glow(700, (99, 102, 241), 20), (-200, -260))
    # 底部微弱冷光
    img.alpha_composite(radial_glow(600, (56, 130, 246), 10), (w // 2 - 300, h - 420))

    # 星点
    rng = random.Random(20260903)
    star_layer = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    sd = ImageDraw.Draw(star_layer)
    for _ in range(130):
        x, y = rng.randint(4, w - 4), rng.randint(4, h - 4)
        r = rng.choice((1, 1, 2, 2, 3))
        a = rng.randint(28, 120)
        sd.ellipse([x - r, y - r, x + r, y + r], fill=(235, 240, 255, a))
    img.alpha_composite(star_layer)

    # 漂浮光斑（bokeh）
    bokeh = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    bd = ImageDraw.Draw(bokeh)
    for _ in range(14):
        x, y = rng.randint(0, w), rng.randint(0, h)
        r = rng.randint(18, 70)
        color = rng.choice([(GOLD, 12), ((99, 102, 241), 14), ((255, 255, 255), 8)])
        bd.ellipse([x - r, y - r, x + r, y + r], fill=color[0] + (color[1],))
    bokeh = bokeh.filter(ImageFilter.GaussianBlur(6))
    img.alpha_composite(bokeh)

    # 暗角
    vignette = Image.radial_gradient("L").resize((w, h), Image.LANCZOS)
    vignette = vignette.point(lambda v: int(max(0, v - 110) * 0.62))
    dark = Image.new("RGBA", (w, h), (4, 6, 16, 0))
    dark.putalpha(vignette)
    img.alpha_composite(dark)

    img.save(RESOURCE_DIR / "bg.png")
    print("bg.png", img.size)


# ---- 金币翻转帧 ------------------------------------------------------
def draw_coin_face(face):
    """绘制完整圆形币面（128 画布，币径 112）。"""
    size = 128
    cx = cy = 64
    r = 56
    if face == "gold":
        c_light, c_mid, c_dark = GOLD_LIGHT, GOLD, GOLD_DARK
        edge = GOLD_EDGE_DARK
    else:
        c_light, c_mid, c_dark = SILVER_LIGHT, SILVER, SILVER_DARK
        edge = SILVER_EDGE_DARK

    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # 径向渐变币面（受光点偏左上）
    ox, oy = -14, -18
    steps = 56
    for i in range(steps, 0, -1):
        t = i / steps
        ring_r = r * t
        color = lerp_color(c_light, c_dark, t ** 0.85)
        d.ellipse([cx + ox * t - ring_r, cy + oy * t - ring_r,
                   cx + ox * t + ring_r, cy + oy * t + ring_r], fill=color)

    # 裁剪成圆
    mask = Image.new("L", (size, size), 0)
    ImageDraw.Draw(mask).ellipse([cx - r, cy - r, cx + r, cy + r], fill=255)
    img.putalpha(ImageChops_multiply(img.getchannel("A"), mask))

    d = ImageDraw.Draw(img)
    # 外缘描边
    d.ellipse([cx - r, cy - r, cx + r, cy + r], outline=edge, width=4)
    # 内环（浮雕线）
    ring_r = r - 11
    d.ellipse([cx - ring_r, cy - ring_r, cx + ring_r, cy + ring_r],
              outline=lerp_color(c_dark, (0, 0, 0), 0.25), width=3)
    # 内环高光（左上弧）
    hl = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    ImageDraw.Draw(hl).arc([cx - ring_r, cy - ring_r, cx + ring_r, cy + ring_r],
                           start=150, end=290, fill=(255, 255, 255, 110), width=3)
    img.alpha_composite(hl)

    # 中心徽记：金币五角星 / 银币菱形（带浮雕）
    if face == "gold":
        emblem = star_points(cx + 2, cy + 3, 26, 11)
        d.polygon(emblem, fill=lerp_color(c_dark, (0, 0, 0), 0.3))
        emblem_main = star_points(cx, cy, 26, 11)
        d.polygon(emblem_main, fill=lerp_color(c_light, c_dark, 0.45))
        d.polygon(star_points(cx - 1, cy - 1, 26, 11), outline=(255, 255, 255, 90))
    else:
        diamond = [(cx, cy - 28), (cx + 21, cy + 2), (cx, cy + 30), (cx - 21, cy + 2)]
        d.polygon([(x + 2, y + 3) for x, y in diamond], fill=lerp_color(c_dark, (0, 0, 0), 0.3))
        d.polygon(diamond, fill=lerp_color(c_light, c_dark, 0.4))
        d.line(diamond + [diamond[0]], fill=(255, 255, 255, 90))

    # 顶部高光
    gloss = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    ImageDraw.Draw(gloss).ellipse([cx - r * 0.62, cy - r * 0.82,
                                   cx + r * 0.62, cy - r * 0.10], fill=(255, 255, 255, 55))
    gloss = gloss.filter(ImageFilter.GaussianBlur(7))
    img.alpha_composite(gloss)
    return img


def ImageChops_multiply(a, b):
    from PIL import ImageChops
    return ImageChops.multiply(a, b)


def draw_coin_edge(face, height_ratio=1.0):
    """硬币侧缘（近侧面视角的薄片）。"""
    size = 128
    cx = 64
    r = 56
    band_w = 14
    top = cy_top = 64 - r
    bottom = 64 + r
    if face == "gold":
        c_light, c_dark = GOLD, GOLD_EDGE_DARK
    else:
        c_light, c_dark = SILVER, SILVER_EDGE_DARK

    img = vertical_gradient((band_w, int(2 * r)), [(0.0, c_light), (1.0, c_dark)])
    mask = Image.new("L", img.size, 0)
    ImageDraw.Draw(mask).rounded_rectangle([0, 0, band_w - 1, 2 * r - 1], radius=band_w // 2, fill=255)
    img.putalpha(mask)

    canvas = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    canvas.paste(img, (cx - band_w // 2, 64 - r))
    return canvas


def make_coin_frames():
    """8 帧翻转动画：正面(金) → 侧面 → 背面(银)。输出 56x56 逻辑尺寸。"""
    for f in range(1, 9):
        theta = math.pi * (f - 1) / 7
        s = math.cos(theta)
        face = "gold" if f <= 4 else "silver"

        canvas = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
        # 投影
        shadow = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
        ImageDraw.Draw(shadow).ellipse([24, 104, 104, 122], fill=(0, 0, 0, 80))
        shadow = shadow.filter(ImageFilter.GaussianBlur(5))
        canvas.alpha_composite(shadow)

        if abs(s) < 0.22:
            body = draw_coin_edge(face)
        else:
            body = draw_coin_face(face)
            w = max(4, int(128 * abs(s)))
            body = body.resize((w, 128), Image.LANCZOS)
        canvas.alpha_composite(body, ((128 - body.width) // 2, 0))

        canvas.resize((56, 56), Image.LANCZOS).save(RESOURCE_DIR / f"coin_{f}.png")
    print("coin_1..8.png")


# ---- 开始按钮 --------------------------------------------------------
def make_start_button():
    size = 220
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))

    # 投影
    shadow = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    ImageDraw.Draw(shadow).ellipse([28, 44, 192, 200], fill=(0, 0, 0, 110))
    shadow = shadow.filter(ImageFilter.GaussianBlur(10))
    img.alpha_composite(shadow)

    cx = cy = 110
    r = 88
    # 外环金色渐变（同心环）
    steps = 40
    for i in range(steps, 0, -1):
        t = i / steps
        ring_r = r * t
        color = lerp_color(GOLD_LIGHT, GOLD_DARK, t ** 1.3)
        ImageDraw.Draw(img).ellipse([cx - ring_r, cy - ring_r, cx + ring_r, cy + ring_r], fill=color)

    # 内盘靛蓝渐变
    inner = vertical_gradient((2 * (r - 16), 2 * (r - 16)), [(0.0, INK_TOP), (1.0, INK_BOT)])
    mask = Image.new("L", inner.size, 0)
    ImageDraw.Draw(mask).ellipse([0, 0, inner.width - 1, inner.height - 1], fill=255)
    inner.putalpha(mask)
    img.alpha_composite(inner, (cx - inner.width // 2, cy - inner.height // 2))

    d = ImageDraw.Draw(img)
    d.ellipse([cx - r, cy - r, cx + r, cy + r], outline=GOLD_DARK, width=3)

    # 播放三角（带辉光）
    tri = [(cx - 18, cy - 30), (cx - 18, cy + 30), (cx + 34, cy)]
    glow = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    ImageDraw.Draw(glow).polygon([(x + 2, y + 3) for x, y in tri], fill=(0, 0, 0, 120))
    img.alpha_composite(glow)
    d = ImageDraw.Draw(img)
    d.polygon(tri, fill=GOLD)
    d.line(tri + [tri[0]], fill=GOLD_LIGHT, width=2, joint="curve")

    # 顶部高光弧
    gloss = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    ImageDraw.Draw(gloss).ellipse([cx - 62, cy - 76, cx + 62, cy + 6], fill=(255, 255, 255, 40))
    gloss = gloss.filter(ImageFilter.GaussianBlur(10))
    img.alpha_composite(gloss)

    img.resize((110, 110), Image.LANCZOS).save(RESOURCE_DIR / "btn_start.png")
    print("btn_start.png")


# ---- 返回按钮 --------------------------------------------------------
def make_back_button():
    size = 120
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    cx = cy = 60
    r = 52

    # 投影
    shadow = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    ImageDraw.Draw(shadow).ellipse([16, 24, 104, 108], fill=(0, 0, 0, 90))
    shadow = shadow.filter(ImageFilter.GaussianBlur(6))
    img.alpha_composite(shadow)

    # 玻璃圆盘
    glass = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    ImageDraw.Draw(glass).ellipse([cx - r, cy - r, cx + r, cy + r], fill=(255, 255, 255, 34))
    img.alpha_composite(glass)

    d = ImageDraw.Draw(img)
    d.ellipse([cx - r, cy - r, cx + r, cy + r], outline=(255, 255, 255, 95), width=3)

    # V 形箭头（带描影）
    chevron = [(cx + 12, cy - 22), (cx - 14, cy), (cx + 12, cy + 22)]
    under = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    ImageDraw.Draw(under).line([(x, y + 3) for x, y in chevron], fill=(0, 0, 0, 110),
                               width=10, joint="curve")
    img.alpha_composite(under)
    d = ImageDraw.Draw(img)
    d.line(chevron, fill=(240, 244, 255, 240), width=10, joint="curve")
    for px, py in (chevron[0], chevron[2]):
        d.ellipse([px - 5, py - 5, px + 5, py + 5], fill=(240, 244, 255, 240))

    img.resize((56, 56), Image.LANCZOS).save(RESOURCE_DIR / "btn_back.png")
    print("btn_back.png")


# ---- 关卡徽章 --------------------------------------------------------
def make_level_badge():
    size = 116
    radius = 28
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))

    # 投影
    shadow = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    ImageDraw.Draw(shadow).rounded_rectangle([10, 18, 106, 110], radius=radius, fill=(0, 0, 0, 100))
    shadow = shadow.filter(ImageFilter.GaussianBlur(7))
    img.alpha_composite(shadow)

    # 深色玻璃面板：靛蓝底 + 顶部白色微光
    mask = Image.new("L", (size, size), 0)
    ImageDraw.Draw(mask).rounded_rectangle([4, 4, size - 4, size - 4], radius=radius, fill=255)

    base = Image.new("RGBA", (size, size), (20, 26, 54, 228))
    base.putalpha(ImageChops_multiply(base.getchannel("A"), mask))
    img.alpha_composite(base)

    sheen = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    ImageDraw.Draw(sheen).rounded_rectangle([8, 6, size - 8, size // 2 + 6], radius=radius // 2,
                                            fill=(255, 255, 255, 30))
    sheen = sheen.filter(ImageFilter.GaussianBlur(5))
    sheen.putalpha(ImageChops_multiply(sheen.getchannel("A"), mask))
    img.alpha_composite(sheen)

    d = ImageDraw.Draw(img)
    d.rounded_rectangle([4, 4, size - 4, size - 4], radius=radius,
                        outline=(255, 255, 255, 90), width=3)
    # 底部金色饰条
    d.rounded_rectangle([size // 2 - 17, size - 22, size // 2 + 17, size - 16],
                        radius=3, fill=(GOLD + (170,)))

    img.resize((58, 58), Image.LANCZOS).save(RESOURCE_DIR / "badge_level.png")
    print("badge_level.png")


# ---- 应用图标 --------------------------------------------------------
def make_app_icon():
    face = draw_coin_face("gold").resize((256, 256), Image.LANCZOS)
    face.save(RESOURCE_DIR / "app.ico",
              sizes=[(16, 16), (24, 24), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)])
    print("app.ico")


if __name__ == "__main__":
    RESOURCE_DIR.mkdir(exist_ok=True)
    make_background()
    make_coin_frames()
    make_start_button()
    make_back_button()
    make_level_badge()
    make_app_icon()
    print("全部素材已生成 ->", RESOURCE_DIR)
