// ===================================================================
// NEON DRIFT — branding page interactions
// ===================================================================

// --- boot intro ---
document.body.classList.add('booting');
const boot = document.getElementById('boot');
window.addEventListener('load', () => {
  setTimeout(() => {
    boot?.classList.add('gone');
    document.body.classList.remove('booting');
    setTimeout(() => boot?.remove(), 700);
  }, 1700);
});

// --- shrink nav on scroll ---
const nav = document.querySelector('.nav');
const onScroll = () => nav.classList.toggle('shrink', window.scrollY > 40);
window.addEventListener('scroll', onScroll, { passive: true });
onScroll();

// --- logo image fallback: hide title when img loads successfully ---
const logoImg = document.querySelector('.hero__logo-img');
const logoTitle = document.querySelector('.hero__title');
if (logoImg && logoTitle) {
  logoImg.addEventListener('load', () => {
    if (logoImg.naturalWidth > 0) logoTitle.style.display = 'none';
  });
}

// --- scroll reveal ---
const revealTargets = document.querySelectorAll(
  '.hero__inner > *, .keyart, .pillars-section .section__head, .pillar, .spec-section, .controls-section, .cta__frame'
);
revealTargets.forEach((el) => el.setAttribute('data-reveal', ''));

const io = new IntersectionObserver(
  (entries) => {
    entries.forEach((entry) => {
      if (entry.isIntersecting) {
        entry.target.classList.add('in');
        io.unobserve(entry.target);
      }
    });
  },
  { threshold: 0.1 }
);
revealTargets.forEach((el) => io.observe(el));

// --- stagger ---
document.querySelectorAll('.pillars').forEach((wrap) => {
  [...wrap.querySelectorAll('.pillar')].forEach((el, i) => {
    el.style.transitionDelay = `${i * 110}ms`;
  });
});
document.querySelectorAll('.spec-grid').forEach((wrap) => {
  [...wrap.querySelectorAll('.spec')].forEach((el, i) => {
    el.style.transitionDelay = `${i * 60}ms`;
  });
});

// --- targeting mini-game ---
(() => {
  const hud = document.getElementById('targeting');
  if (!hud || !window.matchMedia('(hover: hover)').matches) return;

  const $score = document.getElementById('ts-score');
  const $hits  = document.getElementById('ts-hits');
  const $acc   = document.getElementById('ts-acc');
  const $btn   = document.getElementById('ts-btn');
  const $hint  = document.getElementById('ts-hint');

  const DURATION = 30;       // seconds
  const SPAWN_MS = 1100;
  const LIFETIME = 2400;
  const VARIANTS = ['', '--mag', '--orng'];
  const SCORES   = { '': 50, '--mag': 80, '--orng': 65 };

  let state = { score: 0, hits: 0, misses: 0, t: DURATION, active: false, spawnId: 0, tickId: 0 };

  const draw = () => {
    $score.textContent = String(state.score).padStart(4, '0');
    $hits.textContent  = String(state.hits).padStart(2, '0');
    const total = state.hits + state.misses;
    const acc = total === 0 ? 100 : Math.round((state.hits / total) * 100);
    $acc.textContent = `${acc}%`;
  };

  const popScore = (x, y, val, variant) => {
    const el = document.createElement('div');
    el.className = 'target__score' + (variant ? ' target__score' + variant : '');
    el.textContent = `+${val}`;
    el.style.left = `${x}px`;
    el.style.top  = `${y}px`;
    document.body.appendChild(el);
    setTimeout(() => el.remove(), 800);
  };

  const spawn = () => {
    const pad = 100;
    const w = window.innerWidth, h = window.innerHeight;
    const variant = VARIANTS[Math.floor(Math.random() * VARIANTS.length)];
    const x = pad + Math.random() * (w - pad * 2 - 56);
    const y = 90 + Math.random() * (h - 90 - 200 - 56);

    const t = document.createElement('div');
    t.className = 'target' + (variant ? ' target' + variant : '');
    t.style.left = `${x}px`;
    t.style.top  = `${y}px`;
    t.innerHTML = '<svg viewBox="0 0 56 56" fill="none" stroke="currentColor" stroke-width="1.5"><line x1="28" y1="2" x2="28" y2="16"/><line x1="28" y1="40" x2="28" y2="54"/><line x1="2" y1="28" x2="16" y2="28"/><line x1="40" y1="28" x2="54" y2="28"/></svg>';

    let killed = false;
    t.addEventListener('click', () => {
      if (killed || !state.active) return;
      killed = true;
      const val = SCORES[variant];
      state.score += val;
      state.hits  += 1;
      draw();
      popScore(x + 28, y, val, variant);
      t.classList.add('hit');
      setTimeout(() => t.remove(), 460);
    });

    setTimeout(() => {
      if (killed) return;
      killed = true;
      if (state.active) { state.misses += 1; draw(); }
      t.classList.add('miss');
      setTimeout(() => t.remove(), 360);
    }, LIFETIME);

    document.body.appendChild(t);
  };

  const tick = () => {
    state.t -= 1;
    $btn.textContent = `▶ ${state.t}s`;
    if (state.t <= 0) end();
  };

  const start = () => {
    state = { score: 0, hits: 0, misses: 0, t: DURATION, active: true, spawnId: 0, tickId: 0 };
    draw();
    hud.classList.add('active');
    $hint.textContent = '◆ 타겟 클릭';
    $btn.textContent = `▶ ${DURATION}s`;
    spawn();
    state.spawnId = setInterval(spawn, SPAWN_MS);
    state.tickId  = setInterval(tick, 1000);
  };

  const end = () => {
    state.active = false;
    clearInterval(state.spawnId);
    clearInterval(state.tickId);
    hud.classList.remove('active');
    document.querySelectorAll('.target').forEach(el => el.remove());
    $btn.textContent = '▶ REPLAY';
    $hint.textContent = `최종 ${state.score} · ${state.hits}/${state.hits + state.misses}`;
  };

  $btn.addEventListener('click', () => {
    if (state.active) return;
    start();
  });
})();

// --- hero mouse parallax (desktop only) ---
const hero = document.querySelector('.hero');
const supportsParallax = window.matchMedia('(min-width: 980px)').matches
  && !window.matchMedia('(prefers-reduced-motion: reduce)').matches;

if (hero && supportsParallax) {
  const layers = [...document.querySelectorAll('[data-parallax]')].map((el) => {
    const key = el.dataset.parallax;
    const depth = key === 'brand' ? 14 : (key === 'hud-tl' ? -6 : -9);
    return { el, depth };
  });

  let tx = 0, ty = 0, cx = 0, cy = 0;
  let active = false;

  hero.addEventListener('mousemove', (e) => {
    const rect = hero.getBoundingClientRect();
    tx = ((e.clientX - rect.left) / rect.width - 0.5) * 2;
    ty = ((e.clientY - rect.top) / rect.height - 0.5) * 2;
    active = true;
  });
  hero.addEventListener('mouseleave', () => { tx = 0; ty = 0; });

  const tick = () => {
    cx += (tx - cx) * 0.08;
    cy += (ty - cy) * 0.08;
    if (active || Math.abs(cx) > 0.001 || Math.abs(cy) > 0.001) {
      layers.forEach(({ el, depth }) => {
        el.style.transform = `translate(${cx * depth}px, ${cy * depth * 0.7}px)`;
      });
    }
    requestAnimationFrame(tick);
  };
  tick();
}
