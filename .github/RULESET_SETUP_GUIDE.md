# Quick Setup Guide: Applying Repository Rulesets

This guide helps you quickly apply the defined rulesets to the Equilibria repository.

## Prerequisites

You must have **admin access** to the repository to configure rulesets.

## Option 1: GitHub UI (Recommended)

### Step 1: Navigate to Rulesets

1. Go to https://github.com/wiifm74/equilibria
2. Click **Settings** (top navigation)
3. In the left sidebar, scroll to **Code and automation**
4. Click **Rules** → **Rulesets**

### Step 2: Create main Branch Protection Ruleset

1. Click **New ruleset** → **New branch ruleset**
2. Fill in the form:
   - **Ruleset Name:** `main branch protection`
   - **Enforcement status:** Active
   - **Target branches:** 
     - Click "Add target"
     - Select "Include by pattern"
     - Enter: `main`
3. **Bypass list:** Leave empty (no bypasses)
4. Enable these rules:
   - ✅ **Restrict deletions** (checked)
   - ✅ **Require a pull request before merging** (checked)
     - Required approvals: `1`
     - Uncheck "Dismiss stale reviews"
     - Leave code owner review unchecked (for now)
   - ✅ **Require status checks to pass** (checked)
     - Click "Add checks"
     - Add: `CI` (if you have a CI workflow)
     - Check "Require branches to be up to date before merging"
   - ✅ **Block force pushes** (checked)
5. Click **Create**

### Step 3: Create Development Branches Ruleset

1. Click **New ruleset** → **New branch ruleset**
2. Fill in the form:
   - **Ruleset Name:** `development branches`
   - **Enforcement status:** Active
   - **Target branches:** 
     - Click "Add target" four times for each pattern:
       - `feature/*`
       - `bugfix/*`
       - `rfc/*`
       - `copilot/*`
3. **Rules:** Leave all unchecked (no restrictions)
4. **Bypass list:** Leave empty
5. Click **Create**

### Step 4: Create Release Tag Protection Ruleset

1. Click **New ruleset** → **New tag ruleset**
2. Fill in the form:
   - **Ruleset Name:** `release tag protection`
   - **Enforcement status:** Active
   - **Target tags:**
     - Click "Add target"
     - Select "Include by pattern"
     - Enter: `v*`
3. Enable these rules:
   - ✅ **Restrict deletions** (checked)
   - ✅ **Block force pushes** (checked)
4. Click **Create**

### Step 5: Verify Setup

1. Go back to **Settings** → **Rules** → **Rulesets**
2. You should see three active rulesets:
   - ✅ main branch protection
   - ✅ development branches
   - ✅ release tag protection
3. Test by trying to push directly to main (should be blocked)

---

## Option 2: GitHub CLI

If you prefer command-line setup:

```bash
# Install GitHub CLI (if not already installed)
# macOS: brew install gh
# Ubuntu: sudo apt install gh
# Windows: choco install gh

# Authenticate
gh auth login

# Navigate to repo directory
cd /path/to/equilibria

# Apply rulesets using JSON configs
gh api --method POST \
  -H "Accept: application/vnd.github+json" \
  /repos/wiifm74/equilibria/rulesets \
  --input .github/ruleset-configs/main-protection.json

gh api --method POST \
  -H "Accept: application/vnd.github+json" \
  /repos/wiifm74/equilibria/rulesets \
  --input .github/ruleset-configs/development-branches.json

gh api --method POST \
  -H "Accept: application/vnd.github+json" \
  /repos/wiifm74/equilibria/rulesets \
  --input .github/ruleset-configs/release-tags.json

# Verify
gh api /repos/wiifm74/equilibria/rulesets | jq '.[] | {name, enforcement}'
```

---

## Option 3: REST API with curl

If you have a personal access token with repo admin permissions:

```bash
TOKEN="your_github_token_here"
REPO="wiifm74/equilibria"

# Apply main branch protection
curl -X POST \
  -H "Authorization: token $TOKEN" \
  -H "Accept: application/vnd.github+json" \
  https://api.github.com/repos/$REPO/rulesets \
  -d @.github/ruleset-configs/main-protection.json

# Apply development branches
curl -X POST \
  -H "Authorization: token $TOKEN" \
  -H "Accept: application/vnd.github+json" \
  https://api.github.com/repos/$REPO/rulesets \
  -d @.github/ruleset-configs/development-branches.json

# Apply release tags
curl -X POST \
  -H "Authorization: token $TOKEN" \
  -H "Accept: application/vnd.github+json" \
  https://api.github.com/repos/$REPO/rulesets \
  -d @.github/ruleset-configs/release-tags.json
```

---

## Verification Checklist

After applying rulesets, verify they work:

- [ ] Try pushing directly to main → Should be **blocked**
- [ ] Try creating a PR to main without review → Should be **blocked** from merging
- [ ] Try merging PR without CI passing → Should be **blocked**
- [ ] Push to feature/test branch → Should **succeed**
- [ ] Force push to feature/test branch → Should **succeed**
- [ ] Create a tag v0.0.1 → Should **succeed**
- [ ] Try deleting tag v0.0.1 → Should be **blocked**

---

## Troubleshooting

### "I don't see the Rulesets option"

- Ensure you have admin access to the repository
- Rulesets require GitHub Free/Pro/Team/Enterprise
- Try using branch protection rules as a fallback

### "CI status check not available"

- The CI workflow name in `main-protection.json` is generic
- Update it to match your actual CI workflow name
- Or skip status checks until CI is configured

### "Ruleset API returns 404"

- Verify you have admin permissions
- Check your authentication token has `repo` scope
- Ensure GitHub Enterprise version supports rulesets (2022+)

### "I want to bypass rulesets temporarily"

- Add yourself to the bypass list in the ruleset settings
- Only use for emergencies (defeats the purpose)
- Remove bypass after resolving the issue

---

## Next Steps

After rulesets are active:

1. ✅ Document in team communication that main is now protected
2. ✅ Create a feature branch to test the workflow
3. ✅ Open a test PR and verify CI runs
4. ✅ Ensure all team members understand the new workflow
5. ✅ Update this guide if you encounter issues

For detailed documentation, see:
- [.github/RULESETS.md](RULESETS.md) - Complete ruleset specifications
- [docs/dev_quickstart.md](../docs/dev_quickstart.md) - Developer workflow guide
