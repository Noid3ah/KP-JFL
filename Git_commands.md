# Git Commands Reference

## 1. Check Repository Status

```bash
git status
```

Displays modified files, staged changes and the current branch.

## 2. Get the Latest Changes

```bash
git switch main
git pull origin main
```

Updates your local `main` branch with the latest remote changes.

## 3. Switch to Your Feature Branch

```bash
git switch feature/your-section
```

Switches to your assigned branch.

To create a new branch:

```bash
git switch -c feature/your-section
```

## 4. Stage and Commit Changes

```bash
git add .
git commit -m "Describe your changes"
```

Stages your changes and creates a commit.

## 5. Push Changes to GitHub

First push:

```bash
git push -u origin feature/your-section
```

Subsequent pushes:

```bash
git push
```

Uploads your commits to GitHub.

## 6. Update Your Feature Branch

To incorporate the latest changes from `main`:

```bash
git switch main
git pull origin main
git switch feature/your-section
git merge main
```

## 7. View Available Branches

```bash
git branch
```

Lists your local branches. The asterisk indicates your current branch.

---

**Remember:**

- Pull before starting a work session.
- Commit frequently.
- Push whenever you have meaningful working progress.
- Work on your assigned feature branch.
- Never push directly to `main`.
- Use Pull Requests to integrate completed features into `main`.
