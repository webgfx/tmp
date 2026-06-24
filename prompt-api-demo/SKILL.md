---
name: prompt-api-demo
description: >-
  Run and explain a beautiful browser demo of the experimental Prompt API
  (on-device Phi-4-mini) in Microsoft Edge Canary/Dev. Use when asked to "demo
  the Prompt API", "show built-in AI in Edge", "try LanguageModel / window.ai",
  showcase on-device model download progress, or set up the edge://flags needed
  to run a local SLM in the browser. The same API ships in Chrome, so the demo
  works there too once the equivalent flag is enabled.
---

# Prompt API Demo — On-device AI in Microsoft Edge

A single, self-contained HTML page that demonstrates the experimental
[Prompt API](https://learn.microsoft.com/en-us/microsoft-edge/web-platform/prompt-api):
a small language model (**Phi-4-mini**) built into Microsoft Edge that runs
**entirely on the user's device**. The same `LanguageModel` API is the one
Chrome exposes for its built-in AI, so this demo runs in Chrome too once the
equivalent flag is enabled.

The demo is deliberately built to **surface the work happening in the
background** — feature detection, availability checks, the one-time model
download (with a live progress bar), session creation, streaming token
generation, and context-window usage — all traced in a real-time activity log.

## What the demo shows

- **Feature detection** — `"LanguageModel" in self`, rendered as a live badge.
- **Capability probe** — `LanguageModel.params()` (default/max temperature and
  top-K) wired into the sampling sliders, and `LanguageModel.availability()`
  (`unavailable` / `downloadable` / `downloading` / `available`).
- **Model download, visualized** — a `monitor` listener on the
  `downloadprogress` event drives a progress bar with MB-loaded/total and
  percentage, so the user can literally watch Phi-4-mini download on first run.
- **Streaming chat** — `promptStreaming()` renders tokens as they arrive, with
  time-to-first-token, total latency, and an approximate tokens/sec readout.
- **Context-window meter** — `session.inputUsage` / `session.inputQuota`.
- **Session lifecycle** — system prompt via `initialPrompts`, settings changes
  that rebuild the session, `AbortController` to stop generation, and
  `session.destroy()` on reset.
- **Activity log** — every API call and event, timestamped.

## Files

```
prompt-api-demo/
  SKILL.md            # this file
  demo/
    index.html        # the entire demo — no build step, no dependencies
```

## How to run it

1. **Install Microsoft Edge Canary or Dev**, version **138.0.3309.2 or later**.
   (Or use a recent Chrome — see "Chrome" below.)
2. **Enable the flag:**
   - Open `edge://flags` (or `chrome://flags`).
   - Search for **"Prompt API for on-device language model"**.
   - Set it to **Enabled**.
   - *(Optional)* also enable **"Enable on device AI model debug logs"** for
     local debug logging.
   - Restart the browser.
3. **Check your hardware** at `edge://on-device-internals`:
   - **Device performance class** should be **High** or greater for Phi-4-mini.
   - Requirements: Windows 10/11 or macOS 13.3+, ≥ 20 GB free disk on the Edge
     profile volume, **≥ 5.5 GB VRAM**, and an unmetered network for the
     one-time download.
   - On **Medium/Low** devices, enable **"Enable prerelease on-device language
     model"** (Edge 150.0.4070+) to use the smaller **Aion-1.0-Instruct** model,
     which can run on CPU.
4. **Open the demo.** Because it's a single static file, any of these work:
   - Double-click `demo/index.html`, **or**
   - Serve it (recommended, avoids `file://` quirks):
     ```bash
     cd prompt-api-demo/demo
     python -m http.server 8000
     # then open http://localhost:8000/
     ```
5. **Use it:** pick a sample prompt or type your own, then click **Send prompt**.
   On the very first run the model downloads — watch the progress bar and the
   activity log on the left.

> **First-run download:** The model is fetched the first time any site calls the
> API, and is then **shared across all sites** in the browser. If the download
> doesn't start, restart the browser and try again.

## Chrome

The demo also works in Google Chrome, which exposes the same `LanguageModel`
global for its built-in AI (backed by Gemini Nano rather than Phi-4-mini).
Enable the equivalent **"Prompt API for Gemini Nano"** flag at
`chrome://flags`, restart, and open the page. No code changes are needed — the
demo feature-detects and adapts.

## The API at a glance

The demo exercises the full surface; the essentials:

```javascript
// 1. Is the API present?
if (!("LanguageModel" in self)) { /* flag not enabled */ }

// 2. Can the model be used? -> "unavailable" | "downloadable" | "downloading" | "available"
const availability = await LanguageModel.availability();

// 3. Sampling defaults/limits
const { defaultTemperature, maxTemperature, defaultTopK, maxTopK } =
  await LanguageModel.params();

// 4. Create a session (downloads the model on first call) and watch progress
const session = await LanguageModel.create({
  temperature: 1.0,
  topK: 3,
  initialPrompts: [{ role: "system", content: "You are a helpful assistant." }],
  monitor(m) {
    m.addEventListener("downloadprogress", (e) => {
      const pct = (e.loaded / e.total) * 100; // 0..100
    });
  },
});

// 5. Stream a response
const stream = session.promptStreaming("Write a haiku about local AI.");
for await (const chunk of stream) { /* append chunk */ }

// 6. Inspect context-window usage
console.log(session.inputUsage, "/", session.inputQuota);

// 7. Stop early / clean up
const ac = new AbortController();
session.promptStreaming(text, { signal: ac.signal });
ac.abort();          // stop generating
session.destroy();   // unload the model from memory
```

Other capabilities the API supports (easy extensions to the demo):

- **Structured output** — pass `responseConstraint` (a JSON schema or RegExp) to
  `prompt()` to force machine-parseable responses.
- **N-shot prompting** — seed `initialPrompts` with `user`/`assistant` example
  turns for more deterministic behavior.
- **Multi-message prompts** — pass an array of `{ role, content }` instead of a
  string.
- **`session.clone()`** — reuse a session's options without its conversation
  history.

## Troubleshooting

| Symptom | Cause / fix |
|---|---|
| Badge shows `LanguageModel: false` | Flag not enabled or browser too old. Enable "Prompt API for on-device language model" and restart; update to 138.0.3309.2+. |
| `availability()` is `"unavailable"` | Device performance class too low or insufficient VRAM/disk. Check `edge://on-device-internals`; try the Aion-1.0-Instruct prerelease flag. |
| Download never starts | Metered connection (model won't download), or low disk. Switch to an unmetered network; free up disk (model is deleted if free space drops below 10 GB). |
| Works on `http://localhost` but not `file://` | Serve the file over HTTP (`python -m http.server`). |

## Reference

- [Prompt API — Microsoft Edge docs](https://learn.microsoft.com/en-us/microsoft-edge/web-platform/prompt-api)
- [Built-in AI playgrounds (official)](https://microsoftedge.github.io/Demos/built-in-ai/playgrounds/prompt-api/)
- [Prompt API explainer / spec (W3C WebML)](https://webmachinelearning.github.io/prompt-api/)
