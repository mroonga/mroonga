---
name: Release
about: Issue for release
title: 'Mroonga x.xx '
labels: ''
assignees: ''

---

We plan to release x.xx in a few days.

guide: http://mroonga.org/ja/docs/developer/release.html (Japanese)

- [ ] Add a release note
  - [ ] Update `doc/source/news/XX.md`
  - [ ] Translate news
- [ ] Prepare announcement text
  - [ ] Announce
    - [ ] X (Japanese/English)
    - [ ] Facebook (Japanese/English)
- [ ] Update documentation
  - [ ] Update .rst
  - [ ] Translate (Update .po)
- [ ] Check CI https://github.com/mroonga/mroonga/actions
- [ ] Check LaunchPad Nightly https://launchpad.net/~groonga/+archive/ubuntu/nightly/+packages
- [ ] Release: `rake release`
- [ ] Tagging on mroonga/mroonga.org: `rake release`
- [ ] Announce
  - [ ] X (Japanese/English)
  - [ ] Facebook (Japanese/English)
- [ ] Update Docker image on groonga/docker: `./update.sh`
