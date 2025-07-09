#!/bin/sh
# Change s.source in DeliteAI.podspec to = { :git => 'git@github.com:NimbleEdge/deliteAI-iOS.git', :tag => s.version.to_s }
# for publishing the pod on internal org.

set -eExuo pipefail

uname -m
arch

SCRIPT_DIR=$(dirname "$(realpath "$0")")
BASE_DIR=$(git -C "$SCRIPT_DIR" rev-parse --show-toplevel)

usage() {
    echo "Usage: $0 [--release]"
    exit 1
}

target_branch="dev"
target_source_repo="git@github.com:NimbleEdge/deliteAI-iOS.git"
target_podspec_repo="git@github.com:NimbleEdge/deliteAI-iOS-Podspecs.git"
target_dir_name="deliteAI-iOS"

yaml_file="$BASE_DIR/config.yml"
sdk_version=$(grep "sdk_version:" "$yaml_file" | awk '{print $2}' | tr -d '"')

"$SCRIPT_DIR/build-deliteAI-static.sh" --Release

mkdir -p $SCRIPT_DIR/release
cd $SCRIPT_DIR/release

git clone "$target_source_repo"
git clone "$target_podspec_repo"

rm -rf "$SCRIPT_DIR/release/$target_dir_name/*"

cp -r $BASE_DIR/sdks/ios/deliteAI $BASE_DIR/LICENSE $BASE_DIR/sdks/ios/DeliteAI.podspec "$target_dir_name"

cp $BASE_DIR/sdks/ios/README.md "$target_dir_name/README.md"
sed -i "" '/^## Table of Contents/,/^[^* ]/d' "$target_dir_name/README.md" #then removing `Table of Contents` readme

sed -i "" "s#0.0.1-local#$sdk_version#g" "$target_dir_name/DeliteAI.podspec"

mkdir -p "$SCRIPT_DIR/release/deliteAI-iOS-Podspecs/DeliteAI/$sdk_version"
cp "$SCRIPT_DIR/release/$target_dir_name/DeliteAI.podspec" "$SCRIPT_DIR/release/deliteAI-iOS-Podspecs/DeliteAI/$sdk_version"

deploy_changes() {
    local repo_dir="$1"
    local branch="$2"
    local version="$3"

    cd "$repo_dir" || exit
    git add .
    git status
    git commit -m "release/$version"
    git push origin "$branch" --dry-run
    git tag "$version"
    git push --tags --dry-run
    cd - > /dev/null
}

echo "Target Branch: $target_branch"
echo "Target Source Repo: $target_source_repo"
echo "Target Podspec Repo: $target_podspec_repo"
echo "Target Directory Name: $target_dir_name"

deploy_changes "$SCRIPT_DIR/release/$target_dir_name" "main" "$sdk_version"
deploy_changes "$SCRIPT_DIR/release/deliteAI-iOS-Podspecs" "$target_branch" "$sdk_version"

cd ../../
rm -rf "$SCRIPT_DIR/release"
