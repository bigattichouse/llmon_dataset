
echo "Clone project $1..."
mkdir -p ../projects/$1  #this could be better
cd 1-clone-project
make clean; make
cd ../working/
../1-clone-project/bin/llmon_fetch $1
