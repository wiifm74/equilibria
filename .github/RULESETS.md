# Repository Rulesets

This document defines the GitHub repository rulesets for the Equilibria project. These rulesets enforce branch protection, govern development workflows, and ensure code quality through CI validation.

## Overview

Equilibria uses three primary rulesets:

1. **main branch protection** - Enforces PR-based development with CI validation
2. **Development branches** - Lightweight rules for feature work
3. **Release/tag protection** - Prevents accidental deletion of releases

These rulesets balance safety on main with speed on feature branches, suitable for solo maintainers while scaling to collaborative teams.

---

## Ruleset 1: main Branch Protection

**Applies to:** `main` branch

### Rules

- ✅ **Require pull request before merge**
  - Enforces PR-based workflow
  - Prevents accidental direct pushes
  
- ✅ **Require at least 1 approving review**
  - Can be self-review for solo maintainer
  - Ensures intentional review of changes
  
- ✅ **Require status checks to pass before merging**
  - CI workflow must succeed
  - Validates controller build (C++)
  - Validates API lint/tests (Python)
  - Validates UI build (Vue)
  
- ✅ **Require branches to be up to date before merging**
  - Prevents integration issues
  - Ensures changes are tested against latest main
  
- ✅ **Disallow force pushes**
  - Preserves commit history
  - Prevents accidental history rewriting
  
- ✅ **Disallow branch deletion**
  - Protects main branch from removal
  
- ✅ **Disallow direct pushes to main**
  - All changes must go through PRs

### Optional (Recommended for Future)

- **Require signed commits** - Can be enabled when GPG signing is adopted
- **Require linear history** - No merge commits, rebase/squash only (optional preference)

### Rationale

- Prevents accidental pushes to production branch
- Makes CI a mandatory gatekeeper
- Establishes PRs as the unit of change
- Copilot-generated code must flow through PRs
- Lightweight review process (self-approval OK for now)

---

## Ruleset 2: Development Branches

**Applies to:** Branches matching patterns:
- `feature/*`
- `bugfix/*`
- `rfc/*`
- `copilot/*`

### Rules

- ✅ **Allow direct pushes**
  - Fast iteration on feature work
  
- ✅ **Allow force pushes**
  - Enables rebasing and history cleanup
  
- ⛔ **No required reviews**
  - Keeps development velocity high
  
- ⛔ **No required status checks**
  - CI can run but doesn't block pushes
  
- ✅ **Allow branch deletion**
  - Cleanup after merge

### Rationale

- Keeps iteration fast during development
- Allows experimentation and cleanup
- Protection is enforced only at merge time (via main branch ruleset)
- Developers can manage their own branches freely

---

## Ruleset 3: Release/Tag Protection

**Applies to:** Tags matching pattern: `v*` (e.g., `v1.0.0`, `v2.1.3`)

### Rules

- ✅ **Prevent deletion of release tags**
  - Preserves release history
  
- ✅ **Prevent force-updating tags**
  - Ensures immutable releases
  
- ✅ **Require tag to match pattern** `v*`
  - Enforces semantic versioning convention

### Rationale

- Preserves reproducible builds
- Prevents accidental history rewriting once releases exist
- Maintains audit trail for deployed versions
- Future-proofs for when releases begin

---

## Implementation Instructions

### Applying Rulesets via GitHub UI

1. Navigate to repository **Settings** → **Rules** → **Rulesets**
2. Click **New ruleset** → **New branch ruleset**
3. Configure each ruleset according to the specifications above
4. Set enforcement status to **Active**

### Applying Rulesets via GitHub CLI

```bash
# Install GitHub CLI if needed
# brew install gh  # macOS
# apt install gh   # Ubuntu/Debian

# Authenticate
gh auth login

# Create rulesets using the JSON configurations
# (See ruleset-configs/ directory for JSON templates)
gh api --method POST \
  -H "Accept: application/vnd.github+json" \
  /repos/wiifm74/equilibria/rulesets \
  --input .github/ruleset-configs/main-protection.json
```

### Applying Rulesets via GitHub API

Use the reference JSON configurations in `.github/ruleset-configs/` with the GitHub REST API:

```bash
curl -X POST \
  -H "Authorization: token YOUR_TOKEN" \
  -H "Accept: application/vnd.github+json" \
  https://api.github.com/repos/wiifm74/equilibria/rulesets \
  -d @.github/ruleset-configs/main-protection.json
```

---

## CI Requirements

For the main branch ruleset to function properly, the following CI workflows should be present:

- **Controller Build** - C++ compilation and tests
- **API Tests** - Python linting, type checking, and unit tests
- **UI Build** - Vue/TypeScript compilation and build validation

These status checks must be configured as required checks in the main branch ruleset.

---

## Development Workflow

### Creating a Feature Branch

```bash
git checkout main
git pull origin main
git checkout -b feature/my-feature
# ... make changes ...
git push origin feature/my-feature
```

### Opening a Pull Request

1. Push changes to feature branch
2. Open PR against `main` branch
3. Fill out PR template checklist
4. Wait for CI to pass (required)
5. Review code changes
6. Approve PR (can self-approve)
7. Merge using squash or merge commit

### Merging to main

- Ensure all CI checks pass (enforced)
- Ensure branch is up to date with main (enforced)
- Approve the PR
- Use **Squash and merge** for clean history (recommended)

---

## Copilot & Automation Considerations

- All Copilot-generated code must flow through PRs
- Copilot can commit to feature branches but not directly to main
- CI validates Copilot changes before merge
- PR template checklist acts as lightweight safety review
- Copilot branches follow pattern: `copilot/*`

---

## Future Enhancements

As the project matures, consider:

- **Code owners** - Require reviews from specific people for certain paths
- **Signed commits** - Require GPG/SSH signing for all commits
- **Security policies** - Add SECURITY.md and vulnerability reporting
- **Release automation** - Automated tag creation and release notes
- **CLA** - Contributor license agreements for external contributors
- **Branch naming enforcement** - Require feature/bugfix/rfc prefix

---

## References

- [GitHub Rulesets Documentation](https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/managing-rulesets/about-rulesets)
- [GitHub Branch Protection](https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/managing-protected-branches/about-protected-branches)
- [Equilibria Development Quickstart](../docs/dev_quickstart.md)
