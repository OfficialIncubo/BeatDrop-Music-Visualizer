'use strict';

document.documentElement.classList.add('js');

const menuToggle = document.querySelector('.menu-toggle');
const nav = document.querySelector('#site-nav');

function closeMenu(returnFocus = false) {
  menuToggle.setAttribute('aria-expanded', 'false');
  nav.classList.remove('is-open');
  if (returnFocus) menuToggle.focus();
}

menuToggle.addEventListener('click', () => {
  const open = menuToggle.getAttribute('aria-expanded') !== 'true';
  menuToggle.setAttribute('aria-expanded', String(open));
  nav.classList.toggle('is-open', open);
});
nav.addEventListener('click', (event) => {
  if (event.target.closest('a')) closeMenu();
});
document.addEventListener('keydown', (event) => {
  if (event.key === 'Escape' && menuToggle.getAttribute('aria-expanded') === 'true') {
    closeMenu(true);
  }
});
document.addEventListener('click', (event) => {
  if (!event.target.closest('.site-header')) closeMenu();
});
matchMedia('(min-width: 1101px)').addEventListener('change', (event) => {
  if (event.matches) closeMenu();
});

const headerBrand = document.querySelector('.site-header .brand');
const heroWordmark = document.querySelector('.hero-wordmark');
if ('IntersectionObserver' in window && headerBrand && heroWordmark) {
  const header = headerBrand.closest('.site-header');
  let brandObserver;
  const observeHeroWordmark = () => {
    brandObserver?.disconnect();
    const headerHeight = Math.ceil(header.getBoundingClientRect().height);
    brandObserver = new IntersectionObserver(([entry]) => {
      headerBrand.classList.toggle('is-expanded', !entry.isIntersecting);
    }, { rootMargin: `-${headerHeight}px 0px 0px 0px`, threshold: 0 });
    brandObserver.observe(heroWordmark);
  };
  observeHeroWordmark();
  window.addEventListener('resize', observeHeroWordmark, { passive: true });
}

const thumbnails = [...document.querySelectorAll('.gallery-thumb')];
const galleryImage = document.querySelector('#gallery-image');
const galleryLink = document.querySelector('#gallery-full');
const galleryCaption = document.querySelector('#gallery-caption');
const galleryCount = document.querySelector('#gallery-count');
const galleryStatus = document.querySelector('#gallery-status');
const imageDialog = document.querySelector('#image-dialog');
const dialogImage = document.querySelector('#dialog-image');
const dialogCaption = document.querySelector('#dialog-caption');
let selectedIndex = 0;

function updateDialog() {
  dialogImage.src = thumbnails[selectedIndex].href;
  dialogImage.alt = thumbnails[selectedIndex].dataset.alt;
  dialogCaption.textContent = thumbnails[selectedIndex].dataset.caption;
}

function showScreenshot(index, announce = true) {
  selectedIndex = (index + thumbnails.length) % thumbnails.length;
  const selected = thumbnails[selectedIndex];
  galleryImage.src = selected.href;
  galleryImage.alt = selected.dataset.alt;
  galleryLink.href = selected.href;
  galleryLink.setAttribute('aria-label', 'Open screenshot at full size: ' + selected.dataset.caption);
  galleryCaption.textContent = selected.dataset.caption;
  galleryCount.textContent = String(selectedIndex + 1).padStart(2, '0') + ' / ' + thumbnails.length;
  thumbnails.forEach((thumbnail, i) => {
    thumbnail.classList.toggle('is-active', i === selectedIndex);
    if (i === selectedIndex) thumbnail.setAttribute('aria-current', 'true');
    else thumbnail.removeAttribute('aria-current');
  });
  // Scroll only the thumbnail strip, without moving the rest of the page.
  const strip = selected.parentElement;
  const offset = selected.offsetLeft - strip.offsetLeft;
  if (offset < strip.scrollLeft) strip.scrollLeft = offset;
  else if (offset + selected.offsetWidth > strip.scrollLeft + strip.clientWidth) {
    strip.scrollLeft = offset + selected.offsetWidth - strip.clientWidth;
  }
  if (announce) {
    galleryStatus.textContent = 'Screenshot ' + (selectedIndex + 1) + ' of ' +
      thumbnails.length + ': ' + selected.dataset.caption;
  }
  if (imageDialog.open) updateDialog();
}

function isPlainClick(event) {
  return event.button === 0 && !event.ctrlKey && !event.metaKey && !event.shiftKey && !event.altKey;
}

thumbnails.forEach((thumbnail, index) => {
  thumbnail.addEventListener('click', (event) => {
    if (!isPlainClick(event)) return;
    event.preventDefault();
    showScreenshot(index);
  });
});
document.querySelector('#gallery-previous').addEventListener('click', () => showScreenshot(selectedIndex - 1));
document.querySelector('#gallery-next').addEventListener('click', () => showScreenshot(selectedIndex + 1));
document.querySelector('#dialog-previous').addEventListener('click', () => showScreenshot(selectedIndex - 1));
document.querySelector('#dialog-next').addEventListener('click', () => showScreenshot(selectedIndex + 1));

function handleGalleryKeys(event) {
  if (event.key === 'ArrowLeft' || event.key === 'ArrowRight') {
    event.preventDefault();
    showScreenshot(selectedIndex + (event.key === 'ArrowLeft' ? -1 : 1));
  }
}
document.querySelector('#gallery').addEventListener('keydown', handleGalleryKeys);
imageDialog.addEventListener('keydown', handleGalleryKeys);
galleryLink.addEventListener('click', (event) => {
  if (!isPlainClick(event) || typeof imageDialog.showModal !== 'function') return;
  event.preventDefault();
  updateDialog();
  imageDialog.showModal();
  document.body.classList.add('modal-open');
});
document.querySelector('.dialog-close').addEventListener('click', () => imageDialog.close());
imageDialog.addEventListener('click', (event) => {
  if (event.target === imageDialog) imageDialog.close();
});
imageDialog.addEventListener('close', () => {
  document.body.classList.remove('modal-open');
});
showScreenshot(0, false);

const demoPlayer = BeatDropWidgets.mountVideoPlaylist({
  video: document.querySelector('#demo-video'),
  links: [...document.querySelectorAll('[data-demo-video]')],
  previous: document.querySelector('#video-previous'),
  next: document.querySelector('#video-next'),
  shuffle: document.querySelector('#video-shuffle'),
  counter: document.querySelector('#video-counter'),
  status: document.querySelector('#video-status')
});
document.querySelector('[data-watch-demo]').addEventListener('click', () => {
  demoPlayer.play();
});

BeatDropWidgets.loadGitHubStars(document.querySelector('#github-stars'));
