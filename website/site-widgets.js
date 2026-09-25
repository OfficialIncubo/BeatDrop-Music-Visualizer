'use strict';

// Kept independent of the rest of the page so failures in a remote star-count
// request cannot interrupt video or gallery controls.
(function (root) {
  function mountVideoPlaylist({ video, links, previous, next, shuffle, counter, status, random = Math.random }) {
    let current = 0;
    let generation = 0;
    const firstPoster = video.getAttribute('poster');

    function updateSelection() {
      counter.textContent = 'Video ' + (current + 1) + ' of ' + links.length;
      video.setAttribute('aria-label', 'BeatDrop demonstration video ' + (current + 1));
      links.forEach((link, index) => {
        if (index === current) link.setAttribute('aria-current', 'true');
        else link.removeAttribute('aria-current');
      });
    }

    function updateStatus() {
      if (video.error) status.textContent = 'Video could not be played. Try another clip.';
      else if (video.ended) status.textContent = 'Video finished · replay or choose another';
      else if (video.paused) status.textContent = 'Paused · press play to continue';
      else status.textContent = video.muted || video.volume === 0 ? 'Playing · sound off' : 'Playing · sound on';
    }

    async function playSelected() {
      const requestGeneration = ++generation;
      status.textContent = 'Loading video…';
      try {
        await video.play();
        if (requestGeneration === generation) updateStatus();
      } catch (error) {
        // A quick second selection or Pause can cancel an older play promise.
        if (requestGeneration !== generation || error.name === 'AbortError') return;
        status.textContent = 'Press play in the video player to continue.';
      }
    }

    function select(index) {
      const selected = (index + links.length) % links.length;
      if (selected !== current) {
        generation += 1;
        const muted = video.muted;
        const volume = video.volume;
        const rate = video.playbackRate;
        video.pause();
        current = selected;
        // Only the selected clip is loaded, even with all four links visible.
        video.preload = 'metadata';
        video.src = links[current].href;
        if (current === 0 && firstPoster) video.setAttribute('poster', firstPoster);
        else video.removeAttribute('poster');
        video.load();
        video.muted = muted;
        video.volume = volume;
        video.playbackRate = rate;
        updateSelection();
      }
      playSelected();
    }

    links.forEach((link, index) => {
      link.addEventListener('click', (event) => {
        if (event.button !== 0 || event.ctrlKey || event.metaKey || event.shiftKey || event.altKey) return;
        event.preventDefault();
        select(index);
      });
    });
    previous.addEventListener('click', () => select(current - 1));
    next.addEventListener('click', () => select(current + 1));
    ['play', 'pause', 'volumechange', 'error'].forEach((name) => video.addEventListener(name, updateStatus));
    video.addEventListener('ended', () => {
      if (!video.ended) return;
      if (links.length < 2) {
        updateStatus();
        return;
      }
      if (!shuffle.checked) {
        select(current + 1);
        return;
      }
      // Choose any other clip; never repeat the video that just finished.
      const offset = 1 + Math.floor(random() * (links.length - 1));
      select(current + offset);
    });
    updateSelection();
    return { play: playSelected };
  }

  async function loadGitHubStars(element, options = {}) {
    const fetchRequest = options.fetch || root.fetch.bind(root);
    const now = options.now || Date.now;
    const cacheKey = 'beatdrop:github-stars:v1';
    const cacheLifetime = 60 * 60 * 1000;
    let storage;
    try { storage = options.storage === undefined ? root.localStorage : options.storage; } catch (_) { /* Storage can be disabled. */ }

    function showCount(count) {
      element.textContent = new Intl.NumberFormat('en').format(count);
      element.setAttribute('aria-label', count + ' GitHub stars');
      element.title = 'GitHub star count · refreshed at most once per hour';
    }

    try {
      const cached = JSON.parse(storage?.getItem(cacheKey) || 'null');
      if (cached && Number.isSafeInteger(cached.count) && cached.count >= 0 &&
          Number.isFinite(cached.at) && now() >= cached.at && now() - cached.at < cacheLifetime) {
        showCount(cached.count);
        return cached.count;
      }
    } catch (_) { /* Ignore unavailable or invalid cached data. */ }

    const controller = new AbortController();
    const timeout = setTimeout(() => controller.abort(), 8000);
    try {
      const response = await fetchRequest('https://api.github.com/repos/OfficialIncubo/BeatDrop-Music-Visualizer', {
        headers: { Accept: 'application/vnd.github+json' },
        signal: controller.signal,
        credentials: 'omit'
      });
      if (!response.ok) throw new Error('GitHub request failed');
      const data = await response.json();
      if (!Number.isSafeInteger(data.stargazers_count) || data.stargazers_count < 0) throw new Error('Invalid star count');
      showCount(data.stargazers_count);
      try { storage?.setItem(cacheKey, JSON.stringify({ count: data.stargazers_count, at: now() })); } catch (_) { /* Optional cache. */ }
      return data.stargazers_count;
    } catch (_) {
      // Keep the GitHub link usable without inventing a count when offline or rate limited.
      element.textContent = '—';
      element.setAttribute('aria-label', 'Star count unavailable');
      element.title = 'Open GitHub to see the current star count';
      return null;
    } finally {
      clearTimeout(timeout);
    }
  }

  const widgets = { mountVideoPlaylist, loadGitHubStars };
  if (typeof module !== 'undefined' && module.exports) module.exports = widgets;
  else root.BeatDropWidgets = widgets;
})(globalThis);
