SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"
make clean; make
cd ../working/
../1-clone-project/bin/llmon_fetch $1
