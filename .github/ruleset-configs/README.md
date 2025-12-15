# Repository Ruleset Configurations

This directory contains JSON configuration files for GitHub repository rulesets. These files serve as reference configurations and can be used with the GitHub API to programmatically set up branch protection rules.

## Files

- **main-protection.json** - Configuration for main branch protection
- **development-branches.json** - Configuration for feature/bugfix/rfc branches
- **release-tags.json** - Configuration for release tag protection

## Usage

### Option 1: GitHub UI (Recommended for Manual Setup)

1. Go to repository **Settings** → **Rules** → **Rulesets**
2. Click **New ruleset** → **New branch ruleset** (or tag ruleset)
3. Manually configure rules according to the JSON specifications
4. Set enforcement to **Active**

### Option 2: GitHub CLI

```bash
# Authenticate with GitHub
gh auth login

# Apply main branch protection
gh api --method POST \
  -H "Accept: application/vnd.github+json" \
  /repos/wiifm74/equilibria/rulesets \
  --input main-protection.json

# Apply development branch ruleset
gh api --method POST \
  -H "Accept: application/vnd.github+json" \
  /repos/wiifm74/equilibria/rulesets \
  --input development-branches.json

# Apply release tag protection
gh api --method POST \
  -H "Accept: application/vnd.github+json" \
  /repos/wiifm74/equilibria/rulesets \
  --input release-tags.json
```

### Option 3: GitHub REST API

```bash
# Replace YOUR_TOKEN with a personal access token with repo admin permissions
TOKEN="YOUR_TOKEN"
REPO="wiifm74/equilibria"

# Apply main branch protection
curl -X POST \
  -H "Authorization: token $TOKEN" \
  -H "Accept: application/vnd.github+json" \
  https://api.github.com/repos/$REPO/rulesets \
  -d @main-protection.json

# Apply development branch ruleset
curl -X POST \
  -H "Authorization: token $TOKEN" \
  -H "Accept: application/vnd.github+json" \
  https://api.github.com/repos/$REPO/rulesets \
  -d @development-branches.json

# Apply release tag protection
curl -X POST \
  -H "Authorization: token $TOKEN" \
  -H "Accept: application/vnd.github+json" \
  https://api.github.com/repos/$REPO/rulesets \
  -d @release-tags.json
```

## Configuration Details

### main-protection.json

Enforces the following on the `main` branch:
- Pull request required before merge (1 approval)
- Status checks must pass (CI workflow)
- Branch must be up to date before merge
- No force pushes allowed
- Branch cannot be deleted
- Direct pushes blocked

### development-branches.json

Applies to branches: `feature/*`, `bugfix/*`, `rfc/*`, `copilot/*`
- No restrictions (allows fast iteration)
- Direct pushes allowed
- Force pushes allowed
- No required reviews or status checks

### release-tags.json

Applies to tags matching: `v*` (e.g., v1.0.0)
- Tags cannot be deleted
- Tags cannot be force-updated
- Preserves release history

## Notes

- The `required_status_checks` context in `main-protection.json` should match your CI workflow name
- Adjust the `required_approving_review_count` if you need more reviewers
- The `bypass_actors` array can be used to allow specific users/teams to bypass rules
- Rulesets require admin access to the repository to configure

## References

- [GitHub Rulesets API Documentation](https://docs.github.com/en/rest/repos/rules)
- [GitHub Rulesets Guide](https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/managing-rulesets/about-rulesets)
