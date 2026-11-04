# Git Workflow & Commands

This guide outlines how we will use Git to collaborate on the project, manage our individual contributions, and integrate completed features.

## 1. Before You Start Working

**Update your local repository.**

Before starting work, make sure you have the latest version of the project. This prevents you from working with outdated code or missing changes made by your teammates.

```bash
git switch main
git pull origin main
```

Next, switch to your assigned feature branch:

```bash
git switch feature/your-section
```

If you haven't created your branch yet:

```bash
git switch -c feature/your-section
```

**Why do we use branches?**

Each member works on a separate branch corresponding to their assigned section. This allows everyone to develop independently without directly affecting the main project.

We avoid working directly on `main` because it should contain the stable, integrated version of our application.

## 2. While Working on Your Feature

**Save your progress regularly.**

As you work, you should stage and commit your changes frequently.

```bash
git add .
git commit -m "Describe your changes"
```

- `git add .` – Stages your modified and new files for committing.
- `git commit` – Saves a snapshot of your staged changes.

**Why commit regularly?**

Commits act as checkpoints in your development process. If something breaks, you can refer back to previous versions of your code rather than having to reconstruct everything from scratch.

Don't wait until your entire section is finished before committing.

## 3. Upload Your Progress to GitHub

**Push your commits when you have meaningful progress.**

```bash
git push
```

For your first push to a newly created branch:

```bash
git push -u origin feature/your-section
```

**Why push?**

Committing saves your changes locally. Pushing uploads those commits to GitHub, making your progress available to the rest of the team.

Remember: Committing and pushing are two different operations.

## 4. When Your Feature Is Complete

**Create a Pull Request (PR).**

Once you're satisfied with your implementation and have tested your feature:

1. Push your latest commits to GitHub.
2. Open the repository on GitHub.
3. Create a Pull Request from your feature branch into `main`.
4. Have your changes reviewed.
5. Once approved, merge the Pull Request.

**Why use Pull Requests?**

Pull Requests allow the team to review changes before they become part of the main application. This helps us identify errors, discuss improvements, and avoid introducing broken functionality.

## 5. Keep Your Branch Updated

As other members merge their completed features into `main`, you'll need to incorporate those changes into your own branch.

First, update your local `main`:

```bash
git switch main
git pull origin main
```

Then return to your feature branch and merge the updates:

```bash
git switch feature/your-section
git merge main
```

This ensures your branch contains the latest integrated version of the project alongside your own work.

---

## Quick Reference

| Command                     | Purpose                                        |
| --------------------------- | ---------------------------------------------- |
| `git status`                | Check your current changes                     |
| `git switch main`           | Switch to the main branch                      |
| `git pull origin main`      | Download and integrate the latest main changes |
| `git switch branch-name`    | Switch to an existing branch                   |
| `git switch -c branch-name` | Create and switch to a new branch              |
| `git add .`                 | Stage changes                                  |
| `git commit -m "message"`   | Save a snapshot of your changes                |
| `git push`                  | Upload commits to GitHub                       |
| `git merge main`            | Merge main into your current branch            |

## Team Guidelines

- Always update your local repository before starting a work session.
- Work exclusively on your assigned feature branch.
- Commit frequently and use descriptive commit messages.
- Push meaningful progress regularly.
- Keep your branch synchronized with `main`.
- Never push directly to `main`.
- Use Pull Requests to integrate completed features.
- Communicate with teammates when making changes to shared files or functionality.
