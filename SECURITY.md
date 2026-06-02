# Security Policy

This plugin handles **private keys** and places **real orders on a live exchange**.
Security issues are taken seriously. Please help keep users safe by reporting
responsibly.

## Reporting a vulnerability

**Do not open a public issue for security vulnerabilities.**

Instead, report privately via GitHub's
[Security Advisories](https://github.com/opmau/hyperliquid-zorro-plugin/security/advisories/new)
("Report a vulnerability"). This keeps the report confidential until a fix is
available.

Please include:

- A description of the vulnerability and its impact
- Steps to reproduce (proof of concept if possible)
- Affected version(s) / commit
- Any suggested remediation

You can expect an initial acknowledgement within a few days. We will work with
you on a fix and coordinated disclosure, and credit you in the advisory unless
you prefer to remain anonymous.

## Scope

Examples of in-scope issues:

- Mishandling, logging, or leakage of private keys or signatures
- Incorrect EIP-712 / signing logic that could produce unintended authorizations
- Order-construction bugs that could send wrong size/price/side
- Memory-safety issues exploitable via malicious exchange responses
- Dependency vulnerabilities affecting the built plugin

Out of scope:

- Vulnerabilities in Zorro itself, the Hyperliquid exchange, or third-party
  dependencies (report those upstream)
- Issues that require a compromised local machine or physical access

## Supported versions

Security fixes target the latest released version on the `main` branch. Older
tagged releases are not maintained — please upgrade to the latest release.

## Handling keys safely

- The plugin reads credentials at runtime from Zorro's User/Password fields and
  does not persist them to the repository.
- Prefer a Hyperliquid **API/agent wallet** over your master account's private
  key, and revoke it if you suspect exposure.
- Never commit `Accounts.csv`, `.env`, or any file containing a private key.
