#!/usr/bin/env bash
# The page of a GitHub release, laid out (tools/release_notes.py): "PlayGuard
# X.Y.Z" as its title, then the summary, the figures, the commit list folded
# (what users notice, then what contributors do), the screens that changed,
# the install section with the assets' sizes, the build report and the
# contributors. The summary is .github/release-notes/<version>.md, else one a
# language model writes through OpenRouter when OPENROUTER_API_KEY is set
# (tools/summarize_release.py), else none. With --apply, the release also gets
# a discussion in the RELEASE_DISCUSSION_CATEGORY category (default
# Announcements; "none" for none).
#
#   tools/release_page.sh vX.Y.Z [--apply]
#
# Without --apply the new notes are only printed (and written to the job
# summary in Actions). Needs gh (GH_TOKEN, GH_REPO); run from the repository's
# root. The build report is this run when it is release-please's (the release
# job), else the last release-please run started before the release was
# created. A release already laid out (a <details> block) is left as it is.
# Nothing here fails a release: a part that cannot be had is left out.
set -euo pipefail
tag=${1:?usage: tools/release_page.sh vX.Y.Z [--apply]}
apply=${2:-}
version=${tag#v}
repo_url="${GITHUB_SERVER_URL:-https://github.com}/${GH_REPO:?GH_REPO is needed}"
summary_out=${GITHUB_STEP_SUMMARY:-/dev/null}
work=$(mktemp -d)
here=$(cd "$(dirname "$0")" && pwd)

gh release view "$tag" --json body --jq .body > "$work/notes.md"
if grep -q '^<details>' "$work/notes.md"; then
  echo "$tag is already laid out: nothing to do."
  echo "ℹ️ **Release page** — $tag already laid out" >> "$summary_out"
  exit 0
fi

args=()
summary=".github/release-notes/$version.md"
if [ -f "$summary" ]; then
  args=(--summary "$summary"); how="the hand-written summary ($summary)"
elif [ -z "${OPENROUTER_API_KEY:-}" ]; then
  how="no summary (no $summary, no OpenRouter key)"
elif python3 "$here/summarize_release.py" "$version" .github/release-notes resources/img/emoji < "$work/notes.md" > "$work/summary.md"; then
  args=(--summary "$work/summary.md"); how="a summary by ${OPENROUTER_MODEL:-the default model} through OpenRouter"
else
  echo "::warning title=Release page::no summary: OpenRouter gave none (see the log above)"
  how="no summary (OpenRouter gave none)"
fi

gh release download "$tag" -p compat.json -D "$work" 2>/dev/null && args+=(--compat "$work/compat.json") || true
gh release view "$tag" --json assets > "$work/assets.json" 2>/dev/null && args+=(--assets "$work/assets.json") || true
# The previous release, from the compare link release-please puts in the heading.
prev=$(sed -n '1,/^## /s|.*/compare/\([^/ ]*\)\.\.\..*|\1|p' "$work/notes.md" | head -n 1)
if [ -n "$prev" ] && gh api "repos/$GH_REPO/compare/$prev...$tag" > "$work/compare.json" 2>/dev/null; then
  args+=(--compare "$work/compare.json")
fi
if [ "${GITHUB_WORKFLOW:-}" = release-please ] && [ -n "${GITHUB_RUN_ID:-}" ]; then
  run_url="$repo_url/actions/runs/$GITHUB_RUN_ID"
else
  created=$(gh release view "$tag" --json createdAt --jq .createdAt 2>/dev/null || true)
  run_url=$(gh run list -w release-please.yml -L 100 --json url,createdAt \
    --jq "[.[] | select(.createdAt <= \"$created\")][0].url // empty" 2>/dev/null || true)
fi
[ -z "$run_url" ] || args+=(--run-url "$run_url")
python3 "$here/release_notes.py" --version "$version" --repo-url "$repo_url" "${args[@]}" < "$work/notes.md" > "$work/new.md"

if [ "$apply" = --apply ]; then
  gh release edit "$tag" --title "PlayGuard $version" --notes-file "$work/new.md"
  echo "✅ **Release page** — $tag laid out, with $how" >> "$summary_out"
  category=${RELEASE_DISCUSSION_CATEGORY:-Announcements}
  if [ "$category" != none ]; then
    if gh release edit "$tag" --discussion-category "$category" > /dev/null; then
      echo "✅ **Release page** — a discussion in $category" >> "$summary_out"
    else
      echo "::warning title=Release page::no discussion: is there a \"$category\" category (RELEASE_DISCUSSION_CATEGORY)?"
    fi
  fi
else
  cat "$work/new.md"
  { echo "## $tag laid out, with $how (not published)"; echo; cat "$work/new.md"; } >> "$summary_out"
fi
