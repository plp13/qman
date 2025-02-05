# Git hooks

These scripts add the following functionality to `git`:
- Call `bump.sh` upon commit, to set the program version

## Installation instructions

```
$ cp pre-commit post-commit ../.git/hooks
$ cd ../.git/hooks
$ chmod +x pre-commit post-commit
```
