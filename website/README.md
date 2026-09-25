# BeatDrop website

This is a static landing page for BeatDrop. It has no build step or paid services. Logos, screenshots, videos and license notices are served from this folder. The optional star counter reads GitHub's public API.

## Media and controls

- The four supplied MP4 recordings appear in the opening player. Previous/next arrows and numbered links change the video.
- Playback starts on request. Only the selected video is loaded; the other recordings are links, not hidden video players.
- Random video selection is on by default. After a clip ends, another clip is chosen without immediately repeating the last one. Turn random selection off to play clips in order, looping back to the first clip after the fourth.
- The header shows the B icon at the top of the page. After the hero wordmark scrolls out of view, its text fades and slides in beside the icon; it fades away again when scrolling back up.
- The Features section includes the media playback shortcuts and a screenshot of the visualizer context menu. The same menu is available by right-clicking the visualizer or taskbar tray icon.
- The screenshot gallery starts with the BeatDrop startup preset and preserves the supplied preset-author captions. It supports arrows, thumbnail selection and a full-size dialog.
- Without JavaScript, video and screenshot links still open the original files, the navigation remains visible, and the FAQs use native HTML controls.

## GitHub stars and contact

The star button opens the repository; it does not star automatically. The count is fetched without credentials and cached locally for one hour. An unavailable count displays a dash rather than a guessed number. Failure to retrieve it does not affect the rest of the site.

The contact links open a mail composer addressed to `beatdropmusicvisualizer@gmail.com`. No message is sent by the website.
The support section links to the project maintainer's Ko-fi page. Donations are optional and do not change preset or media permissions.

## Content and permissions

The Linux FAQ distinguishes Wine-based compatibility from a native Linux build. The Winlator settings are attributed to the maintainer's Android experience, with the reported Desktop Mode and Windows media-session limitations retained. The macOS FAQ separates a community report about MilkDrop 3 from unconfirmed BeatDrop compatibility in Parallels or Prism.

The original software license and preset notice are in `licenses/`. Preset-specific restrictions are explained separately from the software's BSD license. Media supplied for this page has not been assigned a new blanket license.

## Local checks

Run `node --check website/script.js`, `node --check website/site-widgets.js`, and `node --test tests/website-widgets.test.cjs` from the repository root. The tests cover video selection, random switching, playback-request races and star-count failures without launching a browser. Visual layout, video decoding and browser-specific playback still need a browser check.

## Publish with GitHub Pages

1. Commit and push the `website/` folder and `.github/workflows/pages-site.yml` to `master`.
2. In the repository, open **Settings → Pages** and choose **GitHub Actions** as the build and deployment source.
3. The workflow publishes the site on pushes that change `website/`. GitHub Pages provides the free project URL: `https://officialincubo.github.io/BeatDrop-Music-Visualizer/`.

The repository must remain public to use GitHub Pages on GitHub Free. GitHub issues the site's HTTPS certificate at no charge.

## Domain options

The `github.io` URL is free and works without DNS setup. A domain registered in your own name (such as `beatdrop.example`) normally has a recurring registration fee, even though hosting and HTTPS can remain free. A third-party free subdomain may work as a custom domain if its provider allows the required DNS records, but availability, approval, and continued service are controlled by that provider.

To connect a domain later, configure its DNS to point to GitHub Pages, then add it under **Settings → Pages → Custom domain**. Do not add a `CNAME` file until a real domain has been selected.
