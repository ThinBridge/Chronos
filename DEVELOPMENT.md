# Development Guidelines

* Git config guidelines
* Branch guidelines
* Issue guidelines
* Release guidelines

## Git config guidelines

* Set `git config diff.cp932.textconv "iconv -f CP932 -t UTF-8"` to avoid diffs due to character code
  * E.g. Execute `git clone --config diff.cp932.textconv="iconv -f CP932 -t UTF-8" git@github.com:ThinBridge/Chronos.git` when cloning.

## Branch guidelines

* Use master branch as stable branch
* Use topic branch to implement new features
* Don't directly commit to master branch

## Issue and Pull request guidelines

* Create new issue before implement new features
  * Discuss and review specification beforehand
* Create new pull request corresponding to existing issue
* Each pull request must be reviewed
* Assign bug, enhancement or breaking-change label to pull request

## Release guidelines

See [HOWTOBUILD](https://github.com/ThinBridge/Chronos/blob/master/HOWTOBUILD.md) to build Chronos.

* Create a new tag vN.N.N.N (such as v12.x.y.z)
  * y must be Chromium version
  * z must be revision to update Chromium
* Create new issue for release
* Create new release via https://github.com/ThinBridge/Chronos/releases/new
* Test assets for release
* Publish release note
