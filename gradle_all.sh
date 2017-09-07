PARAM=$0 

for d in `find . -maxdepth 1 -type d \( ! -regex '.*/\..*' \)`
do
    ( cd $d && ./gradlew $PARAM )
done