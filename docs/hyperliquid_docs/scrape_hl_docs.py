#!/usr/bin/env python3
"""
Scrape Hyperliquid API docs and trade[XYZ] docs from GitBook into markdown files
for Claude Code reference.

Usage:
    pip install requests beautifulsoup4 markdownify
    python scrape_hl_docs.py

Creates:
    docs/hyperliquid-api/  - Hyperliquid core API documentation
    docs/xyz/              - trade[XYZ] HIP-3 DEX documentation
"""

import os
import re
import time
import requests
from bs4 import BeautifulSoup
from markdownify import markdownify as md

# ── Hyperliquid API docs ─────────────────────────────────────────────────────

HL_BASE = "https://hyperliquid.gitbook.io/hyperliquid-docs/for-developers/api"
HL_OUTPUT = "docs/hyperliquid-api"
HL_PAGES = [
    ("00-api-overview",            ""),
    ("01-notation",                "/notation"),
    ("02-asset-ids",               "/asset-ids"),
    ("03-tick-and-lot-size",       "/tick-and-lot-size"),
    ("04-nonces-and-api-wallets",  "/nonces-and-api-wallets"),
    ("05-info-endpoint",           "/info-endpoint"),
    ("06-exchange-endpoint",       "/exchange-endpoint"),
    ("07-websocket",               "/websocket"),
    ("08-error-responses",         "/error-responses"),
    ("09-signing",                 "/signing"),
    ("10-rate-limits",             "/rate-limits-and-user-limits"),
    ("11-activation-gas-fee",      "/activation-gas-fee"),
    ("12-optimizing-latency",      "/optimizing-latency"),
    ("13-bridge2",                 "/bridge2"),
    ("14-deploying-hip-assets",    "/deploying-hip-1-and-hip-2-assets"),
    ("15-hip-3-deployer-actions",  "/hip-3-deployer-actions"),
]

# ── trade[XYZ] docs ──────────────────────────────────────────────────────────

XYZ_BASE = "https://docs.trade.xyz"
XYZ_OUTPUT = "docs/xyz"
XYZ_PAGES = [
    ("00-architecture",            "/"),
    ("01-perp-overview",           "/perp-mechanics/overview"),
    ("02-oracle-price",            "/perp-mechanics/oracle-price"),
    ("03-mark-price",              "/perp-mechanics/mark-price"),
    ("04-external-price",          "/perp-mechanics/external-price"),
    ("05-funding",                 "/perp-mechanics/funding"),
    ("06-fees",                    "/perp-mechanics/fees"),
    ("07-commodities",             "/asset-directory/commodities"),
    ("08-equity-indices",          "/asset-directory/equity-indices"),
    ("09-stocks",                  "/asset-directory/stocks"),
    ("10-fx",                      "/asset-directory/fx"),
    ("11-specification-index",     "/consolidated-resources/specification-index"),
    ("12-holiday-closures",        "/consolidated-resources/holiday-closures"),
    ("13-roll-schedules",          "/consolidated-resources/roll-schedules"),
    ("14-margin-modes",            "/risk-and-margining/margin-modes"),
    ("15-liquidation-mechanics",   "/risk-and-margining/liquidation-mechanics"),
    ("16-auto-deleveraging",       "/risk-and-margining/auto-deleveraging"),
    ("17-oracle-time-constant",    "/changelog/oracle-time-constant-update"),
    ("18-funding-rate-update",     "/changelog/funding-rate-formula-update"),
]


def fetch_and_convert(url: str) -> str:
    """Fetch a GitBook page and return cleaned markdown."""
    resp = requests.get(url, timeout=30, headers={
        "User-Agent": "Mozilla/5.0 (compatible; docs-scraper/1.0)"
    })
    resp.raise_for_status()

    soup = BeautifulSoup(resp.text, "html.parser")

    # Remove nav, header, footer, sidebar, scripts, styles
    for tag in soup.find_all(["nav", "header", "footer", "script", "style", "noscript"]):
        tag.decompose()

    # GitBook content selectors (try in order)
    content = None
    for selector in ["main", '[data-testid="page.contentEditor"]', ".page-body", '[role="main"]']:
        content = soup.select_one(selector)
        if content:
            break

    if not content:
        content = soup.body

    if not content:
        return "# Error\nCould not extract content from page."

    # Remove "Powered by GitBook" and nav links
    for el in content.find_all(string=re.compile(r"Powered by GitBook")):
        parent = el.parent
        if parent:
            parent.decompose()

    # Convert to markdown
    markdown = md(str(content), heading_style="atx", strip=["img"])

    # Clean up excessive whitespace
    markdown = re.sub(r"\n{4,}", "\n\n\n", markdown)
    markdown = re.sub(r"[ \t]+\n", "\n", markdown)
    markdown = markdown.strip()

    return markdown


def scrape_doc_set(base_url: str, output_dir: str, pages: list, label: str):
    """Scrape a full doc set into output_dir."""
    os.makedirs(output_dir, exist_ok=True)

    print(f"\n{'='*60}")
    print(f"  {label}")
    print(f"  {len(pages)} pages -> {output_dir}/")
    print(f"{'='*60}\n")

    success = 0
    for filename, path in pages:
        url = base_url.rstrip("/") + path if path != "/" else base_url
        print(f"  [{filename}] {url}")

        try:
            content = fetch_and_convert(url)
            header = f"<!-- Source: {url} -->\n\n"

            filepath = os.path.join(output_dir, f"{filename}.md")
            with open(filepath, "w", encoding="utf-8") as f:
                f.write(header + content)

            print(f"    -> {len(content)} chars")
            success += 1
        except Exception as e:
            print(f"    !! Error: {e}")

        time.sleep(1)  # rate limit politeness

    print(f"\n  {success}/{len(pages)} pages scraped successfully.")
    return success


def write_hl_index(output_dir: str):
    """Write index for Hyperliquid API docs."""
    path = os.path.join(output_dir, "INDEX.md")
    with open(path, "w") as f:
        f.write("# Hyperliquid API Documentation\n\n")
        f.write(f"Source: {HL_BASE}\n\n")
        f.write("## Endpoints\n\n")
        f.write("- **Mainnet REST**: https://api.hyperliquid.xyz\n")
        f.write("- **Testnet REST**: https://api.hyperliquid-testnet.xyz\n")
        f.write("- **Mainnet WS**: wss://api.hyperliquid.xyz/ws\n")
        f.write("- **Testnet WS**: wss://api.hyperliquid-testnet.xyz/ws\n\n")
        f.write("## Doc Pages\n\n")
        for filename, _ in HL_PAGES:
            title = filename.split("-", 1)[1].replace("-", " ").title()
            f.write(f"- [{title}]({filename}.md)\n")
    print(f"  Index: {path}")


def write_xyz_index(output_dir: str):
    """Write index for XYZ docs."""
    path = os.path.join(output_dir, "INDEX.md")
    with open(path, "w") as f:
        f.write("# trade[XYZ] Documentation\n\n")
        f.write(f"Source: {XYZ_BASE}\n\n")
        f.write("XYZ is a HIP-3 DEX on Hyperliquid for equities, indices, commodities, and FX.\n\n")
        f.write("## Key Info\n\n")
        f.write("- **DEX name**: xyz\n")
        f.write("- **Deployer**: 0x88806a71D74ad0a510b350545C9aE490912F0888\n")
        f.write("- **Oracle Updater**: 0x1234567890545d1Df9EE64B35Fdd16966e08aCEC\n")
        f.write("- **App**: https://app.trade.xyz/trade\n\n")
        f.write("## Doc Pages\n\n")
        for filename, _ in XYZ_PAGES:
            title = filename.split("-", 1)[1].replace("-", " ").title()
            f.write(f"- [{title}]({filename}.md)\n")
    print(f"  Index: {path}")


def main():
    scrape_doc_set(HL_BASE, HL_OUTPUT, HL_PAGES, "Hyperliquid API Docs")
    write_hl_index(HL_OUTPUT)

    scrape_doc_set(XYZ_BASE, XYZ_OUTPUT, XYZ_PAGES, "trade[XYZ] Docs")
    write_xyz_index(XYZ_OUTPUT)

    print(f"\n{'='*60}")
    print("  All done! Add the following to your CLAUDE.md")
    print(f"{'='*60}")
    print("""
  ## Hyperliquid API Reference
  See docs/hyperliquid-api/INDEX.md

  ## trade[XYZ] Reference
  See docs/xyz/INDEX.md
""")


if __name__ == "__main__":
    main()
