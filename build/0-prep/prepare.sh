SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"
mkdir -p ../../projects/$1  #this could be better
cd ../
rm -rf working/*/  #delete all working directories, but leave README.md
