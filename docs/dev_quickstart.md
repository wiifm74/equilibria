# Development Quickstart

This guide provides essential information for contributing to the Equilibria project.

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

## Development Workflow

### Branch Strategy

Equilibria uses a protected main branch with PR-based development. All changes must go through pull requests.

#### Branch Types

**Protected branch:**
- `main` - Production-ready code, protected by rulesets

**Development branches:**
- `feature/*` - New features (e.g., `feature/add-temperature-control`)
- `bugfix/*` - Bug fixes (e.g., `bugfix/fix-sensor-timeout`)
- `rfc/*` - Design proposals and RFCs (e.g., `rfc/new-protocol-version`)
- `copilot/*` - AI-assisted development branches

### Creating a Feature Branch

```bash
# Start from latest main
git checkout main
git pull origin main

# Create your feature branch
git checkout -b feature/my-feature

# Make your changes
# ... edit files ...

# Commit changes
git add .
git commit -m "Add my feature"

# Push to remote
git push origin feature/my-feature
```

### Opening a Pull Request

1. Push your feature branch to GitHub
2. Navigate to the repository on GitHub
3. Click "Compare & pull request"
4. Fill out the PR template:
   - Summary of changes
   - Related issue numbers
   - Scope (which components are affected)
   - Safety considerations
   - Test verification
5. Submit the PR

### Pull Request Requirements

All PRs to `main` must satisfy:

- ✅ **At least 1 approving review** (self-review OK for solo maintainer)
- ✅ **All CI checks must pass**
  - Controller build (C++)
  - API tests (Python)
  - UI build (Vue)
- ✅ **Branch must be up to date with main**
- ✅ **PR template checklist completed**

### Merging Pull Requests

Once all requirements are met:

1. Ensure CI is green ✅
2. Review the changes one final time
3. Click "Squash and merge" (recommended) or "Merge pull request"
4. Delete the feature branch after merge

## Repository Governance

### Branch Protection Rulesets

Equilibria uses GitHub rulesets to enforce code quality and safety:

#### Ruleset 1: main Branch Protection

The `main` branch is protected with:
- Pull request required before merge
- At least 1 approving review required
- Status checks must pass (CI)
- Branch must be up to date before merge
- Force pushes disallowed
- Direct pushes disallowed
- Branch deletion disallowed

**Rationale:** Prevents accidental changes to production code, ensures all changes are reviewed and tested.

#### Ruleset 2: Development Branches

Branches matching `feature/*`, `bugfix/*`, `rfc/*`, `copilot/*`:
- Direct pushes allowed
- Force pushes allowed (for rebasing/cleanup)
- No required reviews
- No required status checks

**Rationale:** Enables fast iteration during development. Protection is enforced at merge time.

#### Ruleset 3: Release Tags

Tags matching `v*` (semantic versioning):
- Tag deletion disallowed
- Force-updating tags disallowed

**Rationale:** Preserves reproducible builds and release history.

### Applying Rulesets

Repository rulesets must be configured by a repository administrator through the GitHub UI or API.

**Manual setup:**
1. Go to repository Settings → Rules → Rulesets
2. Create rulesets according to [.github/RULESETS.md](../.github/RULESETS.md)
3. Activate each ruleset

**API setup:**
Use the reference configurations in `.github/ruleset-configs/` with the GitHub API or CLI. See [.github/ruleset-configs/README.md](../.github/ruleset-configs/README.md) for details.

### Why These Rules?

- **Safety:** Process control is safety-critical; changes must be deliberate
- **Quality:** CI acts as a gatekeeper for build/test failures
- **Collaboration:** PRs enable code review and knowledge sharing
- **Flexibility:** Development branches remain unrestricted for rapid iteration
- **Scalability:** Rules work for solo developers and teams

## Building and Testing

### C++ Components (controller, shared, sim)

```bash
# From the component directory
cd controller/  # or shared/ or sim/
mkdir -p build && cd build
cmake ..
make

# Run tests
ctest
# or
make test
```

### Python API

```bash
cd api/

# Install in development mode
pip install -e .

# Run tests
pytest
pytest -v                    # Verbose output
pytest tests/test_foo.py     # Specific test file

# Lint and format
black .                      # Auto-format
ruff check .                 # Lint
mypy .                       # Type checking
```

### Vue UI

```bash
cd ui/

# Install dependencies
npm install

# Development server
npm run dev

# Production build
npm run build

# Run tests
npm run test

# Lint and format
npm run lint
npm run format
```

## Development Best Practices

### Architecture Boundaries

Respect the separation of concerns:

- **controller/** - Process control logic ONLY
- **api/** - HTTP/WebSocket gateway, NO control logic
- **ui/** - Presentation ONLY, NO business logic
- **shared/** - Protocol contracts and utilities
- **sim/** - Test harnesses

### Safety and Determinism

When working on controller code:

- ❌ No dynamic allocations in the 100ms control loop
- ✅ Use value types and POD structs for protocol frames
- ✅ Explicit state machines for process control
- ✅ Consider failure modes (timeouts, disconnects, invalid config)
- ✅ Log important state transitions

### Configuration

- Configuration is declarative and role-based
- No hard-coded sensor/actuator assumptions
- All messages and schemas must be versioned
- Configuration changes require test updates

### Testing

- Protocol encode/decode must have unit tests
- Config parsing must fail loudly on invalid input
- Controllers should expose test/simulation entry points
- Run tests before opening a PR

## Continuous Integration

Equilibria uses GitHub Actions for CI (when configured). CI runs on every PR and validates:

1. **Controller build** - C++ compilation and tests
2. **API validation** - Python linting, type checking, and tests
3. **UI build** - Vue/TypeScript compilation

All checks must pass before a PR can be merged to `main`.

## Copilot Workflow

When using GitHub Copilot:

- Copilot can commit directly to feature branches
- Copilot CANNOT push directly to `main` (ruleset enforces this)
- All Copilot-generated code goes through PR review
- CI validates Copilot changes before merge
- Use PR template checklist as a safety review

Copilot branches follow the pattern: `copilot/*`

## Getting Help

- Review architecture: [docs/architecture.md](architecture.md)
- Protocol documentation: [docs/protocol.md](protocol.md)
- Repository rulesets: [.github/RULESETS.md](../.github/RULESETS.md)
- Open an issue for questions or proposals

## Quick Reference

```bash
# Clone repository
git clone https://github.com/wiifm74/equilibria.git
cd equilibria

# Create feature branch
git checkout -b feature/my-feature

# Build all components
cd controller/build && cmake .. && make && cd ../..
cd api && pip install -e . && cd ..
cd ui && npm install && npm run build && cd ..

# Run tests
cd controller/build && ctest && cd ../..
cd api && pytest && cd ..
cd ui && npm run test && cd ..

# Open PR
git push origin feature/my-feature
# Then open PR on GitHub

# Keep branch updated
git checkout main
git pull origin main
git checkout feature/my-feature
git merge main  # or git rebase main
```
