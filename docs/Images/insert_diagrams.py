#!/usr/bin/env python3
"""
NEON DRIFT — PDF Diagram Inserter
기획서·설계서 PDF에 클래스 다이어그램 및 플로우차트 페이지를 삽입한다.
pymupdf(fitz) 사용.
"""

import fitz
from pathlib import Path

# ── 경로 ───────────────────────────────────────────────────────────────
DOCS  = Path(r"C:\Users\user\Documents\Unreal Projects\NEONDRIFT\docs")
IMGS  = DOCS / "Images"

IMG_CLASS    = IMGS / "class_diagram_preview.png"
IMG_LOOP     = IMGS / "flowchart_game_loop_preview.png"
IMG_RESOURCE = IMGS / "flowchart_resource_preview.png"
IMG_COMBAT   = IMGS / "flowchart_combat_preview.png"

SRC_PLANNING = DOCS / "NEONDRIFT_기획서.pdf"
SRC_DESIGN   = DOCS / "NEONDRIFT_설계서.pdf"
OUT_PLANNING = DOCS / "NEONDRIFT_기획서_updated.pdf"
OUT_DESIGN   = DOCS / "NEONDRIFT_설계서_updated.pdf"

# ── 색상 (0-1 float) ───────────────────────────────────────────────────
BG   = (0.051, 0.051, 0.102)   # #0d0d1a
CYAN = (0.000, 0.941, 1.000)   # #00f0ff
MAG  = (1.000, 0.176, 0.816)   # #ff2fd0
GRY  = (0.400, 0.467, 0.667)   # #6677aa
WHT  = (0.847, 0.855, 0.941)   # #d8daf0

PAGE_W, PAGE_H = 612, 792
MARGIN = 36


def _img_size(img_path: Path):
    """이미지의 자연 크기(pt)를 반환."""
    tmp = fitz.open(str(img_path))
    r = tmp[0].rect
    tmp.close()
    return r.width, r.height


def insert_diagram_page(doc: fitz.Document, after_index: int,
                         img_path: Path, caption: str, section_label: str):
    """
    doc의 after_index 페이지 뒤에 다이어그램 페이지를 삽입한다.
    반환값: 삽입된 페이지 인덱스.
    """
    insert_at = after_index + 1          # fitz.new_page는 이 위치 앞에 삽입

    # ── 새 페이지 생성 ──────────────────────────────────────────────────
    page = doc.new_page(pno=insert_at, width=PAGE_W, height=PAGE_H)

    # 배경
    page.draw_rect(
        fitz.Rect(0, 0, PAGE_W, PAGE_H),
        color=BG, fill=BG
    )

    # 상단 장식선
    page.draw_line(
        fitz.Point(MARGIN, MARGIN - 8),
        fitz.Point(PAGE_W - MARGIN, MARGIN - 8),
        color=CYAN, width=1.5
    )
    # 좌측 포인트 강조
    page.draw_line(
        fitz.Point(MARGIN, MARGIN - 8),
        fitz.Point(MARGIN + 4, MARGIN - 8),
        color=MAG, width=3
    )

    # 섹션 레이블 (좌상단 소문자)
    page.insert_text(
        fitz.Point(MARGIN, MARGIN + 8),
        section_label,
        fontsize=9, color=GRY
    )

    # ── 이미지 배치 ────────────────────────────────────────────────────
    img_w, img_h = _img_size(img_path)

    avail_w = PAGE_W - 2 * MARGIN
    avail_h = PAGE_H - 2 * MARGIN - 52   # 캡션 영역 확보

    scale = min(avail_w / img_w, avail_h / img_h)
    draw_w = img_w * scale
    draw_h = img_h * scale

    x0 = MARGIN + (avail_w - draw_w) / 2
    y0 = MARGIN + 20

    # 이미지 테두리 (미세 네온 glow 효과 흉내)
    border_rect = fitz.Rect(x0 - 2, y0 - 2, x0 + draw_w + 2, y0 + draw_h + 2)
    page.draw_rect(border_rect, color=CYAN, width=0.8)

    page.insert_image(
        fitz.Rect(x0, y0, x0 + draw_w, y0 + draw_h),
        filename=str(img_path)
    )

    # ── 캡션 ──────────────────────────────────────────────────────────
    cap_y = y0 + draw_h + 20
    cap_rect = fitz.Rect(MARGIN, cap_y - 14, PAGE_W - MARGIN, cap_y + 4)
    page.insert_textbox(
        cap_rect, caption,
        fontsize=11, color=CYAN,
        align=fitz.TEXT_ALIGN_CENTER
    )

    # 하단 장식선
    page.draw_line(
        fitz.Point(MARGIN, PAGE_H - MARGIN + 8),
        fitz.Point(PAGE_W - MARGIN, PAGE_H - MARGIN + 8),
        color=CYAN, width=0.8
    )
    page.draw_line(
        fitz.Point(PAGE_W - MARGIN - 4, PAGE_H - MARGIN + 8),
        fitz.Point(PAGE_W - MARGIN, PAGE_H - MARGIN + 8),
        color=MAG, width=3
    )

    return insert_at


# ══════════════════════════════════════════════════════════════════════
# 기획서 수정
#   p08 (idx 7) : 3-1. 게임 루프 상태 머신  → 뒤에 게임루프 플로우차트 삽입
#   p12 (idx 11): 5-2. 클래스 구조          → 뒤에 클래스 다이어그램 삽입
#   p08 전후 (idx 7 리소스/전투 내용)       → 뒤에 리소스·전투 플로우차트 삽입
# ══════════════════════════════════════════════════════════════════════
print("▶ 기획서 수정 중 …")
doc_p = fitz.open(str(SRC_PLANNING))

# 삽입 순서: 뒤 페이지부터 역순으로 진행해야 앞 삽입이 인덱스를 밀지 않는다

# ① p12 (idx 11) 뒤 → 클래스 다이어그램
insert_diagram_page(
    doc_p, after_index=11,
    img_path=IMG_CLASS,
    caption="▲ 클래스 다이어그램 — 전체 아키텍처 구조",
    section_label="// 5-2. 클래스 구조"
)

# ② p08 (idx 7) 뒤 → 게임루프 플로우차트
#    (①에서 8번 이후가 밀렸으므로 idx 7은 그대로)
insert_diagram_page(
    doc_p, after_index=7,
    img_path=IMG_LOOP,
    caption="▲ 게임 루프 상태머신 플로우차트",
    section_label="// 3-1. 게임 루프 상태머신"
)

doc_p.save(str(OUT_PLANNING), garbage=4, deflate=True)
doc_p.close()
print(f"  저장 → {OUT_PLANNING.name}  ({OUT_PLANNING.stat().st_size // 1024} KB)")


# ══════════════════════════════════════════════════════════════════════
# 설계서 수정
#   p02 (idx 1) : Context        → 뒤에 클래스 다이어그램 삽입
#   p05 (idx 4) : AResourceBlock → 뒤에 자원 채집 플로우차트 삽입
#   p05+1=6     : after resource  → 뒤에 전투 플로우차트 삽입
# ══════════════════════════════════════════════════════════════════════
print("▶ 설계서 수정 중 …")
doc_d = fitz.open(str(SRC_DESIGN))

# 역순 삽입
# ① p05 (idx 4) 뒤 → 자원 채집 플로우차트 + 전투 플로우차트 (두 장)
insert_diagram_page(
    doc_d, after_index=4,
    img_path=IMG_COMBAT,
    caption="▲ 전투 & 피해 처리 흐름",
    section_label="// AMonster · ABase · IDamageable"
)
insert_diagram_page(
    doc_d, after_index=4,
    img_path=IMG_RESOURCE,
    caption="▲ 자원 채집 흐름 — AResourceBlock → AResourceShard → APlayerShip",
    section_label="// AResourceBlock · AResourceShard · APlayerShip"
)

# ② p02 (idx 1) 뒤 → 클래스 다이어그램 (역순이므로 인덱스 변화 없음)
insert_diagram_page(
    doc_d, after_index=1,
    img_path=IMG_CLASS,
    caption="▲ 클래스 다이어그램 — 전체 아키텍처 구조",
    section_label="// 전체 클래스 개요"
)

doc_d.save(str(OUT_DESIGN), garbage=4, deflate=True)
doc_d.close()
print(f"  저장 → {OUT_DESIGN.name}  ({OUT_DESIGN.stat().st_size // 1024} KB)")

print("\n✓ 완료. _updated.pdf 파일을 확인해 주세요.")
