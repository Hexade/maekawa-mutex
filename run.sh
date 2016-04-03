#./bin/sh

cd bin/
for i in {1..3}
do
    ./server $i &
done

for i in {1..7}
do
    ./client $i &
done
