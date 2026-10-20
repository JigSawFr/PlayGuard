#!/usr/bin/env bash
# The page of a GitHub release, laid out (tools/release_notes.py): "PlayGuard
# X.Y.Z" as its title, then the summary, the figures, the commit list folded
# (what users notice, then what contributors do), the screens that changed,
# the install section with the assets' sizes, the build report and the
# contributors. The summary is .github/release-notes/<version>.md, else one a
# language model writes through OpenRouter when OPENROUTER_API_KEY is set
# (tools/summarize_release.py, from the notes and the descriptions of their
# pull requests) when the release has something to highlight,
# else none. With --apply, the release also gets
# a discussion in the RELEASE_DISCUSSION_CATEGORY category (default
# Announcements; "none" for none), and the discussion's post kept the same
# as the page.
#
#   tools/release_page.sh vX.Y.Z [--apply]
#
# Without --apply the new notes are only printed (and written to the job
# summary in Actions). Needs gh (GH_TOKEN, GH_REPO); run from the repository's
# root. The build report is this run when it is release-please's (the release
# job), else the last release-please run started before the release was
# created. A release laid out before is laid out again from its lists (a
# changed summary file, a new layout), keeping its summary when there is no
# file for it: no second call to OpenRouter. The same page is not edited.
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
# release-please's notes, also out of a page laid out before (a re-run).
python3 "$here/release_notes.py" --version "$version" --repo-url "$repo_url" --raw < "$work/notes.md" > "$work/raw.md"
python3 "$here/release_notes.py" --version "$version" --repo-url "$repo_url" --existing-summary < "$work/notes.md" > "$work/existing.md"

args=()
summary=".github/release-notes/$version.md"
if [ -f "$summary" ]; then
  args=(--summary "$summary"); how="the hand-written summary ($summary)"
elif [ -s "$work/existing.md" ]; then
  how="the summary already on the page"
elif [ -z "${OPENROUTER_API_KEY:-}" ]; then
  how="no summary (no $summary, no OpenRouter key)"
else
  # Exit 3: nothing in the release to highlight (no change a user notices, or
  # the model said so). Not a failure: the page goes on with the lists alone.
  rc=0
  python3 "$here/summarize_release.py" "$version" .github/release-notes resources/img/emoji < "$work/raw.md" > "$work/summary.md" || rc=$?
  if [ "$rc" = 0 ]; then
    args=(--summary "$work/summary.md"); how="a summary by ${OPENROUTER_MODEL:-the default model} through OpenRouter"
  elif [ "$rc" = 3 ]; then
    how="no summary (nothing to highlight)"
  else
    echo "::warning title=Release page::no summary: OpenRouter gave none (see the log above)"
    how="no summary (OpenRouter gave none)"
  fi
fi

gh release download "$tag" -p compat.json -D "$work" 2>/dev/null && args+=(--compat "$work/compat.json") || true
gh release view "$tag" --json assets > "$work/assets.json" 2>/dev/null && args+=(--assets "$work/assets.json") || true
# The previous release, from the compare link release-please puts in the heading.
prev=$(sed -n '1,/^## /s|.*/compare/\([^/ ]*\)\.\.\..*|\1|p' "$work/notes.md" | head -n 1)
if [ -n "$prev" ] && gh api "repos/$GH_REPO/compare/$prev...$tag" > "$work/compare.json" 2>/dev/null; then
  args+=(--compare "$work/compare.json")
elif [ -z "$prev" ] && gh api "repos/$GH_REPO/contributors" --jq '[.[] | select(.type == "User") | .login]' > "$work/contributors.json" 2>/dev/null; then
  args+=(--contributors "$work/contributors.json")   # a first release: everyone so far
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

if [ "$apply" != --apply ]; then
  cat "$work/new.md"
  { echo "## $tag laid out, with $how (not published)"; echo; cat "$work/new.md"; } >> "$summary_out"
  exit 0
fi

if cmp -s "$work/notes.md" "$work/new.md"; then
  echo "$tag already has this page: nothing to edit."
  echo "ℹ️ **Release page** — $tag already has this page" >> "$summary_out"
else
  gh release edit "$tag" --title "PlayGuard $version" --notes-file "$work/new.md"
  echo "✅ **Release page** — $tag laid out, with $how" >> "$summary_out"
fi

# The release's discussion: opened from the notes, but GitHub does not
# follow later edits of the release, so its post is kept the same as the
# page here (a GraphQL updateDiscussion; skipped when it already is). Only
# its author can edit it: the app that published the release, whose token is
# DISCUSSION_TOKEN (GITHUB_TOKEN when unset).
discussion=$(gh api "repos/$GH_REPO/releases/tags/$tag" --jq '.discussion_url // empty' 2>/dev/null || true)
category=${RELEASE_DISCUSSION_CATEGORY:-Announcements}
if [ -z "$discussion" ] && [ "$category" != none ]; then
  if gh release edit "$tag" --discussion-category "$category" > /dev/null; then
    echo "✅ **Release page** — a discussion in $category" >> "$summary_out"
  else
    echo "::warning title=Release page::no discussion: is there a \"$category\" category (RELEASE_DISCUSSION_CATEGORY)?"
  fi
elif [ -n "$discussion" ]; then
  query='query($owner: String!, $repo: String!, $number: Int!) {
    repository(owner: $owner, name: $repo) { discussion(number: $number) { id body } } }'
  if gh api graphql -f query="$query" -F owner="${GH_REPO%%/*}" -F repo="${GH_REPO#*/}" -F number="${discussion##*/}" \
       --jq '.data.repository.discussion' > "$work/discussion.json" 2>/dev/null && [ -s "$work/discussion.json" ]; then
    python3 -c 'import json, sys; print(json.load(open(sys.argv[1]))["body"], end="")' "$work/discussion.json" > "$work/discussion.md"
    if [ "$(cat "$work/discussion.md")" = "$(cat "$work/new.md")" ]; then
      echo "ℹ️ **Release page** — its discussion already has this page" >> "$summary_out"
    elif GH_TOKEN="${DISCUSSION_TOKEN:-$GH_TOKEN}" gh api graphql -f query='mutation($id: ID!, $body: String!) {
             updateDiscussion(input: {discussionId: $id, body: $body}) { discussion { url } } }' \
           -F id="$(python3 -c 'import json, sys; print(json.load(open(sys.argv[1]))["id"])' "$work/discussion.json")" \
           -F body=@"$work/new.md" > /dev/null; then
      echo "✅ **Release page** — its [discussion]($discussion) updated" >> "$summary_out"
    else
      echo "::warning title=Release page::the discussion ($discussion) could not be updated: its author, the release's (the PlayGuard Release app), can edit it; give the app the Discussions (read and write) permission"
    fi
  else
    echo "::warning title=Release page::the discussion ($discussion) could not be read"
  fi
fi
