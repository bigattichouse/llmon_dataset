
echo "Clone project $1..."

cd 1-clone-project
make clean; make
cd ../working/
../1-clone-project/bin/llmon_fetch $1
