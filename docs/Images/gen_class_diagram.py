#!/usr/bin/env python3
"""NEON DRIFT - Class Diagram SVG Generator"""

out = r"C:\Users\user\Documents\Unreal Projects\NEONDRIFT\docs\Images\class_diagram.svg"

W, H = 1400, 940
BG      = "#0d0d1a"
CYAN    = "#00f0ff"
MAG     = "#ff2fd0"
WHITE   = "#d8daf0"
GRAY    = "#6677aa"
CARD    = "#0e0e22"
BORDER  = "#1e1e44"
CDIM    = "#002830"
MDIM    = "#28001a"
LBLUE   = "#00bcd4"

parts = []

def emit(s): parts.append(s)

# ── helpers ────────────────────────────────────────────────
def esc(s): return s.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;")

def rct(x,y,w,h,fill,stroke,rx=5):
    emit(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" fill="{fill}" stroke="{stroke}" stroke-width="1.5" rx="{rx}"/>')

def txt(x,y,s,size=12,fill=WHITE,anchor="middle",bold=False,italic=False):
    fw = "bold" if bold else "normal"
    fs = "italic" if italic else "normal"
    emit(f'<text x="{x}" y="{y}" font-family="\'Courier New\',monospace" font-size="{size}" '
         f'font-weight="{fw}" font-style="{fs}" fill="{fill}" text-anchor="{anchor}">{esc(s)}</text>')

def hline(x,y,w,color=BORDER,dash=""):
    da = f'stroke-dasharray="{dash}"' if dash else ""
    emit(f'<line x1="{x}" y1="{y}" x2="{x+w}" y2="{y}" stroke="{color}" stroke-width="1" {da}/>')

def vline(x,y1,y2,color=BORDER,dash=""):
    da = f'stroke-dasharray="{dash}"' if dash else ""
    emit(f'<line x1="{x}" y1="{y1}" x2="{x}" y2="{y2}" stroke="{color}" stroke-width="1" {da}/>')

def arr(x1,y1,x2,y2,color=CYAN,dashed=False,label=None):
    da = 'stroke-dasharray="7,4"' if dashed else ""
    emit(f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" stroke="{color}" stroke-width="1.4" {da} '
         f'marker-end="url(#ah_{color[1:]})"/>')
    if label:
        mx,my = (x1+x2)//2,(y1+y2)//2
        txt(mx,my-5,label,size=10,fill=color,anchor="middle")

def classbox(x,y,w, title, stereo=None, members=[], methods=[], color=CYAN, is_iface=False):
    """Draw a class/interface box. Returns (center_x, top_y, bot_y, left_mid_y, right_mid_y)."""
    hdr_h = 48 if stereo else 36
    row_h = 16
    body_h = max(40, len(members)*row_h + (8 if members else 0) +
                      len(methods)*row_h + (8 if methods else 0) + 12)
    h = hdr_h + body_h

    card_fill = MDIM if is_iface else CARD
    border_c  = MAG  if is_iface else color
    hdr_fill  = "#280018" if is_iface else CDIM

    # outer box
    rct(x,y,w,h,card_fill,border_c)
    # header fill
    rct(x,y,w,hdr_h,hdr_fill,border_c)
    # cover rounded bottom of header
    rct(x,y+hdr_h-4,w,6,hdr_fill,"none",rx=0)

    # stereotype + title
    if stereo:
        txt(x+w//2, y+15, stereo, size=10, fill=border_c, italic=True)
        txt(x+w//2, y+33, title,  size=13, fill=border_c, bold=True)
    else:
        txt(x+w//2, y+23, title, size=13, fill=border_c, bold=True)

    # divider
    hline(x, y+hdr_h, w, border_c)

    cy = y + hdr_h + 14
    for m in members:
        txt(x+10, cy, m, size=11, fill=GRAY, anchor="start")
        cy += row_h

    if members and methods:
        hline(x, cy+2, w, BORDER, dash="3,3")
        cy += 10

    for m in methods:
        txt(x+10, cy, m, size=11, fill=WHITE, anchor="start")
        cy += row_h

    cx = x + w//2
    return cx, y, y+h, y+h//2, x, x+w  # cx, top, bot, mid_y, left_x, right_x

# ── SVG header ─────────────────────────────────────────────
emit(f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" viewBox="0 0 {W} {H}">')
emit('<defs>')
for cname,cval in [("cyan",CYAN),("mag",MAG),("gray",GRAY),("lblue",LBLUE)]:
    emit(f'''  <marker id="ah_{cval[1:]}" markerWidth="9" markerHeight="7"
    refX="8" refY="3.5" orient="auto">
    <polygon points="0 0,9 3.5,0 7" fill="{cval}"/>
  </marker>''')
emit('</defs>')

# background
emit(f'<rect width="{W}" height="{H}" fill="{BG}"/>')

# subtle grid
for gx in range(0,W,60):
    emit(f'<line x1="{gx}" y1="0" x2="{gx}" y2="{H}" stroke="#ffffff" stroke-width="0.15" opacity="0.08"/>')
for gy in range(0,H,60):
    emit(f'<line x1="0" y1="{gy}" x2="{W}" y2="{gy}" stroke="#ffffff" stroke-width="0.15" opacity="0.08"/>')

# ── Title ──────────────────────────────────────────────────
txt(W//2, 28, "NEON DRIFT — Class Diagram", size=18, fill=CYAN, bold=True)
hline(40, 38, W-80, BORDER)

# ── Legend ─────────────────────────────────────────────────
lx, ly = W-270, 55
rct(lx-10,ly-18,260,84,CARD,BORDER)
txt(lx+115,ly-4,"Legend",size=11,fill=GRAY)
emit(f'<line x1="{lx}" y1="{ly+14}" x2="{lx+60}" y2="{ly+14}" stroke="{MAG}" stroke-width="1.4" stroke-dasharray="7,4" marker-end="url(#ah_{MAG[1:]})"/>')
txt(lx+65,ly+18,"implements interface",size=10,fill=GRAY,anchor="start")
emit(f'<line x1="{lx}" y1="{ly+34}" x2="{lx+60}" y2="{ly+34}" stroke="{CYAN}" stroke-width="1.4" marker-end="url(#ah_{CYAN[1:]})"/>')
txt(lx+65,ly+38,"owns / references",size=10,fill=GRAY,anchor="start")
emit(f'<line x1="{lx}" y1="{ly+54}" x2="{lx+60}" y2="{ly+54}" stroke="{LBLUE}" stroke-width="1.4" stroke-dasharray="5,3" marker-end="url(#ah_{LBLUE[1:]})"/>')
txt(lx+65,ly+58,"provides / spawns",size=10,fill=GRAY,anchor="start")

# ═══════════════════════════════════════════════════════════
# ROW 1: IDamageable (interface) — y=60, centered
# ═══════════════════════════════════════════════════════════
iface_x, iface_w = 490, 420
i_cx, i_top, i_bot, i_midy, i_lx, i_rx = classbox(
    iface_x, 60, iface_w,
    "IDamageable",
    stereo="<<interface>>",
    methods=["+ TakeHit(Damage: float, AttackerPower: int32) = 0"],
    color=MAG, is_iface=True
)

# ═══════════════════════════════════════════════════════════
# ROW 2: Implementors — y=180
# ═══════════════════════════════════════════════════════════
R2Y = 180
PS_cx,PS_top,PS_bot,_,PS_lx,PS_rx = classbox(
    20, R2Y, 300, "APlayerShip",
    stereo="APawn",
    members=["# Stats: FShipStats","# CurrentHP: float"],
    methods=["+ TakeHit()","# FireOnce()","# CollectShards()"],
    color=CYAN
)
MO_cx,MO_top,MO_bot,_,MO_lx,MO_rx = classbox(
    345, R2Y, 280, "AMonster",
    stereo="AActor",
    members=["+ HP, MaxHP: float","+ MoveSpeed: float","+ Color, Variant"],
    methods=["+ TakeHit()","+ Tick()"],
    color=CYAN
)
BA_cx,BA_top,BA_bot,_,BA_lx,BA_rx = classbox(
    755, R2Y, 280, "ABase",
    stereo="AActor",
    members=["+ MaxHP, CurrentHP: float"],
    methods=["+ TakeHit()","+ GetHPFraction()"],
    color=CYAN
)
RB_cx,RB_top,RB_bot,_,RB_lx,RB_rx = classbox(
    1060, R2Y, 310, "AResourceBlock",
    stereo="AActor",
    members=["- RequiredPower: int32","- HP, ShardMin/Max: int"],
    methods=["+ TakeHit()","- DropShards()","- SpawnSpark()"],
    color=CYAN
)

# dashed arrows: implementors → IDamageable
for cx,top in [(PS_cx,PS_top),(MO_cx,MO_top),(BA_cx,BA_top),(RB_cx,RB_top)]:
    arr(cx, top, i_cx, i_bot, color=MAG, dashed=True)

# ═══════════════════════════════════════════════════════════
# ROW 3: GameMode (center large) + Turrets — y=390
# ═══════════════════════════════════════════════════════════
R3Y = 398
MT_cx,MT_top,MT_bot,MT_midy,MT_lx,MT_rx = classbox(
    20, R3Y, 280, "AManualTurret",
    stereo="AActor",
    members=["+ bPlayerBoarded: bool","- Stats: FTurretStats"],
    methods=["+ Fire()","+ ApplyStats()","+ SetPlayerBoarded()"],
    color=CYAN
)
AT_cx,AT_top,AT_bot,AT_midy,AT_lx,AT_rx = classbox(
    1060, R3Y, 310, "AAutoTurret",
    stereo="AActor",
    members=["+ bEnabled: bool","- Stats: FTurretStats","- Target: TWeakObjectPtr"],
    methods=["+ SetEnabled()","+ ApplyStats()","- FindTarget()","- FireAtTarget()"],
    color=CYAN
)
GM_cx,GM_top,GM_bot,GM_midy,GM_lx,GM_rx = classbox(
    330, R3Y-20, 700, "ANeonGameMode",
    stereo="AGameModeBase",
    members=[
        "+ Phase: EGamePhase   (Single Source of Truth)",
        "+ WaveIndex: int32    Resources: int32",
        "+ Base: ABase*        ManualTurret: AManualTurret*",
        "+ AutoTurrets[]       SpawnPoints[]",
        "+ WaveTable[]         UpgradeTable[]",
    ],
    methods=[
        "+ EnterPhase(NewPhase)   + AddResources(Amount)",
        "+ ApplyUpgrade(Id)       + OnMonsterKilled()",
        "+ RequestStartWave()     + OnBaseDestroyed()",
    ],
    color=CYAN
)

# arrows: GameMode → Turrets
arr(GM_lx, GM_midy, MT_rx, MT_midy, color=CYAN)
arr(GM_rx, GM_midy, AT_lx, AT_midy, color=CYAN)
# GameMode → Base (down-left area)
arr(GM_cx-80, GM_bot, BA_cx, BA_bot, color=CYAN, label="owns ref")

# ═══════════════════════════════════════════════════════════
# ROW 4: Bottom — y=666
# ═══════════════════════════════════════════════════════════
R4Y = 680
RS_cx,RS_top,RS_bot,RS_midy,RS_lx,RS_rx = classbox(
    20, R4Y, 280, "AResourceShard",
    stereo="AActor",
    members=["+ Value: int32","- HomingTarget: TWeakObjectPtr<APlayerShip>"],
    methods=["+ AttractTo(Ship*)","+ Tick()"],
    color=CYAN
)
GI_cx,GI_top,GI_bot,GI_midy,GI_lx,GI_rx = classbox(
    360, R4Y, 400, "UNeonGameInstance",
    stereo="UGameInstance",
    members=["+ Upgrades: FUpgradeState"],
    methods=[
        "+ GetShipStats(): FShipStats",
        "+ GetManualTurretStats(): FTurretStats",
        "+ GetAutoTurretStats(): FTurretStats",
        "+ TryPurchase(Id, InOutRes, Cost, MaxLv)",
        "+ ResetRun()",
    ],
    color=CYAN
)
SP_cx,SP_top,SP_bot,SP_midy,SP_lx,SP_rx = classbox(
    820, R4Y, 240, "ASpawnPoint",
    stereo="AActor",
    members=["(spawn marker actor)"],
    methods=["+ GetSpawnTransform()"],
    color=GRAY
)

# ResourceBlock spawns ResourceShard
arr(RB_cx, RB_bot, RS_cx, RS_top, color=LBLUE, dashed=True, label="spawns")
# GameInstance provides stats → GameMode
arr(GI_cx, GI_top, GM_cx, GM_bot, color=LBLUE, label="provides stats")
# GameMode owns SpawnPoints
arr(GM_rx-60, GM_bot, SP_cx, SP_top, color=CYAN)
# PlayerShip ← ResourceShard homing
arr(RS_rx, RS_midy, PS_rx, PS_bot, color=LBLUE, dashed=True, label="homing →")

# ── EGamePhase note box ─────────────────────────────────────
# Placed below AAutoTurret, right side (clear of other boxes)
NX, NY = 1090, AT_bot + 14
rct(NX, NY, 165, 112, "#0a0a18", MAG, rx=4)
txt(NX+83, NY+16, "<<enum>>", size=10, fill=MAG, italic=True)
txt(NX+83, NY+30, "EGamePhase", size=12, fill=MAG, bold=True)
hline(NX, NY+36, 165, MAG)
for i,s in enumerate(["MainMenu","PreWave","Gather","Combat","Shop","Victory / GameOver"]):
    txt(NX+8, NY+52+i*14, s, size=10, fill=GRAY, anchor="start")
# arrow note → GameMode (right edge)
emit(f'<line x1="{NX}" y1="{NY+40}" x2="{GM_rx}" y2="{GM_midy+20}" '
     f'stroke="{MAG}" stroke-width="0.8" stroke-dasharray="4,3" opacity="0.5"/>')

emit('</svg>')

with open(out, "w", encoding="utf-8") as f:
    f.write("\n".join(parts))

print(f"Saved → {out}")
