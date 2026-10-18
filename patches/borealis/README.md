# PlayGuard's fixes to borealis

`extern/borealis` is the upstream submodule (xfangfang/borealis, wiliwili
branch), left at an upstream commit. The fixes PlayGuard needs before
upstream has them are kept here as patches, applied in name order to the
submodule's files when CMake configures (`CMakeLists.txt`). A patch already
in place is left as it is; one that no longer applies stops the configure,
so a borealis bump cannot silently drop a fix. `git status` shows the
submodule as modified once they are applied: that is expected.

To change one: edit the file in `extern/borealis`, then
`git -C extern/borealis diff > patches/borealis/NNNN-name.patch` (one fix
per patch) and `git -C extern/borealis checkout .` before configuring again.
Once upstream has a fix, the bump that brings it removes its patch.

| Patch | Why |
|---|---|
| `0001-i18n-look-strings-up-without-exceptions.patch` | `getRawStr` looked each string up in the current locale first, with a lookup that throws when it misses. In English (the default locale) the current one is empty, so every `getStr` / `_i18n` threw and caught an exception, and `operator[]` added a null to that locale each time. The empty locale is now skipped and the lookup is `at()` on a const catalogue. Desktop build, 20,000 strings: 45 ms → 7.5 ms in English, French unchanged (8 ms). |
