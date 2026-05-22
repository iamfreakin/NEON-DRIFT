// ===================================================================
// NEON DRIFT — landing page interactions
// ===================================================================

// --- shrink nav on scroll ---
const nav = document.querySelector('.nav');
const onScroll = () => nav.classList.toggle('shrink', window.scrollY > 40);
window.addEventListener('scroll', onScroll, { passive: true });
onScroll();

// --- scroll reveal ---
const revealTargets = document.querySelectorAll(
  '.section, .card, .phase, .tech, .doc, .hero__stats li'
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
  { threshold: 0.12 }
);
revealTargets.forEach((el) => io.observe(el));

// --- stagger items within a grid ---
document.querySelectorAll('.cards, .tech-grid, .doc-links, .hero__stats').forEach((grid) => {
  [...grid.children].forEach((child, i) => {
    child.style.transitionDelay = `${i * 70}ms`;
  });
});

// --- count-up hero stats ---
const countEls = document.querySelectorAll('.hero__stats .num');
const animateCount = (el) => {
  const raw = el.textContent.trim();
  const isPct = raw.includes('%');
  const target = parseInt(raw, 10);
  if (Number.isNaN(target)) return;
  const start = performance.now();
  const dur = 1100;
  const tick = (now) => {
    const p = Math.min((now - start) / dur, 1);
    const eased = 1 - Math.pow(1 - p, 3);
    el.textContent = Math.round(target * eased) + (isPct ? '%' : '');
    if (p < 1) requestAnimationFrame(tick);
  };
  requestAnimationFrame(tick);
};

const statObserver = new IntersectionObserver(
  (entries) => {
    entries.forEach((entry) => {
      if (entry.isIntersecting) {
        animateCount(entry.target);
        statObserver.unobserve(entry.target);
      }
    });
  },
  { threshold: 1 }
);
countEls.forEach((el) => statObserver.observe(el));
