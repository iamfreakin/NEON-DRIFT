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
