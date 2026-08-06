#!/usr/bin/env python3
"""
Generate a 5x7 column-based Hebrew bitmap font from a real system font.
Renders at large size, then carefully downsamples to 5x7 pixel grid.
"""
from PIL import Image, ImageDraw, ImageFont
import sys

HEBREW_LETTERS = [
    ('\u05D0', 'Alef',       'א', 0x90),
    ('\u05D1', 'Bet',        'ב', 0x91),
    ('\u05D2', 'Gimel',      'ג', 0x92),
    ('\u05D3', 'Dalet',      'ד', 0x93),
    ('\u05D4', 'He',         'ה', 0x94),
    ('\u05D5', 'Vav',        'ו', 0x95),
    ('\u05D6', 'Zayin',      'ז', 0x96),
    ('\u05D7', 'Het',        'ח', 0x97),
    ('\u05D8', 'Tet',        'ט', 0x98),
    ('\u05D9', 'Yod',        'י', 0x99),
    ('\u05DA', 'Final Kaf',  'ך', 0x9A),
    ('\u05DB', 'Kaf',        'כ', 0x9B),
    ('\u05DC', 'Lamed',      'ל', 0x9C),
    ('\u05DD', 'Final Mem',  'ם', 0x9D),
    ('\u05DE', 'Mem',        'מ', 0x9E),
    ('\u05DF', 'Final Nun',  'ן', 0x9F),
    ('\u05E0', 'Nun',        'נ', 0xA0),
    ('\u05E1', 'Samekh',     'ס', 0xA1),
    ('\u05E2', 'Ayin',       'ע', 0xA2),
    ('\u05E3', 'Final Pe',   'ף', 0xA3),
    ('\u05E4', 'Pe',         'פ', 0xA4),
    ('\u05E5', 'Final Tsadi','ץ', 0xA5),
    ('\u05E6', 'Tsadi',      'צ', 0xA6),
    ('\u05E7', 'Qof',        'ק', 0xA7),
    ('\u05E8', 'Resh',       'ר', 0xA8),
    ('\u05E9', 'Shin',       'ש', 0xA9),
    ('\u05EA', 'Tav',        'ת', 0xAA),
]

def render_char(char, font, target_w=5, target_h=7):
    """Render a character to target_w x target_h bitmap using large intermediate."""
    # Render at a large size
    big_size = 120
    img = Image.new('L', (big_size, big_size), 255)
    draw = ImageDraw.Draw(img)
    
    # Draw the character centered
    bbox = font.getbbox(char)
    if bbox is None:
        return [[0]*target_w for _ in range(target_h)]
    
    tw = bbox[2] - bbox[0]
    th = bbox[3] - bbox[1]
    x = (big_size - tw) // 2 - bbox[0]
    y = (big_size - th) // 2 - bbox[1]
    draw.text((x, y), char, fill=0, font=font)
    
    # Find the actual ink bounding box
    ink_bbox = None
    pixels = img.load()
    min_x, min_y, max_x, max_y = big_size, big_size, 0, 0
    for py in range(big_size):
        for px in range(big_size):
            if pixels[px, py] < 128:
                min_x = min(min_x, px)
                min_y = min(min_y, py)
                max_x = max(max_x, px)
                max_y = max(max_y, py)
    
    if max_x <= min_x or max_y <= min_y:
        return [[0]*target_w for _ in range(target_h)]
    
    # Add 1 pixel padding
    min_x = max(0, min_x - 1)
    min_y = max(0, min_y - 1)
    max_x = min(big_size - 1, max_x + 1)
    max_y = min(big_size - 1, max_y + 1)
    
    # Crop to ink bounds
    cropped = img.crop((min_x, min_y, max_x + 1, max_y + 1))
    
    # Resize to target dimensions
    resized = cropped.resize((target_w, target_h), Image.LANCZOS)
    
    # Convert to binary with threshold
    result = []
    for py in range(target_h):
        row = []
        for px in range(target_w):
            val = resized.getpixel((px, py))
            row.append(1 if val < 140 else 0)
        result.append(row)
    
    return result

def pixels_to_columns(pixels, w=5, h=7):
    """Convert pixel grid to column bytes (LSB=row0)."""
    columns = []
    for col in range(w):
        byte_val = 0
        for row in range(min(h, 8)):
            if pixels[row][col]:
                byte_val |= (1 << row)
        columns.append(byte_val)
    return columns

def print_grid(pixels, w=5, h=7):
    """Print visual grid."""
    for row in range(h):
        line = ""
        for col in range(w):
            line += "██" if pixels[row][col] else "  "
        print(f"    // {line}")

# Try fonts in order of preference
font_candidates = [
    ("/System/Library/Fonts/ArialHB.ttc", 80),
    ("/System/Library/Fonts/Supplemental/Arial Hebrew.ttc", 80),
    ("/System/Library/Fonts/Supplemental/Arial Hebrew Bold.ttc", 80),
    ("/Library/Fonts/Arial Hebrew.ttf", 80),
    ("/System/Library/Fonts/Helvetica.ttc", 80),
    ("/System/Library/Fonts/SFCompact.ttf", 80),
]

font = None
font_path_used = None
for fp, sz in font_candidates:
    try:
        font = ImageFont.truetype(fp, sz)
        # Test if it can actually render Hebrew
        test_img = Image.new('L', (100, 100), 255)
        test_draw = ImageDraw.Draw(test_img)
        test_draw.text((10, 10), '\u05D0', fill=0, font=font)
        # Check if any pixels were drawn
        found = False
        for py in range(100):
            for px in range(100):
                if test_img.getpixel((px, py)) < 128:
                    found = True
                    break
            if found:
                break
        if found:
            font_path_used = fp
            break
    except (IOError, OSError):
        continue

if font is None:
    print("ERROR: No Hebrew-capable font found!", file=sys.stderr)
    sys.exit(1)

print(f"// Generated from font: {font_path_used}")
print(f"// Each glyph visual grid shown above its data")
print()
print("const uint8_t PROGMEM hebrew_font_5x7[27][5] = {")

for i, (char, name, display, code) in enumerate(HEBREW_LETTERS):
    pixels = render_char(char, font)
    cols = pixels_to_columns(pixels)
    hex_str = ", ".join(f"0x{c:02x}" for c in cols)
    comma = "," if i < len(HEBREW_LETTERS) - 1 else ""
    
    print(f"    // 0x{code:02X}: {name} ({display})")
    print_grid(pixels)
    print(f"    {{ {hex_str} }}{comma}")

print("};")
