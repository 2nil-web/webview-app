#!/bin/bash

# Create a patch file to go from old file to new file and check it
function create_and_check_patch () {
  old_file=$1
  new_file=$2
  diff "$old_file" "$new_file" >patch_from_old_to_new
  cp "$old_file" test_file
  # Apply patch : patch old_file <patch_from_old_to_new
  patch test_file <patch_from_old_to_new
  # Check that old_file and new_file are now identical ==> command should output nothing and return 0 (echo $?) : diff old_file new_file
  diff test_file "$new_file"
}

#create_and_check_patch wv_ghrepo/webview/webview.h my_webview.h

# Get actual commit of my_webview.h
wv_actual_commit=$(head -1 my_webview.h | sed 's/.*GITHUB_WEBVIEW_REPO_COMMIT=//')

# Prepare to get last commit of webview in github
WV_REPO=wv_ghrepo
test -d "${WV_REPO}" || mkdir "${WV_REPO}"
cd "${WV_REPO}"

if [ -d webview ]
then
  cd webview
  git pull
else
  git clone https://github.com/webview/webview.git
  cd webview
fi

# Get last (short) commit of webview.h
wv_new_commit=$(git rev-list --abbrev-commit HEAD webview.h | head -1)
# git rev-list HEAD --oneline webview.h # with one line comment

NEW_COMMIT_FILE=webview.h.new_commit

# If commits are different then retrieve new 
if [ "${wv_new_commit}" = "${wv_actual_commit}" ]
then
  echo "No change in webview.h from the github webview repo"
  rm -f ../../${NEW_COMMIT_FILE}
else
  echo "// GITHUB_WEBVIEW_REPO_COMMIT=$wv_new_commit" > ../../${NEW_COMMIT_FILE}
  cat webview.h >> ../../${NEW_COMMIT_FILE}
  echo "webview.h from webview github repo has a new commit, have a look at file '${NEW_COMMIT_FILE}'."
  echo "Use command 'diff my_webview.h ${NEW_COMMIT_FILE}' to check."
fi

