#!/usr/bin/env python3
"""NEON DRIFT — Flowchart Generator (3 charts)"""

import os

BASE = r"C:\Users\user\Documents\Unreal Projects\NEONDRIFT\docs\Images"

# ── colour palette ──────────────────────────────────────────
BG    = "#0d0d1a"
CYAN  = "#00f0ff"
MAG   = "#ff2fd0"
ORG   = "#ff8a2f"
GRN   = "#2fff8a"
WHITE = "#d8daf0"
GRAY  = "#6677aa"
CARD  = "#0e0e22"
CDIM  = "#002830"
MDIM  = "#28001a"
GDIM  = "#002818"
ODIM  = "#281000"
BORDER= "#1e1e44"

# ── SVG builder ─────────────────────────────────────────────
class SVG:
    def __init__(self, w, h, title):
        self.w = w; self.h = h; self.title = title
        self.p = []
        self._header()

    def _header(self):
        self.p.append(f'<svg xmlns="http://www.w3.org/2000/svg" '
                      f'width="{self.w}" height="{self.h}" viewBox="0 0 {self.w} {self.h}">')
        self.p.append('<defs>')
        for cname, cv in [("cyan",CYAN),("mag",MAG),("org",ORG),("grn",GRN),("gray",GRAY)]:
            self.p.append(f'<marker id="ah_{cname}" markerWidth="9" markerHeight="7" '
                          f'refX="8" refY="3.5" orient="auto">'
                          f'<polygon points="0 0,9 3.5,0 7" fill="{cv}"/></marker>')
        self.p.append('</defs>')
        self.p.append(f'<rect width="{self.w}" height="{self.h}" fill="{BG}"/>')
        # grid
        for x in range(0, self.w, 50):
            self.p.append(f'<line x1="{x}" y1="0" x2="{x}" y2="{self.h}" '
                          f'stroke="#fff" stroke-width="0.12" opacity="0.07"/>')
        for y in range(0, self.h, 50):
            self.p.append(f'<line x1="0" y1="{y}" x2="{self.w}" y2="{y}" '
                          f'stroke="#fff" stroke-width="0.12" opacity="0.07"/>')
        # title
        self.p.append(f'<text x="{self.w//2}" y="34" font-family="\'Courier New\',monospace" '
                      f'font-size="17" font-weight="bold" fill="{CYAN}" text-anchor="middle">'
                      f'{self.title}</text>')
        self.p.append(f'<line x1="40" y1="44" x2="{self.w-40}" y2="44" '
                      f'stroke="{BORDER}" stroke-width="1"/>')

    def e(self, s): self.p.append(s)

    def txt(self, x, y, s, size=12, fill=WHITE, anchor="middle", bold=False, italic=False):
        fw = "bold" if bold else "normal"
        fs = "italic" if italic else "normal"
        s2 = s.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;")
        self.p.append(f'<text x="{x}" y="{y}" font-family="\'Courier New\',monospace" '
                      f'font-size="{size}" font-weight="{fw}" font-style="{fs}" '
                      f'fill="{fill}" text-anchor="{anchor}">{s2}</text>')

    # ── shapes ──────────────────────────────────────────────

    def terminator(self, x, y, w, h, label, color=CYAN):
        """Pill-shaped start/end node."""
        bg = CDIM if color == CYAN else MDIM if color == MAG else GDIM
        self.e(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" '
               f'fill="{bg}" stroke="{color}" stroke-width="2" rx="{h//2}"/>')
        self.txt(x+w//2, y+h//2+5, label, size=13, fill=color, bold=True)
        return x+w//2, y, y+h  # cx, top, bot

    def state(self, x, y, w, label, sub=None, color=CYAN):
        """Process / state box."""
        hdr = 36 if not sub else 38
        body = 0 if not sub else (len(sub)*15 + 10)
        h = hdr + body
        bg = CDIM if color==CYAN else MDIM if color==MAG else GDIM if color==GRN else ODIM
        self.e(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" '
               f'fill="{bg}" stroke="{color}" stroke-width="1.8" rx="5"/>')
        if sub:
            self.e(f'<rect x="{x}" y="{y}" width="{w}" height="{hdr}" fill="{bg}" rx="5"/>')
            self.e(f'<rect x="{x}" y="{y+hdr-4}" width="{w}" height="6" fill="{bg}" stroke="none"/>')
            self.e(f'<line x1="{x}" y1="{y+hdr}" x2="{x+w}" y2="{y+hdr}" '
                   f'stroke="{color}" stroke-width="1"/>')
        ty = y + (hdr//2) + 6
        self.txt(x+w//2, ty, label, size=13, fill=color, bold=True)
        if sub:
            for i, line in enumerate(sub):
                self.txt(x+10, y+hdr+14+i*15, line, size=10, fill=GRAY, anchor="start")
        return x+w//2, y, y+h, x, x+w   # cx, top, bot, lx, rx

    def diamond(self, cx, y, w, h, label, color=CYAN):
        """Decision diamond."""
        pts = f"{cx},{y} {cx+w//2},{y+h//2} {cx},{y+h} {cx-w//2},{y+h//2}"
        bg = CDIM if color==CYAN else MDIM if color==MAG else "#181008"
        self.e(f'<polygon points="{pts}" fill="{bg}" stroke="{color}" stroke-width="1.8"/>')
        self.txt(cx, y+h//2+5, label, size=11, fill=color, bold=True)
        # returns: top_mid, bot_mid, left_mid, right_mid
        return (cx,y), (cx,y+h), (cx-w//2,y+h//2), (cx+w//2,y+h//2)

    def arrow(self, x1,y1,x2,y2, color=CYAN, label=None, label_side="right", dashed=False):
        """Arrow between two points."""
        cid = ("cyan" if color==CYAN else "mag" if color==MAG
               else "org" if color==ORG else "grn" if color==GRN else "gray")
        da = 'stroke-dasharray="6,3"' if dashed else ""
        self.e(f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" '
               f'stroke="{color}" stroke-width="1.6" {da} marker-end="url(#ah_{cid})"/>')
        if label:
            mx = (x1+x2)//2; my = (y1+y2)//2
            ox = 10 if label_side=="right" else -10
            anch = "start" if label_side=="right" else "end"
            self.txt(mx+ox, my+4, label, size=10, fill=color, anchor=anch)

    def polyline(self, pts, color=CYAN, label=None, dashed=False):
        """Multi-segment arrow (last segment gets arrowhead)."""
        cid = ("cyan" if color==CYAN else "mag" if color==MAG
               else "org" if color==ORG else "grn" if color==GRN else "gray")
        da = 'stroke-dasharray="6,3"' if dashed else ""
        coords = " ".join(f"{x},{y}" for x,y in pts)
        # draw segments
        for i in range(len(pts)-2):
            x1,y1 = pts[i]; x2,y2 = pts[i+1]
            self.e(f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" '
                   f'stroke="{color}" stroke-width="1.6" {da}/>')
        # last segment gets marker
        x1,y1 = pts[-2]; x2,y2 = pts[-1]
        self.e(f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" '
               f'stroke="{color}" stroke-width="1.6" {da} marker-end="url(#ah_{cid})"/>')
        if label:
            mx = (pts[-2][0]+pts[-1][0])//2
            my = (pts[-2][1]+pts[-1][1])//2
            self.txt(mx+8, my+4, label, size=10, fill=color, anchor="start")

    def label_box(self, x, y, text, color=GRAY):
        """Small inline label on an arrow."""
        w = len(text)*7 + 12
        self.e(f'<rect x="{x-w//2}" y="{y-9}" width="{w}" height="16" '
               f'fill="{CARD}" stroke="{color}" stroke-width="0.8" rx="3"/>')
        self.txt(x, y+3, text, size=10, fill=color)

    def save(self, filename):
        self.p.append('</svg>')
        path = os.path.join(BASE, filename)
        with open(path, "w", encoding="utf-8") as f:
            f.write("\n".join(self.p))
        print(f"Saved → {path}")


# ═══════════════════════════════════════════════════════════
# FLOWCHART 1 — Game Loop State Machine
# ═══════════════════════════════════════════════════════════
def flow_game_loop():
    sv = SVG(900, 1060, "NEON DRIFT — Game Loop State Machine")
    CX = 420   # main column center
    W  = 320   # state box width
    LX = CX - W//2

    # ── nodes ──────────────────────────────────────────────
    # MainMenu
    cx,mt,mb = sv.terminator(LX, 60, W, 46, "MAIN MENU", color=MAG)

    # PreWave
    _,pt,pb,_,_ = sv.state(LX, 135, W, "PRE WAVE",
        sub=["기지·포탑·스폰포인트 레퍼런스 수집", "자원 필드 스폰 (블록 40개)", "GatherTimer = 60s 설정"],
        color=CYAN)

    # Gather
    _,gt,gb,_,_ = sv.state(LX, 260, W, "GATHER",
        sub=["호버바이크 자유 비행", "자원 블록 파괴 → 자석 채집", "60초 카운트다운"],
        color=GRN)

    # Combat
    _,ct,cb,clx,crx = sv.state(LX, 390, W, "COMBAT",
        sub=["웨이브 몬스터 스폰 펌프", "몬스터 기지 공격 & 플레이어 방어", "OnMonsterKilled() → AliveMonsters 감소"],
        color=MAG)

    # Decision: 기지 파괴?
    dt, db, dl, dr = sv.diamond(CX, 530, 260, 60, "기지 파괴?", color=ORG)

    # GameOver (right branch)
    gox = crx + 80
    _,got,gob = sv.terminator(gox-70, 523, 160, 46, "GAME OVER", color=ORG)

    # (no branch) → Shop
    _,st,sb,_,_ = sv.state(LX, 630, W, "SHOP",
        sub=["자원으로 8종 강화 구매", "↑↓ 탐색 / Enter 구매", "다음 웨이브 진입 여부 결정"],
        color=CYAN)

    # Decision: 마지막 웨이브?
    vdt, vdb, vdl, vdr = sv.diamond(CX, 770, 280, 60, "WaveIndex == 5?", color=CYAN)

    # Victory (right branch)
    vix = crx + 80
    _,vit,vib = sv.terminator(vix-70, 763, 160, 46, "VICTORY!", color=GRN)

    # ── arrows (main flow) ─────────────────────────────────
    sv.arrow(CX, mb, CX, pt, color=MAG, label="StartGame()")
    sv.arrow(CX, pb, CX, gt, label="F키 준비 완료")
    sv.arrow(CX, gb, CX, ct, color=GRN, label="타이머 만료")
    sv.arrow(CX, cb, CX, dt[1], color=MAG)

    # 기지 파괴? No → Shop
    sv.arrow(CX, db[1], CX, st, color=CYAN, label="No  (적 전멸)")

    # 기지 파괴? Yes → GameOver
    sv.arrow(dr[0], dr[1], gox-70, got+23, color=ORG, label="Yes")

    # Shop → WaveDecision
    sv.arrow(CX, sb, CX, vdt[1], label="구매 완료")

    # WaveIndex<5 → PreWave (loop back left)
    loop_x = LX - 55
    sv.polyline([(CX-140, vdb[1]+30), (loop_x, vdb[1]+30),
                 (loop_x, pt+(pb-pt)//2), (LX, pt+(pb-pt)//2)],
                color=CYAN, label="No (다음 웨이브)")

    # WaveIndex==5 → Victory
    sv.arrow(vdr[0], vdr[1], vix-70, vit+23, color=GRN, label="Yes")

    # GameOver / Victory → MainMenu (R키)
    rv_x = vix + 115
    sv.polyline([(gox+90, got+23), (rv_x, got+23),
                 (rv_x, mt+(mb-mt)//2), (LX+W, mt+(mb-mt)//2)],
                color=ORG, dashed=True)
    sv.polyline([(vix+90, vit+23), (rv_x+20, vit+23),
                 (rv_x+20, mt+(mb-mt)//2+6)],
                color=GRN, dashed=True)
    sv.label_box(rv_x+10, mt+(mb-mt)//2-30, "R키 재시작", color=GRAY)

    # ── phase labels ───────────────────────────────────────
    phase_x = LX - 20
    for y, lbl, col in [(pt+10,"PHASE 1",CYAN),(gt+10,"PHASE 2",GRN),
                         (ct+10,"PHASE 3",MAG),(st+10,"PHASE 4",CYAN)]:
        sv.txt(phase_x, y, lbl, size=9, fill=col, anchor="end")

    sv.save("flowchart_game_loop.svg")


# ═══════════════════════════════════════════════════════════
# FLOWCHART 2 — Resource Collection Flow
# ═══════════════════════════════════════════════════════════
def flow_resource():
    sv = SVG(760, 980, "NEON DRIFT — Resource Collection Flow")
    CX = 360
    W  = 280
    LX = CX - W//2

    # Nodes
    cx,s_t,s_b = sv.terminator(LX, 60, W, 44, "플레이어 발사 (좌클릭)", color=CYAN)

    _,a_t,a_b,_,_ = sv.state(LX, 130, W, "LineTrace 실행",
        sub=["Muzzle 위치에서 AimDir 방향", "HitResult 수신"],
        color=CYAN)

    d1t, d1b, d1l, d1r = sv.diamond(CX, 225, 280, 56, "IDamageable 구현?", color=CYAN)

    # No branch → miss (right)
    miss_x = CX + 200
    sv.txt(miss_x, 258, "Miss", size=11, fill=GRAY, anchor="middle")
    sv.e(f'<rect x="{miss_x-36}" y="244" width="72" height="28" '
         f'fill="{CARD}" stroke="{GRAY}" stroke-width="1" rx="14"/>')
    sv.txt(miss_x, 262, "종료", size=11, fill=GRAY)

    _,b_t,b_b,_,_ = sv.state(LX, 308, W, "Execute_TakeHit() 호출",
        sub=["Damage, AttackerPower 전달"],
        color=CYAN)

    d2t, d2b, d2l, d2r = sv.diamond(CX, 393, 300, 56, "AttackerPower >= RequiredPower?", color=ORG)

    # No → spark
    spark_x = CX + 215
    sv.e(f'<rect x="{spark_x-46}" y="406" width="92" height="28" '
         f'fill="{ODIM}" stroke="{ORG}" stroke-width="1" rx="4"/>')
    sv.txt(spark_x, 424, "스파크 이펙트", size=10, fill=ORG)
    sv.txt(spark_x, 436, "(파괴 불가)", size=9, fill=GRAY)

    _,c_t,c_b,_,_ = sv.state(LX, 478, W, "ResourceBlock HP 감소",
        sub=["HP -= Damage", "MID 피격 플래시"],
        color=GRN)

    d3t, d3b, d3l, d3r = sv.diamond(CX, 566, 180, 52, "HP <= 0?", color=GRN)

    # No → survive
    surv_x = CX + 155
    sv.e(f'<rect x="{surv_x-38}" y="578" width="76" height="26" '
         f'fill="{GDIM}" stroke="{GRN}" stroke-width="1" rx="13"/>')
    sv.txt(surv_x, 595, "유지", size=11, fill=GRN)

    _,d_t,d_b,_,_ = sv.state(LX, 648, W, "AResourceShard 스폰",
        sub=["ShardMin~Max 개수 랜덤", "Impulse로 흩어짐"],
        color=CYAN)

    d4t, d4b, d4l, d4r = sv.diamond(CX, 736, 260, 52, "자석 반경 내 진입?", color=CYAN)

    _,e_t,e_b,_,_ = sv.state(LX, 818, W, "호밍 가속 → 기체에 흡수",
        sub=["Velocity += Dir * HomingAccel * Dt", "AddResources(Value)"],
        color=GRN)

    cx2,f_t,f_b = sv.terminator(LX, 900, W, 44, "자원 누적 완료 → 상점 사용 가능", color=GRN)

    # Arrows
    sv.arrow(CX, s_b, CX, a_t, color=CYAN)
    sv.arrow(CX, a_b, CX, d1t[1], color=CYAN)
    sv.arrow(d1r[0], d1r[1], miss_x, 244, color=GRAY, label="No")
    sv.arrow(CX, d1b[1], CX, b_t, label="Yes (AResourceBlock)")
    sv.arrow(CX, b_b, CX, d2t[1], color=ORG)
    sv.arrow(d2r[0], d2r[1], spark_x-46, 420, color=ORG, label="No")
    sv.arrow(CX, d2b[1], CX, c_t, color=GRN, label="Yes")
    sv.arrow(CX, c_b, CX, d3t[1], color=GRN)
    sv.arrow(d3r[0], d3r[1], surv_x-38, 591, color=GRN, label="No")
    sv.arrow(CX, d3b[1], CX, d_t, color=CYAN, label="Yes")
    sv.arrow(CX, d_b, CX, d4t[1], color=CYAN)
    sv.arrow(CX, d4b[1], CX, e_t, label="Yes (TWeakObjectPtr)")
    sv.arrow(CX, e_b, CX, f_t, color=GRN)

    # No branch of d4 — wait (small loop)
    wait_x = LX - 60
    sv.polyline([(CX-130, d4b[1]+26), (wait_x, d4b[1]+26),
                 (wait_x, d4t[1]-10), (CX-130, d4t[1]-10)],
                color=GRAY, dashed=True)
    sv.label_box(wait_x-30, d4t[1]+8, "No (대기)", color=GRAY)

    sv.save("flowchart_resource.svg")


# ═══════════════════════════════════════════════════════════
# FLOWCHART 3 — Combat / Damage Processing Flow
# ═══════════════════════════════════════════════════════════
def flow_combat():
    sv = SVG(800, 1000, "NEON DRIFT — Combat & Damage Processing Flow")
    CX = 380
    W  = 300
    LX = CX - W//2

    # Nodes
    _,s_t,s_b = sv.terminator(LX, 60, W, 44, "전투 단계 시작 (COMBAT)", color=MAG)

    _,a_t,a_b,_,_ = sv.state(LX, 130, W, "몬스터 스폰 펌프 (Tick)",
        sub=["SpawnAccum += DeltaTime", "SpawnAccum >= SpawnInterval → 스폰"],
        color=CYAN)

    _,b_t,b_b,_,_ = sv.state(LX, 228, W, "AMonster 기지 추적",
        sub=["TargetLoc = Base->GetActorLocation()", "Wander 오프셋으로 경로 다변화"],
        color=MAG)

    d1t, d1b, d1l, d1r = sv.diamond(CX, 328, 280, 56, "기지 공격 범위 진입?", color=ORG)

    _,c_t,c_b,_,_ = sv.state(LX, 414, W, "Base.TakeHit(AttackDPS·Dt, 0)",
        sub=["IDamageable 인터페이스 호출", "HP 감소 → MID 피격 플래시"],
        color=ORG)

    d2t, d2b, d2l, d2r = sv.diamond(CX, 512, 200, 52, "Base HP <= 0?", color=ORG)

    _,d_t,d_b,_,_ = sv.state(LX, 594, W, "플레이어 / 포탑 반격",
        sub=["LineTrace → Execute_TakeHit(Dmg, Power)", "Monster HP 감소"],
        color=CYAN)

    d3t, d3b, d3l, d3r = sv.diamond(CX, 692, 220, 52, "Monster HP <= 0?", color=CYAN)

    _,e_t,e_b,_,_ = sv.state(LX, 774, W, "OnMonsterKilled()",
        sub=["AliveMonsters--", "파티클·사운드 재생"],
        color=GRN)

    d4t, d4b, d4l, d4r = sv.diamond(CX, 872, 240, 52, "AliveMonsters == 0?", color=GRN)

    # Terminals
    go_x = CX + 195
    sv.e(f'<rect x="{go_x-64}" y="518" width="128" height="40" '
         f'fill="{ODIM}" stroke="{ORG}" stroke-width="2" rx="20"/>')
    sv.txt(go_x, 542, "GAME OVER", size=12, fill=ORG, bold=True)

    shop_x = CX
    sv.e(f'<rect x="{shop_x-60}" y="950" width="120" height="40" '
         f'fill="{CDIM}" stroke="{CYAN}" stroke-width="2" rx="20"/>')
    sv.txt(shop_x, 974, "→ SHOP 진입", size=12, fill=CYAN, bold=True)

    # Arrows
    sv.arrow(CX, s_b, CX, a_t, color=MAG)
    sv.arrow(CX, a_b, CX, b_t, color=CYAN)
    sv.arrow(CX, b_b, CX, d1t[1], color=MAG)

    # 범위 진입? Yes → attack
    sv.arrow(CX, d1b[1], CX, c_t, color=ORG, label="Yes")
    # No → 이동 계속 (loop)
    wait_x = LX - 65
    sv.polyline([(CX-140, d1b[1]+26), (wait_x, d1b[1]+26),
                 (wait_x, b_t + (b_b-b_t)//2), (LX, b_t + (b_b-b_t)//2)],
                color=GRAY, dashed=True)
    sv.label_box(wait_x-28, b_t+(b_b-b_t)//2-16, "No (이동)", color=GRAY)

    sv.arrow(CX, c_b, CX, d2t[1], color=ORG)
    # Base HP<=0? Yes → GameOver
    sv.arrow(d2r[0], d2r[1], go_x-64, 538, color=ORG, label="Yes")
    # No → 반격
    sv.arrow(CX, d2b[1], CX, d_t, color=CYAN, label="No")

    sv.arrow(CX, d_b, CX, d3t[1], color=CYAN)
    # Monster survive loop
    surv_x = LX - 70
    sv.polyline([(CX-110, d3b[1]+26), (surv_x, d3b[1]+26),
                 (surv_x, d_t+(d_b-d_t)//2), (LX, d_t+(d_b-d_t)//2)],
                color=GRAY, dashed=True)
    sv.label_box(surv_x-28, d_t+(d_b-d_t)//2-16, "No (생존)", color=GRAY)

    # Yes → OnMonsterKilled
    sv.arrow(CX, d3b[1], CX, e_t, color=GRN, label="Yes")
    sv.arrow(CX, e_b, CX, d4t[1], color=GRN)

    # 전멸? Yes → Shop
    sv.arrow(CX, d4b[1], CX, 950, color=GRN, label="Yes (웨이브 클리어)")
    # No → 계속 전투
    loop_x2 = LX - 65
    sv.polyline([(CX-120, d4b[1]+28), (loop_x2, d4b[1]+28),
                 (loop_x2, a_t+(a_b-a_t)//2), (LX, a_t+(a_b-a_t)//2)],
                color=CYAN, dashed=True)
    sv.label_box(loop_x2-28, a_t+(a_b-a_t)//2-16, "No (계속)", color=GRAY)

    sv.save("flowchart_combat.svg")


# ── Run ─────────────────────────────────────────────────────
flow_game_loop()
flow_resource()
flow_combat()
print("\nAll 3 flowcharts saved to docs/Images/")
