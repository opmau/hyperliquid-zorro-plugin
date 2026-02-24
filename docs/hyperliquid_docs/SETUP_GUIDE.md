# Making Hyperliquid + XYZ Docs Available to Claude Code

## Setup

### Step 1: Run the scraper

```bash
pip install requests beautifulsoup4 markdownify
python scrape_hl_docs.py
```

This creates two directories:
- `docs/hyperliquid-api/` — 16 pages covering the core Hyperliquid API (REST, WebSocket, signing, rate limits, etc.)
- `docs/xyz/` — 19 pages covering trade[XYZ] HIP-3 DEX (perp mechanics, asset specs, margin, liquidation, etc.)

### Step 2: Add to your CLAUDE.md

Copy the contents of `CLAUDE_MD_SNIPPET.md` into your project's `CLAUDE.md` (create one if it doesn't exist).

### Step 3: Verify

```bash
claude "Read the info endpoint docs and list all available query types"
claude "What are the XYZ oracle price mechanics?"
```

## How it works

- Claude Code reads files lazily from `CLAUDE.md` references — it won't load all 35 files at once
- It only pulls in docs relevant to your current query
- The numbered filenames ensure logical reading order when Claude needs to scan multiple pages

## Maintenance

- Re-run `python scrape_hl_docs.py` when either doc set is updated
- Commit the docs to your repo, or gitignore and re-scrape periodically
- The scraper is polite (1s delay between requests)

## Alternative: `@url` in Claude Code

For one-off questions you can paste URLs inline during a session:

```
claude> @https://hyperliquid.gitbook.io/hyperliquid-docs/for-developers/api/websocket
       How do I subscribe to orderbook updates?
```

This works but burns context tokens each time and doesn't persist across sessions.
