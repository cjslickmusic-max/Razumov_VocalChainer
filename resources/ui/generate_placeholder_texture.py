#!/usr/bin/env python3
"""Generate brushed_panel_placeholder.png (512x256 RGBA) without third-party deps."""

import os
import struct
import zlib


def write_png_rgba(path: str, width: int, height: int, pixels: bytes) -> None:
    if len(pixels) != width * height * 4:
        raise ValueError("pixel buffer size mismatch")

    def chunk(tag: bytes, data: bytes) -> bytes:
        return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)

    raw_rows = b""
    for y in range(height):
        row = bytes([0])  # filter 0
        i = y * width * 4
        row += pixels[i : i + width * 4]
        raw_rows += row

    compressed = zlib.compress(raw_rows, 9)
    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(
        b"IHDR",
        struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0),
    )
    png += chunk(b"IDAT", compressed)
    png += chunk(b"IEND", b"")
    with open(path, "wb") as f:
        f.write(png)


def main() -> None:
    w, h = 512, 256
    px = bytearray(w * h * 4)
    for y in range(h):
        for x in range(w):
            # Brushed metal-ish noise + vignette
            n = ((x * 7919 + y * 6151) ^ (x * y * 13 + 0x4A7F)) & 0xFF
            base = 32 + (n % 28)
            vx = 1.0 - abs(x - w / 2) / (w / 2) * 0.22
            vy = 1.0 - abs(y - h / 2) / (h / 2) * 0.18
            g = int(base * vx * vy)
            g = max(18, min(92, g))
            streak = ((x + y * 3) % 37) - 18
            g2 = max(12, min(100, g + streak // 6))
            i = (y * w + x) * 4
            px[i] = g2
            px[i + 1] = g2 + 2
            px[i + 2] = g2 + 5
            px[i + 3] = 255

    out = os.path.join(os.path.dirname(os.path.abspath(__file__)), "brushed_panel_placeholder.png")
    write_png_rgba(out, w, h, bytes(px))
    print("Wrote", out, w, "x", h)


if __name__ == "__main__":
    main()
