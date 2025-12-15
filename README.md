# equilibria
Modular control for distillation and phase-change processes

## Quick Links

- **[Development Quickstart](docs/dev_quickstart.md)** - Get started contributing
- **[Repository Rulesets](.github/RULESETS.md)** - Branch protection and governance
- **[Ruleset Setup Guide](.github/RULESET_SETUP_GUIDE.md)** - How to apply rulesets
- **[Architecture](docs/architecture.md)** - System design
- **[Protocol](docs/protocol.md)** - Communication protocols

## Repository Structure

```
equilibria/
├── controller/     # Real-time process control (C++)
├── api/           # HTTP/WebSocket gateway (Python)
├── ui/            # Web monitoring interface (Vue.js)
├── shared/        # Protocol definitions (C++)
├── sim/           # Test harnesses and MCU simulation (C++)
├── config/        # Configuration files
└── docs/          # Documentation
```

## Contributing

All contributions must go through pull requests. See [Development Quickstart](docs/dev_quickstart.md) for the workflow.

### Branch Protection

The `main` branch is protected. All changes require:
- Pull request with at least 1 approval
- CI checks passing
- Branch up to date with main

Development branches (`feature/*`, `bugfix/*`, `rfc/*`, `copilot/*`) have no restrictions.

See [Repository Rulesets](.github/RULESETS.md) for complete governance details.
