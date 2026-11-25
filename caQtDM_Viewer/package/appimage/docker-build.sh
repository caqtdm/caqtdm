#!/usr/bin/env bash
# docker-build.sh - build a portable caQtDM AppImage inside Rocky Linux 8.
#
# The resulting AppImage targets glibc >= 2.28 (RHEL/Rocky/AlmaLinux 8 and
# newer). The Docker image builds and caches Qt from source.
#
# Usage:
#   caQtDM_Viewer/package/appimage/docker-build.sh [create-appimage.sh flags]
#
# Examples:
#   caQtDM_Viewer/package/appimage/docker-build.sh
#   caQtDM_Viewer/package/appimage/docker-build.sh --branch Development
#   caQtDM_Viewer/package/appimage/docker-build.sh --no-checkout --without-bsread
#
# Environment overrides:
#   DOCKER_IMAGE      Image tag to build/use (default includes Qt version)
#   DOCKER_NO_BUILD   Set to 1 to skip rebuilding the image
#   DOCKER_NETWORK    Docker network mode (default: host)
#   BUILDX_BUILDER    Optional buildx builder name for --buildx-bake
#   BUILDX_BAKE_OUTPUT Buildx bake output, e.g. type=docker or type=oci,dest=...
#   DOCKER_COPY_SOURCE Copy source into the container instead of bind-mounting
#   DOCKER_CURRENT     Clone the current git remote and branch in checkout mode
#   OUTPUT_DIR        Where to write the final AppImage (default: script dir)
#   QT_VERSION        Qt version built into the image (default: 6.11.1)
#   QT_BUILD_JOBS     Parallel jobs for the Qt source build (default: auto)
#   OPENSSL_VERSION   OpenSSL version built into the image (default: 3.5.9)
#   QWT_VERSION       Qwt version built into the image (default: 6.3.0)
#   QWT_REF           Qwt git tag or branch (default: v$QWT_VERSION)
#   QWT_REPOSITORY    Qwt git repository URL (default: official SourceForge git)
#   EPICS_VERSION_TAG EPICS Base git tag built into the image (default: R7.0.11)
#   GCC_TOOLSET       RHEL/Rocky gcc-toolset version (default: 15)

set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd -- "$SCRIPT_DIR/../../.." && pwd)"

QT_VERSION="${QT_VERSION:-6.11.1}"
QT_BUILD_JOBS="${QT_BUILD_JOBS:-auto}"
OPENSSL_VERSION="${OPENSSL_VERSION:-3.5.9}"
QWT_VERSION="${QWT_VERSION:-6.3.0}"
QWT_REF="${QWT_REF:-v${QWT_VERSION}}"
QWT_REPOSITORY="${QWT_REPOSITORY:-https://git.code.sf.net/p/qwt/git}"
EPICS_VERSION_TAG="${EPICS_VERSION_TAG:-R7.0.11}"
GCC_TOOLSET="${GCC_TOOLSET:-15}"

DOCKER_IMAGE="${DOCKER_IMAGE:-caqtdm-appimage-builder:rhel8-gcc${GCC_TOOLSET}-qt${QT_VERSION}}"
DOCKER_NO_BUILD="${DOCKER_NO_BUILD:-${DOCKER_NO_PULL:-0}}"
DOCKER_NETWORK="${DOCKER_NETWORK:-host}"
BUILDX_BUILDER="${BUILDX_BUILDER:-}"
BUILDX_BAKE_OUTPUT="${BUILDX_BAKE_OUTPUT:-}"
DOCKER_CURRENT="${DOCKER_CURRENT:-0}"
DOCKER_COPY_SOURCE_EXPLICIT=0
[ "${DOCKER_COPY_SOURCE+x}" = x ] && DOCKER_COPY_SOURCE_EXPLICIT=1
DOCKER_COPY_SOURCE="${DOCKER_COPY_SOURCE:-0}"
OUTPUT_DIR="${OUTPUT_DIR:-$SCRIPT_DIR}"
USE_BUILDX_BAKE=0
SOURCE_TRANSPORT=bind
FORWARDED_ARGS=()
CONTAINER_ARGS=()

msg() {
  printf '\n==> %s\n' "$*" >&2
}

die() {
  printf 'error: %s\n' "$*" >&2
  exit 1
}

usage() {
  cat <<EOF
Usage: $(basename "$0") [OPTION...]

Build a portable caQtDM AppImage inside the Rocky Linux 8 Docker environment.
Options are forwarded to create-appimage.sh after the Docker wrapper handles
its own --help option.

Common options passed through to create-appimage.sh:
  --no-checkout        Build from the mounted working tree
  --branch REF         Git branch or tag to clone inside the container
  --repo URL           Git repository to clone inside the container
  --source DIR         Build from an existing source tree inside the container
  --skip-build         Reuse binaries from BINARY_DIR
  --appdir-only        Stop after creating the AppDir
  --without-bsread     Exclude the bsread controlsystem plugin
  --no-download-tools  Require AppImage tools to be available locally
  --help, -h           Show this help and exit

Docker wrapper options:
  --buildx-bake        Build the Docker image with 'docker buildx bake'
  --buildx-builder NAME Use a specific buildx builder for --buildx-bake
  --buildx-output OUT  Set buildx bake output, e.g. type=docker
  --copy-source        Copy source into the container instead of bind-mounting
  --current            In checkout mode, clone the current git remote and branch

Environment overrides:
  DOCKER_IMAGE      Image tag to build/use (default: $DOCKER_IMAGE)
  DOCKER_NO_BUILD   Set to 1 to skip rebuilding the image
  DOCKER_NETWORK    Docker network mode (default: $DOCKER_NETWORK)
  BUILDX_BUILDER    Optional buildx builder name (default: ${BUILDX_BUILDER:-unset})
  BUILDX_BAKE_OUTPUT Buildx bake output (default: ${BUILDX_BAKE_OUTPUT:-unset})
  DOCKER_CURRENT    Clone the current git remote and branch in checkout mode (default: $DOCKER_CURRENT)
  DOCKER_COPY_SOURCE Copy source into the container instead of bind-mounting (default: $DOCKER_COPY_SOURCE)
  OUTPUT_DIR        Where to write the final AppImage (default: $OUTPUT_DIR)
  QT_VERSION        Qt version built into the image (default: $QT_VERSION)
  QT_BUILD_JOBS     Parallel jobs for the Qt source build (default: $QT_BUILD_JOBS)
  OPENSSL_VERSION   OpenSSL version built into the image (default: $OPENSSL_VERSION)
  QWT_VERSION       Qwt version built into the image (default: $QWT_VERSION)
  QWT_REF           Qwt git tag or branch (default: $QWT_REF)
  QWT_REPOSITORY    Qwt git repository URL (default: $QWT_REPOSITORY)
  EPICS_VERSION_TAG EPICS Base git tag built into the image (default: $EPICS_VERSION_TAG)
  GCC_TOOLSET       RHEL/Rocky gcc-toolset version (default: $GCC_TOOLSET)

Examples:
  $(basename "$0")
  $(basename "$0") --branch Development
  $(basename "$0") --no-checkout --without-bsread
  $(basename "$0") --buildx-bake --buildx-builder remote-docker
  $(basename "$0") --buildx-bake --buildx-builder remote-docker --current
  $(basename "$0") --buildx-bake --buildx-builder remote-docker --copy-source --no-checkout

For remote Docker builds, use a Docker context/buildx builder backed by the
remote Docker daemon and leave BUILDX_BAKE_OUTPUT unset. Do not use --load for
that case; --load streams a large image back through the client session.
Checkout-based builds copy only the AppImage packaging files and clone the
default upstream repository and Development branch inside the container. Use
--current to clone the current git remote and branch instead. Use --copy-source
explicitly for --no-checkout or --source builds with a remote Docker daemon,
because remote daemons cannot see local bind mounts. Local Docker --no-checkout
continues to use a bind mount.
EOF
}

parse_args() {
  while [ "$#" -gt 0 ]; do
    case "$1" in
      --help|-h)
        usage
        exit 0
        ;;
      --buildx-bake)
        USE_BUILDX_BAKE=1
        ;;
      --buildx-builder)
        [ "$#" -ge 2 ] || die "--buildx-builder requires a name"
        BUILDX_BUILDER="$2"
        shift
        ;;
      --buildx-output)
        [ "$#" -ge 2 ] || die "--buildx-output requires a value"
        BUILDX_BAKE_OUTPUT="$2"
        shift
        ;;
      --copy-source)
        DOCKER_COPY_SOURCE=1
        DOCKER_COPY_SOURCE_EXPLICIT=1
        ;;
      --current)
        DOCKER_CURRENT=1
        ;;
      *)
        FORWARDED_ARGS+=("$1")
        ;;
    esac
    shift
  done
}

forwarded_args_have_option() {
  local option="$1"
  local arg

  for arg in "${FORWARDED_ARGS[@]}"; do
    case "$arg" in
      "$option"|"$option"=*)
        return 0
        ;;
    esac
  done

  return 1
}

forwarded_args_include_no_checkout() {
  forwarded_args_have_option --no-checkout
}

forwarded_args_include_source() {
  forwarded_args_have_option --source
}

resolve_source_transport() {
  SOURCE_TRANSPORT=bind

  if [ "$DOCKER_COPY_SOURCE" = "1" ]; then
    SOURCE_TRANSPORT=copy-source
  elif [ "$USE_BUILDX_BAKE" = "1" ]; then
    SOURCE_TRANSPORT=copy-packaging
    DOCKER_COPY_SOURCE=1
  fi
}

validate_source_transport() {
  if [ "$USE_BUILDX_BAKE" = "1" ] && forwarded_args_include_source; then
    die "--buildx-bake does not support --source. Use checkout mode, or use --copy-source --no-checkout to build the local tree."
  fi

  if [ "$USE_BUILDX_BAKE" = "1" ] \
    && forwarded_args_include_no_checkout \
    && [ "$SOURCE_TRANSPORT" != copy-source ]; then
    die "--buildx-bake --no-checkout requires --copy-source. Without --copy-source, buildx mode clones the repository inside the container, which contradicts --no-checkout."
  fi

  if [ "$DOCKER_CURRENT" = "1" ] && forwarded_args_include_no_checkout; then
    die "--current cannot be combined with --no-checkout"
  fi

  if [ "$DOCKER_CURRENT" = "1" ] && forwarded_args_include_source; then
    die "--current cannot be combined with --source"
  fi
}

current_git_branch() {
  local branch
  branch="$(git -C "$REPO_ROOT" rev-parse --abbrev-ref HEAD 2>/dev/null || true)"
  [ -n "$branch" ] && [ "$branch" != HEAD ] || return 1
  printf '%s\n' "$branch"
}

current_git_remote() {
  git -C "$REPO_ROOT" remote get-url origin 2>/dev/null || true
}

prepare_container_args() {
  local arg
  local remote
  local branch

  CONTAINER_ARGS=()
  for arg in "${FORWARDED_ARGS[@]}"; do
    CONTAINER_ARGS+=("$arg")
  done

  [ "$DOCKER_CURRENT" = "1" ] || return 0

  if ! forwarded_args_have_option --repo; then
    remote="$(current_git_remote)"
    [ -n "$remote" ] && CONTAINER_ARGS+=(--repo "$remote")
  fi

  if ! forwarded_args_have_option --branch; then
    branch="$(current_git_branch || true)"
    [ -n "$branch" ] && CONTAINER_ARGS+=(--branch "$branch")
  fi
}

require_docker() {
  command -v docker >/dev/null 2>&1 || die "docker is required but was not found"
}

buildx_driver() {
  if [ -n "$BUILDX_BUILDER" ]; then
    docker buildx inspect "$BUILDX_BUILDER"
  else
    docker buildx inspect
  fi | sed -n 's/^Driver:[[:space:]]*//p' | head -n 1
}

validate_buildx_bake_output() {
  local driver

  [ "$USE_BUILDX_BAKE" = "1" ] || return 0
  [ -z "$BUILDX_BAKE_OUTPUT" ] || return 0

  driver="$(buildx_driver)"
  [ -n "$driver" ] || die "could not determine buildx builder driver"

  if [ "$driver" != "docker" ]; then
    die "--buildx-bake without --buildx-output requires a buildx builder using the docker driver; current driver is '$driver'. Your current builder can only leave the image in BuildKit cache, so docker run cannot find it. Use a remote Docker-context builder created with '--driver docker', or set --buildx-output explicitly."
  fi
}

hcl_quote() {
  local value="$1"
  value="${value//\\/\\\\}"
  value="${value//\"/\\\"}"
  printf '"%s"' "$value"
}

write_buildx_bake_file() {
  local bake_file="$1"

  {
    printf 'target "appimage" {\n'
    printf '  context = '; hcl_quote "$SCRIPT_DIR"; printf '\n'
    printf '  dockerfile = "Dockerfile"\n'
    printf '  tags = ['; hcl_quote "$DOCKER_IMAGE"; printf ']\n'
    if [ -n "$BUILDX_BAKE_OUTPUT" ]; then
      printf '  output = ['; hcl_quote "$BUILDX_BAKE_OUTPUT"; printf ']\n'
    fi
    printf '  args = {\n'
    printf '    QT_VERSION = '; hcl_quote "$QT_VERSION"; printf '\n'
    printf '    QT_BUILD_JOBS = '; hcl_quote "$QT_BUILD_JOBS"; printf '\n'
    printf '    OPENSSL_VERSION = '; hcl_quote "$OPENSSL_VERSION"; printf '\n'
    printf '    QWT_VERSION = '; hcl_quote "$QWT_VERSION"; printf '\n'
    printf '    QWT_REF = '; hcl_quote "$QWT_REF"; printf '\n'
    printf '    QWT_REPOSITORY = '; hcl_quote "$QWT_REPOSITORY"; printf '\n'
    printf '    EPICS_VERSION_TAG = '; hcl_quote "$EPICS_VERSION_TAG"; printf '\n'
    printf '    GCC_TOOLSET = '; hcl_quote "$GCC_TOOLSET"; printf '\n'
    printf '  }\n'
    printf '}\n'
  } > "$bake_file"
}

build_image_with_buildx_bake() {
  local bake_file
  local bake_args=()
  bake_file="$(mktemp "${TMPDIR:-/tmp}/caqtdm-appimage-bake.XXXXXX.hcl")"

  write_buildx_bake_file "$bake_file"
  bake_args+=(--file "$bake_file")
  [ -z "$BUILDX_BUILDER" ] || bake_args+=(--builder "$BUILDX_BUILDER")
  bake_args+=(--set "appimage.network=$DOCKER_NETWORK")
  bake_args+=(appimage)

  docker buildx bake "${bake_args[@]}"
  rm -f "$bake_file"
}

build_image() {
  [ "$DOCKER_NO_BUILD" = "1" ] && return 0

  msg "Building Docker image $DOCKER_IMAGE (Rocky Linux 8 baseline)"
  if [ "$USE_BUILDX_BAKE" = "1" ]; then
    validate_buildx_bake_output
    build_image_with_buildx_bake
    return 0
  fi

  docker build \
    --network "$DOCKER_NETWORK" \
    --build-arg "QT_VERSION=$QT_VERSION" \
    --build-arg "QT_BUILD_JOBS=$QT_BUILD_JOBS" \
    --build-arg "OPENSSL_VERSION=$OPENSSL_VERSION" \
    --build-arg "QWT_VERSION=$QWT_VERSION" \
    --build-arg "QWT_REF=$QWT_REF" \
    --build-arg "QWT_REPOSITORY=$QWT_REPOSITORY" \
    --build-arg "EPICS_VERSION_TAG=$EPICS_VERSION_TAG" \
    --build-arg "GCC_TOOLSET=$GCC_TOOLSET" \
    --tag "$DOCKER_IMAGE" \
    "$SCRIPT_DIR"
}

resolve_output_dir() {
  mkdir -p "$OUTPUT_DIR"
  OUTPUT_DIR="$(cd -- "$OUTPUT_DIR" && pwd)"
}

build_docker_run_args() {
  docker_run_args=(
    --rm
    --interactive
    --network "$DOCKER_NETWORK"
    --env "APPIMAGE_EXTRACT_AND_RUN=1"
    --env "OUTPUT_DIR=/output"
    --env "BUILD_DIR=/src/caQtDM_Viewer/package/appimage/build-docker"
    --env "TOOLS_DIR=/src/caQtDM_Viewer/package/appimage/tools"
  )

  if [ "$SOURCE_TRANSPORT" = bind ]; then
    docker_run_args+=(
      --volume "$REPO_ROOT:/src"
      --volume "$OUTPUT_DIR:/output"
    )
  fi

  [ ! -t 1 ] || docker_run_args+=(--tty)

  if [ -e /dev/fuse ]; then
    docker_run_args+=(--device /dev/fuse --cap-add SYS_ADMIN --security-opt apparmor:unconfined)
  fi
}

copy_source_to_container() {
  local container_id="$1"
  local tar_excludes=(
    --exclude=.git
    --exclude=.idea
    --exclude=caQtDM_Viewer/package/appimage/build
    --exclude=caQtDM_Viewer/package/appimage/build-docker
    --exclude=caQtDM_Viewer/package/appimage/squashfs-root
    --exclude=caQtDM_Viewer/package/appimage/tools
    --exclude=caQtDM_Viewer/package/appimage/work
    --exclude='caQtDM_Viewer/package/appimage/*.AppImage'
    --exclude='caQtDM_Viewer/package/appimage/*.AppImage.zsync'
  )

  msg "Copying source tree into Docker container"
  tar "${tar_excludes[@]}" -C "$REPO_ROOT" -cf - . | docker cp - "$container_id:/src"
}

copy_packaging_to_container() {
  local container_id="$1"
  local paths=(
    caQtDM_Viewer/package/appimage/create-appimage.sh
    caQtDM_Viewer/package/appimage/io.github.caqtdm.caqtdm.desktop
    caQtDM_Viewer/package/appimage/io.github.caqtdm.caqtdm.metainfo.xml
    caQtDM_Viewer/package/appimage/icons
  )

  msg "Copying AppImage packaging scripts into Docker container"
  tar -C "$REPO_ROOT" -cf - "${paths[@]}" | docker cp - "$container_id:/src"
}

run_container_with_copied_source() {
  local container_id
  local create_args=()
  local arg

  build_docker_run_args
  docker image inspect "$DOCKER_IMAGE" >/dev/null 2>&1 || die "Docker image is not available to docker run: $DOCKER_IMAGE"

  for arg in "${docker_run_args[@]}"; do
    [ "$arg" = --rm ] && continue
    create_args+=("$arg")
  done

  prepare_container_args
  msg "Creating AppImage build container from $DOCKER_IMAGE"
  container_id="$(docker create "${create_args[@]}" --entrypoint /bin/sh "$DOCKER_IMAGE" -lc 'exec /src/caQtDM_Viewer/package/appimage/create-appimage.sh "$@"' sh "${CONTAINER_ARGS[@]}")"
  trap 'docker rm -f "$container_id" >/dev/null 2>&1 || true' RETURN

  case "$SOURCE_TRANSPORT" in
    copy-packaging) copy_packaging_to_container "$container_id" ;;
    copy-source) copy_source_to_container "$container_id" ;;
    *) die "internal error: copied-source runner used with source transport '$SOURCE_TRANSPORT'" ;;
  esac
  msg "Running AppImage build inside $DOCKER_IMAGE"
  docker start --attach "$container_id"

  mkdir -p "$OUTPUT_DIR"
  docker cp "$container_id:/output/." "$OUTPUT_DIR/"
  docker rm -f "$container_id" >/dev/null
  trap - RETURN
}

run_container() {
  if [ "$SOURCE_TRANSPORT" != bind ]; then
    run_container_with_copied_source
    return 0
  fi

  build_docker_run_args
  docker image inspect "$DOCKER_IMAGE" >/dev/null 2>&1 || die "Docker image is not available to docker run: $DOCKER_IMAGE"
  msg "Running AppImage build inside $DOCKER_IMAGE"
  docker run "${docker_run_args[@]}" "$DOCKER_IMAGE" "$@"
}

print_output_summary() {
  msg "AppImage written to: $OUTPUT_DIR"
  ls -lh "$OUTPUT_DIR"/*.AppImage 2>/dev/null || true
}

main() {
  parse_args "$@"
  resolve_source_transport
  validate_source_transport
  require_docker
  build_image
  resolve_output_dir
  run_container "${FORWARDED_ARGS[@]}"
  print_output_summary
}

main "$@"
